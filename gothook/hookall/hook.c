#include <sys/types.h>
#include <stdio.h>
#include <asm/fcntl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/exec_elf.h>
#include <asm/mman.h>
#include <sys/mman.h>
#include "hook.h"

//void* get_module_base(pid_t pid, const char* module_name)
//{
//    int fd;
//    long addr = 0;
//    char *pch;
//    char filename[32];
//    char line[1024];
//
//    if (pid < 0) {
//        /* self process */
//        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
//    } else {
//        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
//    }
//
//    FILE *fp = open(filename, O_RDONLY);
//
//    if (fp != NULL) {
//        while (readline(fd,line,sizeof(line))) {
//            if (strstr(line, module_name)) {
//                pch = strtok( line, "-" );
//                addr = strtoul( pch, NULL, 16 );
//
//                if (addr == 0x8000)
//                    addr = 0;
//                break;
//            }
//        }
//        close(fd) ;
//    }
//    return (void *)addr;
//}

void* get_module_base(pid_t pid, const char* module_name)
{
    FILE *fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    if (pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }

    fp = fopen(filename, "r");

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok( line, "-" );
                addr = strtoul( pch, NULL, 16 );

                if (addr == 0x8000)
                    addr = 0;

                break;
            }
        }

        fclose(fp) ;
    }

    return (void *)addr;
}

int Hook(void* new_func,char* so_path,void* old_func)
{
    void* base_addr = NULL;

    Elf32_Shdr shdr;
    Elf32_Ehdr ehdr;
    unsigned long shdr_addr;
    int shnum;
    int shent_size;
    int i;
    unsigned long stridx;

    uint32_t out_addr = 0;
    uint32_t out_size = 0;
    uint32_t got_item = 0;
    int32_t got_found = 0;
    char * string_table = NULL;

    LOGD("[+]so path = %s [+]\n", so_path);
    if(so_path != NULL)
        base_addr = get_module_base(getpid(),so_path);
    LOGD("%s address = %p\n",so_path,base_addr);

    FILE *fd = open(so_path, O_RDONLY);
    if (-1 == fd) {
        LOGD("[+] error: Open %s failed [+]\n",so_path);
        return -1;
    }
 
    read(fd, &ehdr, sizeof(Elf32_Ehdr));

    shdr_addr = ehdr.e_shoff;
    shnum = ehdr.e_shnum;
    shent_size = ehdr.e_shentsize;
    stridx = ehdr.e_shstrndx;
 
    lseek(fd, shdr_addr + stridx * shent_size, SEEK_SET);
    read(fd, &shdr, shent_size);

    string_table = (char *)malloc(shdr.sh_size);
    lseek(fd, shdr.sh_offset, SEEK_SET);
    read(fd, string_table, shdr.sh_size);
    lseek(fd, shdr_addr, SEEK_SET);

    

    for (i = 0; i < shnum; i++) {
        read(fd, &shdr, shent_size);
        if (shdr.sh_type == SHT_PROGBITS) {
            int name_idx = shdr.sh_name;
            if (strcmp(&(string_table[name_idx]), ".got.plt") == 0
                    || strcmp(&(string_table[name_idx]), ".got") == 0) {
                out_addr = base_addr + shdr.sh_addr;
                out_size = shdr.sh_size;
                LOGD("[+] out_addr = %lx, out_size = %lx [+]\n", out_addr, out_size);

                for (i = 0; i < out_size; i += 4) {
//                  LOGD("loop\n");
                    got_item = *(uint32_t *)(out_addr + i);
                    if (got_item  == old_func) {
                        LOGD("[+] Found target function in got[+]\n");
                        got_found = 1;

                        uint32_t page_size = getpagesize();
                        uint32_t entry_page_start = (out_addr + i) & (~(page_size - 1));
                        mprotect((uint32_t *)entry_page_start, page_size, PROT_READ | PROT_WRITE);
                        *(uint32_t *)(out_addr + i) = new_func;

                        break;
                    } else if (got_item == new_func) {
                        LOGD("[+] Already hooked [+]\n");
                        break;
                    }
                }
                if (got_found)
                    break;
            }
        }
    }

    free(string_table);
    close(fd);

}