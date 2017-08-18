#include <stdio.h>    
#include <stdlib.h>    
#include <asm/user.h>    
#include <asm/ptrace.h>    
#include <sys/ptrace.h>    
#include <sys/wait.h>    
#include <sys/mman.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>    
#include <dlfcn.h>    
#include <dirent.h>    
#include <unistd.h>    
#include <string.h>    
#include <elf.h>    
#include <android/log.h>    
    
#if defined(__i386__)    
#define pt_regs         user_regs_struct    
#endif    
    
#define ENABLE_DEBUG 1    
    
#if ENABLE_DEBUG    
#define  LOG_TAG "HOOK_YY"    
#define  LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, fmt, ##args)    
#define DEBUG_PRINT(format,args...) \    
    LOGD(format, ##args)    
#else    
#define DEBUG_PRINT(format,args...)    
#endif    
    
#define CPSR_T_MASK     ( 1u << 5 )    

const char *libc_path = "/system/lib/libc.so";    
const char *linker_path = "/system/bin/linker";
int enable_hook = 1;    
    
int ptrace_readdata(pid_t pid,  uint8_t *src, uint8_t *buf, size_t size)    
{    
    uint32_t i, j, remain;    
    uint8_t *laddr;    
    
    union u {    
        long val;    
        char chars[sizeof(long)];    
    } d;    
    
    j = size / 4;    
    remain = size % 4;    
    
    laddr = buf;    
    
    for (i = 0; i < j; i ++) {    
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);    
        memcpy(laddr, d.chars, 4);    
        src += 4;    
        laddr += 4;    
    }    
    
    if (remain > 0) {    
        d.val = ptrace(PTRACE_PEEKTEXT, pid, src, 0);    
        memcpy(laddr, d.chars, remain);    
    }    
    
    return 0;    
}    
    
int ptrace_writedata(pid_t pid, uint8_t *dest, uint8_t *data, size_t size)    
{    
    uint32_t i, j, remain;    
    uint8_t *laddr;    
    
    union u {    
        long val;    
        char chars[sizeof(long)];    
    } d;    
    
    j = size / 4;    
    remain = size % 4;    
    
    laddr = data;    
    
    for (i = 0; i < j; i ++) {    
        memcpy(d.chars, laddr, 4);    
        ptrace(PTRACE_POKETEXT, pid, dest, d.val);    
    
        dest  += 4;    
        laddr += 4;    
    }    
    
    if (remain > 0) {    
        d.val = ptrace(PTRACE_PEEKTEXT, pid, dest, 0);    
        for (i = 0; i < remain; i ++) {    
            d.chars[i] = *laddr ++;    
        }    
    
        ptrace(PTRACE_POKETEXT, pid, dest, d.val);    
    }    
    
    return 0;    
}    
    
#if defined(__arm__)    
int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs)    
{    
    uint32_t i;    
    for (i = 0; i < num_params && i < 4; i ++) {    
        regs->uregs[i] = params[i];    
    }    
    
    //    
    // push remained params onto stack    
    //    
    if (i < num_params) {    
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;    
        ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)&params[i], (num_params - i) * sizeof(long));    
    }    
    
    regs->ARM_pc = addr;  
    //fix me:something wrong may occur here  
    if (regs->ARM_pc & 1) {    
        /* thumb */    
        regs->ARM_pc &= (~1u);//thumb should ignore pc bit[0],so we clear it by & 0XFFFFFFFE
        regs->ARM_cpsr |= CPSR_T_MASK;//set T bit to indicate thumb operation
    } else {    
        /* arm */    
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }    
    
    regs->ARM_lr = 0;//set lr = NULL will cause SIGSEGV error when function return        
    
    if (ptrace_setregs(pid, regs) == -1     
            || ptrace_continue(pid) == -1) {    
        printf("error\n");    
        return -1;    
    }    
    
    int stat = 0;  
    waitpid(pid, &stat, WUNTRACED);  
    //0xb7f means SIGSEGV and STOPPED 
    while (stat != 0xb7f) {  
        if (ptrace_continue(pid) == -1) {  
            printf("error\n");  
            return -1;  
        }  
        waitpid(pid, &stat, WUNTRACED);  
    }  
    
    return 0;    
}    
    
