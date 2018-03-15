//Hook代码

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "INJECT"
#define LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define DEBUG_PRINT(format, args...) LOGD(format, ##args)

typedef unsigned int (*PFNSLEEP)(unsigned int);

typedef unsigned short Elf32_Half;
typedef unsigned long Elf32_Word;
typedef unsigned long Elf32_Addr;
typedef unsigned long Elf32_Off;

#define EI_NIDENT (16)
#define SHT_PROGBITS 1

typedef struct
{
  unsigned char    e_ident[EI_NIDENT];    /* Magic number and other info */
  Elf32_Half    e_type;            /* Object file type */
  Elf32_Half    e_machine;        /* Architecture */
  Elf32_Word    e_version;        /* Object file version */
  Elf32_Addr    e_entry;        /* Entry point virtual address */
  Elf32_Off    e_phoff;        /* Program header table file offset */
  Elf32_Off    e_shoff;        /* Section header table file offset */
  Elf32_Word    e_flags;        /* Processor-specific flags */
  Elf32_Half    e_ehsize;        /* ELF header size in bytes */
  Elf32_Half    e_phentsize;        /* Program header table entry size */
  Elf32_Half    e_phnum;        /* Program header table entry count */
  Elf32_Half    e_shentsize;        /* Section header table entry size */
  Elf32_Half    e_shnum;        /* Section header table entry count */
  Elf32_Half    e_shstrndx;        /* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
  Elf32_Word    sh_name;        /* Section name (string tbl index) */
  Elf32_Word    sh_type;        /* Section type */
  Elf32_Word    sh_flags;        /* Section flags */
  Elf32_Addr    sh_addr;        /* Section virtual addr at execution */
  Elf32_Off    sh_offset;        /* Section file offset */
  Elf32_Word    sh_size;        /* Section size in bytes */
  Elf32_Word    sh_link;        /* Link to another section */
  Elf32_Word    sh_info;        /* Additional section information */
  Elf32_Word    sh_addralign;        /* Section alignment */
  Elf32_Word    sh_entsize;        /* Entry size if section holds table */
} Elf32_Shdr;

//新函数
unsigned int NewSleep(unsigned int seconds)
{
    printf("Hello NewSleep\r\n");

    //调用原始函数，也可以定义一个函数指针来调
    //return (*pfnSleep)(seconds);
    return sleep(seconds);
}

//解析Got表的内存偏移和表的大小
int GetGotTableInfo(int *lpnVirtualAddr, int *lpnSize)
{
    int nRet = -1;
    FILE *fp = fopen("/data/local/tmp/hello", "r");
    if (fp == NULL)
    {
        return -1;
    }

    Elf32_Ehdr Elf32Header;
    Elf32_Shdr Elf32SectionHeader;

    //不做返回值检查了
    fread(&Elf32Header, sizeof(Elf32_Ehdr), 1, fp);

    fseek(fp, Elf32Header.e_shstrndx * Elf32Header.e_shentsize + Elf32Header.e_shoff, SEEK_SET);
    fread(&Elf32SectionHeader, Elf32Header.e_shentsize, 1, fp);

    char *lpStringTable = (char *)malloc(Elf32SectionHeader.sh_size);
    if (lpStringTable == NULL)
    {
        goto SAFE_END;
    }

    fseek(fp, Elf32SectionHeader.sh_offset, SEEK_SET);
    fread(lpStringTable, Elf32SectionHeader.sh_size, 1, fp);
    fseek(fp, Elf32Header.e_shoff, SEEK_SET);

    int nNameIndex = 0;
    int i = 0;

    for (i = 0; i < Elf32Header.e_shnum; i++)
    {
        fread(&Elf32SectionHeader, Elf32Header.e_shentsize, 1, fp);
        if (Elf32SectionHeader.sh_type == SHT_PROGBITS)
        {
            nNameIndex = Elf32SectionHeader.sh_name;
            if ((strcmp(&(lpStringTable[nNameIndex]), ".got.plt") == 0) ||
                (strcmp(&(lpStringTable[nNameIndex]), ".got") == 0))
            {
                //不是.so就不用修正地址了, so的话需要加上模块基地址修正
                *lpnVirtualAddr = Elf32SectionHeader.sh_addr;      
                *lpnSize = Elf32SectionHeader.sh_size;
                nRet = 0;
                break;
            }
        }
    }

SAFE_END:
    if (fp != NULL)
    {
        fclose(fp);
    }

    if (lpStringTable != NULL)
    {
        free(lpStringTable);
    }

    return nRet;
}

void MyHook()
{
    DEBUG_PRINT("entry myhook, L:%d", __LINE__);
    int nVirtualAddr = 0;
    int nSize = 0;
    int i = 0;

    //获取GotTable的VirtualAddr, Size
    if (GetGotTableInfo(&nVirtualAddr, &nSize) == -1)
    {
      DEBUG_PRINT("level myhook, L:%d", __LINE__);
        return;
    }

    //遍历Got表中的每一项
    for (i = 0; i < nSize; i += 4)
    {
        if ((int)sleep == (*(int *)(nVirtualAddr + i)))
        {
            //修改内存保护属性为可写
            mprotect((void *)0x9000, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
            *(int *)(nVirtualAddr + i) = (int)NewSleep;
            break;
        }
    }
    DEBUG_PRINT("level myhook, L:%d", __LINE__);
}