#elif defined(__i386__)    
long ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct user_regs_struct * regs)    
{   
    //write params to stack 
    regs->esp -= (num_params) * sizeof(long) ;    
    ptrace_writedata(pid, (void *)regs->esp, (uint8_t *)params, (num_params) * sizeof(long));    
    
    //write return address to stack
    long tmp_addr = 0x00;    
    regs->esp -= sizeof(long);    
    ptrace_writedata(pid, regs->esp, (char *)&tmp_addr, sizeof(tmp_addr));     
    
    //set ip for redirect
    regs->eip = addr;    
    
    if (ptrace_setregs(pid, regs) == -1     
            || ptrace_continue( pid) == -1) {    
        printf("error\n");    
        return -1;    
    }    
    
    int stat = 0;  
    waitpid(pid, &stat, WUNTRACED);  
    while (stat != 0xb7f) {  
        if (ptrace_continue(pid) == -1) {  
            printf("error\n");  
            return -1;  
        }  
        waitpid(pid, &stat, WUNTRACED);  
    }  
    
    return 0;    
}    
#else     
#error "Not supported"    
#endif    
    
int ptrace_getregs(pid_t pid, struct pt_regs * regs)    
{    
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {    
        perror("ptrace_getregs: Can not get register values");    
        return -1;    
    }    
    
    return 0;    
}    
    
int ptrace_setregs(pid_t pid, struct pt_regs * regs)    
{    
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {    
        perror("ptrace_setregs: Can not set register values");    
        return -1;    
    }    
    
    return 0;    
}    
    
int ptrace_continue(pid_t pid)    
{    
    if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {    
        perror("ptrace_cont");    
        return -1;    
    }    
    
    return 0;    
}    
    
int ptrace_attach(pid_t pid)    
{    
    if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {    
        perror("ptrace_attach");    
        return -1;    
    }    
    
    int status = 0;    
    waitpid(pid, &status , WUNTRACED);    
    
    return 0;    
}    
    
int ptrace_detach(pid_t pid)    
{    
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {    
        perror("ptrace_detach");    
        return -1;    
    }    
    
    return 0;    
}    

//遍历proc/pid/maps文件，查找模块的映射基址
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

//本地地址在远端进程里的映射地址：远端基址 + 偏移地址  
void* get_remote_addr(pid_t target_pid, const char* module_name, void* local_addr)    
{    
    void* local_handle, *remote_handle;    
    
    local_handle = get_module_base(-1, module_name);    
    remote_handle = get_module_base(target_pid, module_name);    
    
    DEBUG_PRINT("[+] get_remote_addr: local[%x], remote[%x]\n", local_handle, remote_handle);    
    
    void * ret_addr = (void *)((uint32_t)local_addr + (uint32_t)remote_handle - (uint32_t)local_handle);    
    
#if defined(__i386__)    
    if (!strcmp(module_name, libc_path)) {    
        ret_addr += 2;    
    }    
#endif    
    return ret_addr;    
}    

//遍历/proc/目录查找目标进程 
int find_pid_of(const char *process_name)    
{    
    int id;    
    pid_t pid = -1;    
    DIR* dir;    
    FILE *fp;    
    char filename[32];    
    char cmdline[256];    
    
    struct dirent * entry;    
    
    if (process_name == NULL)    
        return -1;    
    
    dir = opendir("/proc");    
    if (dir == NULL)    
        return -1;    
    
    while((entry = readdir(dir)) != NULL) {    
        id = atoi(entry->d_name);    
        if (id != 0) {    
            sprintf(filename, "/proc/%d/cmdline", id);    
            fp = fopen(filename, "r");    
            if (fp) {    
                fgets(cmdline, sizeof(cmdline), fp);    
                fclose(fp);    
    
                if (strcmp(process_name, cmdline) == 0) {    
                    /* process found */    
                    pid = id;    
                    break;    
                }    
            }    
        }    
    }    
    
    closedir(dir);    
    return pid;    
}    

//下面的数据在android 源码里面找到定义
//\Android\external\kernel-headers\original\asm-arm\ptrace.h
//\Android\external\kernel-headers\original\asm-x86\ptrace.h
//获取函数调用返回值寄存器
long ptrace_retval(struct pt_regs * regs)    
{    
#if defined(__arm__)    
    return regs->ARM_r0;    
#elif defined(__i386__)    
    return regs->eax;    
#else    
#error "Not supported"    
#endif    
}    

//获取指令寄存器
long ptrace_ip(struct pt_regs * regs)    
{    
#if defined(__arm__)    
    return regs->ARM_pc;    
#elif defined(__i386__)    
    return regs->eip;    
#else    
#error "Not supported"    
#endif    
}    

//这里只是简单处理了返回值小于4字节的情况，返回更多的数据需要按照对应平台的ABI约定
//来对返回值进行读取
int ptrace_call_wrapper(pid_t target_pid, const char * func_name, void * func_addr, long * parameters, int param_num, struct pt_regs * regs)     
{    
    DEBUG_PRINT("[+] Calling %s in target process.\n", func_name);    
    if (ptrace_call(target_pid, (uint32_t)func_addr, parameters, param_num, regs) == -1)    
        return -1;    
    
    if (ptrace_getregs(target_pid, regs) == -1)    
        return -1;

    int ret = ptrace_retval(regs);
    DEBUG_PRINT("[+] Target process returned from %s, return value=%d, pc=%x \n",     
            func_name, ret, ptrace_ip(regs));    
    return ret;    
}    
    
int inject_remote_process(pid_t target_pid, const char *library_path, const char *function_name, const char *param, size_t param_size)    
{    
    int ret = -1, server_port = 0;    
    void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;    
    void *local_handle, *remote_handle, *dlhandle;    
    uint8_t *map_base = 0;      
    
    struct pt_regs regs, original_regs;      
        
    long parameters[10];    
    
    DEBUG_PRINT("[+] Injecting process: %d\n", target_pid);    
    
    if (ptrace_attach(target_pid) == -1) {    
        DEBUG_PRINT("ptrace_attach failed, no root priority\n");
        goto exit;    
    }
    
    if (ptrace_getregs(target_pid, &regs) == -1) {    
        DEBUG_PRINT("ptrace_getregs failed\n");
        goto exit;    
    }
    
    /* save original registers */    
    memcpy(&original_regs, &regs, sizeof(regs));    
    
    mmap_addr = get_remote_addr(target_pid, libc_path, (void *)mmap);    
    DEBUG_PRINT("[+] Remote mmap address: %x\n", mmap_addr);    
    
    /* call mmap to alloc 16kb memory for params pass*/    
    parameters[0] = 0;  // addr    
    parameters[1] = 0x4000; // 16kb size    
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot    
    parameters[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // flags    
    parameters[4] = 0; //fd    
    parameters[5] = 0; //offset    
    
    if (ptrace_call_wrapper(target_pid, "mmap", mmap_addr, parameters, 6, &regs) == -1) {   
        DEBUG_PRINT("ptrace_call(mmap) failed\n");
        goto exit;    
    }
    
    map_base = ptrace_retval(&regs);    
    
    dlopen_addr = get_remote_addr( target_pid, linker_path, (void *)dlopen );    
    dlsym_addr = get_remote_addr( target_pid, linker_path, (void *)dlsym );    
    dlclose_addr = get_remote_addr( target_pid, linker_path, (void *)dlclose );    
    dlerror_addr = get_remote_addr( target_pid, linker_path, (void *)dlerror );    
    
    DEBUG_PRINT("[+] Get imports: dlopen: %x, dlsym: %x, dlclose: %x, dlerror: %x\n",    
            dlopen_addr, dlsym_addr, dlclose_addr, dlerror_addr);    
    
    DEBUG_PRINT("library path = %s\n", library_path);
    //write library_path content to map_base anh then we pass map_base to dlopen()    
    ptrace_writedata(target_pid, map_base, library_path, strlen(library_path) + 1);    
    
    parameters[0] = map_base;       
    parameters[1] = RTLD_NOW| RTLD_GLOBAL;     
    
    if (ptrace_call_wrapper(target_pid, "dlopen", dlopen_addr, parameters, 2, &regs) == -1) {   
        DEBUG_PRINT("ptrace_call(dlopen) failed:%s\n", library_path);
        goto exit;    
    }
    
    //got so handle for library_path
    void * sohandle = ptrace_retval(&regs);    
    
#define FUNCTION_NAME_ADDR_OFFSET       0x100 //OFFSET 256B
    //call "dlsym" with sohandle and function_name 
    ptrace_writedata(target_pid, map_base + FUNCTION_NAME_ADDR_OFFSET, function_name, strlen(function_name) + 1);    
    parameters[0] = sohandle;       
    parameters[1] = map_base + FUNCTION_NAME_ADDR_OFFSET;     
    DEBUG_PRINT("dlopen result:%p\n", sohandle);
    if (ptrace_call_wrapper(target_pid, "dlsym", dlsym_addr, parameters, 2, &regs) == -1) {    
        DEBUG_PRINT("ptrace_call(dlsym) failed\n");
        goto exit;    
    }
    
    void * hook_entry_addr = ptrace_retval(&regs);    
    DEBUG_PRINT("hook_entry_addr = %p\n", hook_entry_addr);    
    
#define FUNCTION_PARAM_ADDR_OFFSET      0x200//OFFSET 512B
    ptrace_writedata(target_pid, map_base + FUNCTION_PARAM_ADDR_OFFSET, param, strlen(param) + 1);    
    parameters[0] = map_base + FUNCTION_PARAM_ADDR_OFFSET;      
  
    if (enable_hook == 1) {
        server_port = ptrace_call_wrapper(target_pid, "hook_entry", hook_entry_addr, parameters, 1, &regs);
        if (server_port == -1) {    
            DEBUG_PRINT("ptrace_call(hook_entry) failed\n");
            goto exit;
        }
        DEBUG_PRINT("\nhook_entry get server port:%d\n", server_port);
    }        
       
    parameters[0] = sohandle;
    
    /* restore */    
    ptrace_setregs(target_pid, &original_regs);    
    ptrace_detach(target_pid);    
    ret = 0;    
    
exit:    
    return server_port;    
}

//test for command server
void connectToCommandServer(const char *command, size_t command_size) {
    int  sockfd;
    struct hostent *he;
    struct sockaddr_in server;

    if ((he = gethostbyname("127.0.0.1")) == NULL) {
        DEBUG_PRINT("error when transform ip\n");
        return;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        DEBUG_PRINT("socket error\n");
        return;
    }

    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(32455);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    if(connect(sockfd,(struct sockaddr *)&server, sizeof(server))==-1){
        DEBUG_PRINT("connect()error\n");
        return;
    }

    //send test message
    int ret;
    if (-1 == (ret = send(sockfd, command, command_size, 0))) {
        DEBUG_PRINT("send command fail:%d\n", errno);
    }
    DEBUG_PRINT("send command%d", ret);
}    
    
int main(int argc, char** argv) {
    char *function_name;
    char *target_process_name;
    char *hookPath, *substratePath;
    int opt, connectToServer = 0;
    while ((opt = getopt(argc, argv, "p:r:c:h:s:")) != -1) {
        switch (opt) {
            case 'p': {
                int length = strlen(optarg) + 1;
                length = length/4 + (length%4 ? 1 : 0);
                target_process_name = malloc(length * sizeof(unsigned long));
                memcpy(target_process_name, optarg, length*4);
            }
                break;
            case 'r':
                enable_hook = strtol(optarg, NULL ,0);
                DEBUG_PRINT("enable hook action:%d\n", enable_hook);
                break;
            case 'c': {
                int length = strlen(optarg) + 1;
                length = length/4 + (length%4 ? 1 : 0);
                char *command = malloc(length * sizeof(unsigned long));
                memcpy(command, optarg, length*4);
                connectToCommandServer(command, length*4);
                connectToServer = 1;
            }
                break;
            case 'h': {
                int length = strlen(optarg) + 1;
                length = length/4 + (length%4 ? 1 : 0);
                hookPath = malloc(length * sizeof(unsigned long));
                memcpy(hookPath, optarg, length*4);
            }
                break;
            case 's': {
                int length = strlen(optarg) + 1;
                length = length/4 + (length%4 ? 1 : 0);
                substratePath = malloc(length * sizeof(unsigned long));
                memcpy(substratePath, optarg, length*4);
            }
                break;
            default:
                DEBUG_PRINT("invalid param\n");
                break;
        }
    }

    if (1 == connectToServer) {
        return -1;
    }

    DEBUG_PRINT("inject begin\n");    
    pid_t target_pid;    
    target_pid = find_pid_of(target_process_name);    
    if (-1 == target_pid) {  
        DEBUG_PRINT("Can't find the process:%s\n", target_process_name);  
        return -2;  
    }  
    

    DEBUG_PRINT("hookPath:%s\n", hookPath);
    DEBUG_PRINT("substratePath:%s\n", substratePath);
    int server_port = inject_remote_process(target_pid, hookPath, "hook_entry",  substratePath, strlen(substratePath));
    DEBUG_PRINT("command_server port:%d", server_port);
    DEBUG_PRINT("inject end");
    printf("server_port#%d", server_port);//for shell result
    return server_port;  
}    