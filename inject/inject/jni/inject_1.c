//Inject代码

#include <jni.h>
#include <stdio.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define LOG_TAG "INJECT"
#define LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define DEBUG_PRINT(format, args...) LOGD(format, ##args)

#define LIBC_PATH   "/system/lib/libc.so"
#define LINKER_PATH "/system/bin/linker"

#define CPSR_T_MASK     ( 1u << 5 )

/*--------------------------------------------------
*   功能:   通过进程的名称获取对应的进程Pid
*
*   返回值: 未找到返回-1
*--------------------------------------------------*/
int FindProIdByProName(const char *lpszProName)
{
    DIR* lpDirp = NULL;
    struct dirent* lpDirentp = NULL;
    int nPid = 0;
    FILE *fp = NULL;
    char szLines[1024] = {0};
    char szCmdlinePath[256] = {0};
    int nFind = 0;

    lpDirp = opendir("/proc");
    if(lpDirp == NULL)
    {
        DEBUG_PRINT("[-]FindProIdByProName::opendir error\r\n");
        return -1;
    }

    while ((lpDirentp = readdir(lpDirp)) != NULL)
    {
        nPid = atoi(lpDirentp->d_name);
        memset(szCmdlinePath, sizeof(szCmdlinePath), 0);
        snprintf(szCmdlinePath, sizeof(szCmdlinePath), "/proc/%d/cmdline", nPid);
        fp = fopen(szCmdlinePath, "r");
        if (fp != NULL)
        {
            fgets(szLines, sizeof(szLines), fp);
            if (strcmp(szLines, lpszProName) == 0)
            {
                nFind = 1;
                DEBUG_PRINT("[+]Find ProId = %d\r\n", nPid);
                fclose(fp);
                fp = NULL;
                break;
            }

            fclose(fp);
            fp = NULL;
        }
    }

    if (nFind == 0)
    {
        DEBUG_PRINT("[-]No Find ProId\r\n");
        return -1;
    }

    return nPid;
}

/*--------------------------------------------------
*   功能:   附加进程
*
*   返回值: 失败返回-1
*--------------------------------------------------*/
int PtraceAttach(int nPid)
{
    //被跟踪进程将成为当前进程的子进程，并进入中止状态。
    if (ptrace(PTRACE_ATTACH, nPid, NULL, NULL) == -1)
    {
        return -1;
    }

    int nStatus = 0;

    //如果子进程进入暂停执行情况则马上返回,但结束状态不予以理会。
    //父进程退出时, 不影响子进程
    waitpid(nPid, &nStatus , WUNTRACED);

    return 0;
}

/*--------------------------------------------------
*   功能:   获取指定进程的寄存器信息
*
*   返回值: 失败返回-1
*--------------------------------------------------*/
int PtraceGetRegs(int nPid, struct pt_regs *lpRegs)
{
    if (ptrace(PTRACE_GETREGS, nPid, NULL, lpRegs) == -1)
    {
        return -1;
    }

    return 0;
}

/*--------------------------------------------------
*   功能:   取消附加
*
*   返回值: 失败返回-1
*--------------------------------------------------*/
int PtraceDetach(int nPid)
{
    if (ptrace(PTRACE_DETACH, nPid, NULL, NULL) == -1)
    {
        return -1;
    }

    return 0;
}

/*--------------------------------------------------
*   功能:   获取进程中指定模块的首地址
*
*   参数:
*           nPid            需要注入的进程Pid, 如果为0则获取自身进程
*           lpLibraryPath   需要获取模块路径
*
*   返回值: 失败返回NULL, 成功返回Addr
*--------------------------------------------------*/
void* GetModuleBase(int nPid, const char *lpLibraryPath)
{
    char szPath[256] = {0};
    char szLines[1024] = {0};
    char *lpCh = NULL;
    void *lpBaseAddr = NULL;

    if (nPid == 0)
    {
        snprintf(szPath, sizeof(szPath), "/proc/self/maps");
    }
    else
    {
        snprintf(szPath, sizeof(szPath), "/proc/%d/maps", nPid);
    }

    FILE *fp = fopen(szPath, "r");
    if (fp != NULL)
    {
        while (fgets(szLines, sizeof(szLines), fp))
        {
            if (strstr(szLines, lpLibraryPath))
            {
                lpCh = strtok(szLines, "-");
                lpBaseAddr = strtoul(lpCh, NULL, 16);
                fclose(fp);
                break;
            }
        }

        fclose(fp);
        fp = NULL;
    }

    return lpBaseAddr;
}

/*--------------------------------------------------
*   功能:   获取目标进程中函数指针
*
*   参数:
*           nPid            需要注入的进程Pid
*           lpLibraryPath   需要获取的函数所在的lib库路径
*           lpFunctionAddr  需要获取的函数所在当前进程内存中的地址
*
*           目标进程中函数指针 = 目标进程模块基址 - 自身进程模块基址 + 内存中的地址
*
*   返回值: 失败返回NULL, 成功返回Addr
*--------------------------------------------------*/
void* GetRemoteFunctionAddr(int nPid, const char *lpLibraryPath, void *lpFunctionAddr)
{
    //获取目标进程模块基址
    void *lpRemoteBaseAddr = GetModuleBase(nPid, lpLibraryPath);
    void *lpLocalBaseAddr = GetModuleBase(0, lpLibraryPath);
    void *lpRemoteFunctionAddr = NULL;

    if ((lpRemoteBaseAddr == NULL) || (lpLocalBaseAddr == NULL))
    {
        return lpRemoteFunctionAddr;
    }

    DEBUG_PRINT("[+] GetRemoteFunctionAddr: local[%p], remote[%p]\n", lpLocalBaseAddr, lpRemoteBaseAddr);

    lpRemoteFunctionAddr = (void *)((uint32_t)lpRemoteBaseAddr - (uint32_t)lpLocalBaseAddr + (uint32_t)lpFunctionAddr);

    return lpRemoteFunctionAddr;
}

/*--------------------------------------------------
*   功能:   向目标进程指定的地址中写入数据
*
*   参数:
*           nPid        需要注入的进程Pid
*           lpAddr      需要写入的目标进程地址
*           lpData      需要写入的数据缓冲区
*           nLength     需要写入的数据长度
*
*   返回值: -1
*--------------------------------------------------*/
int PtraceWriteProcessMemory(int nPid, void *lpAddr, const uint8_t *lpData, uint32_t nLength)
{
    uint32_t i, j, remain;
    uint8_t *lpDataBuff = NULL;

    union u {
        long val;
        char chars[sizeof(long)];
    } d;

    j = nLength / 4;
    remain = nLength % 4;

    lpDataBuff = lpData;

    //先4字节拷贝
    for (i = 0; i < j; i++)
    {
        memcpy(d.chars, lpDataBuff, 4);
        ptrace(PTRACE_POKETEXT, nPid, lpAddr, d.val);

        lpAddr  += 4;
        lpDataBuff += 4;
    }

    //最后不足4字节的，单字节拷贝
    if (remain > 0)
    {
        d.val = ptrace(PTRACE_PEEKTEXT, nPid, lpAddr, 0);
        for (i = 0; i < remain; i ++) {
            d.chars[i] = *lpDataBuff ++;
        }

        ptrace(PTRACE_POKETEXT, nPid, lpAddr, d.val);
    }

    return 0;
}

/*--------------------------------------------------
*   功能:   修改目标进程寄存器的值
*
*   参数:
*           nPid        需要注入的进程Pid
*           lpRegs      需要修改的新寄存器信息
*
*   返回值: -1
*--------------------------------------------------*/
int PtraceSetRegs(int nPid, struct pt_regs *lpRegs)
{
    if (ptrace(PTRACE_SETREGS, nPid, NULL, lpRegs) == -1)
    {
        return -1;
    }

    return 0;
}

/*--------------------------------------------------
*   功能:   恢复程序运行
*
*   参数:
*           nPid        需要注入的进程Pid
*
*   返回值: -1
*--------------------------------------------------*/
int PtraceContinue(int nPid)
{
    if (ptrace(PTRACE_CONT, nPid, NULL, NULL) == -1)
    {
        return -1;
    }

    return 0;
}

/*--------------------------------------------------
*   功能:   调用远程函数指针
*
*   参数:
*           nPid            需要注入的进程Pid
*           pfnFunctionAddr 调用的函数指针地址
*           lpParamArg      调用的参数
*           nParamCount     调用的参数个数
*           lpRegs          远程进程寄存器信息(ARM前4个参数由r0 ~ r3传递)
*
*   返回值: 失败返回-1
*--------------------------------------------------*/
int PtraceCallRemoteFunction(int nPid, void *pfnFunctionAddr, long *lpParamArg, int nParamCount, struct pt_regs *lpRegs)
{
    uint32_t i = 0;
    int nStatus = 0;

    //首先将前4个参数赋值给r0~r3
    for (; (i < nParamCount) && (i < 4); i++)
    {
        lpRegs->uregs[i] = lpParamArg[i];
    }

    //如果有超过4个的参数, 则将剩余参数拷贝到目标栈上
    if (i < nParamCount)
    {
        //抬高栈顶sub esp, xxx
        lpRegs->ARM_sp -= (nParamCount - i) * sizeof(long);
        if (PtraceWriteProcessMemory(nPid, (void *)lpRegs->ARM_sp, (const uint8_t *)&lpParamArg[i], (uint32_t)((nParamCount - i) * sizeof(long))) == -1)
        {
            DEBUG_PRINT("[-]PtraceCallRemoteFunction::PtraceWriteProcessMemory Error\r\n");
            return -1;
        }
    }

    //将PC的值设置为函数地址
    lpRegs->ARM_pc = pfnFunctionAddr;

    //设置ARM_cpsr寄存器的值
    if (lpRegs->ARM_pc & 1)
    {
        /* thumb */
        lpRegs->ARM_pc &= (~1u);
        lpRegs->ARM_cpsr |= CPSR_T_MASK;
    }
    else
    {
        /* arm */
        lpRegs->ARM_cpsr &= ~CPSR_T_MASK;
    }

    //设置返回地址为0的原因见下面注释
    lpRegs->ARM_lr = 0;

    //修改目标进程寄存器的值
    if (PtraceSetRegs(nPid, lpRegs) == -1)
    {
        DEBUG_PRINT("[-]PtraceCallRemoteFunction::PtraceSetRegs Error\r\n");
        return -1;
    }

    /*
        WUNTRACED告诉waitpid，如果子进程进入暂停状态，那么就立即返回。
        如果是被ptrace的子进程，那么即使不提供WUNTRACED参数，也会在子进程进入暂停状态的时候立即返回。
        对于使用ptrace_cont运行的子进程，它会在3种情况下进入暂停状态：
        ①下一次系统调用；
        ②子进程退出；
        ③子进程的执行发生错误。
        这里的0xb7f就表示子进程进入了暂停状态，且发送的错误信号为11(SIGSEGV)，它表示试图访问未分配给自己的内存, 或试图往没有写权限的内存地址写数据。
        那么什么时候会发生这种错误呢？
        显然，当子进程执行完注入的函数后，由于我们在前面设置了regs->ARM_lr = 0，它就会返回到0地址处继续执行，这样就会产生SIGSEGV了！
    */
    do
    {
        //恢复程序运行, 由于之前Attach被挂起了
        if (PtraceContinue(nPid) == -1)
        {
            DEBUG_PRINT("[-]PtraceCallRemoteFunction::PtraceContinue Error\r\n");
            return -1;
        }

        waitpid(nPid, &nStatus, WUNTRACED);

    } while(nStatus != 0xb7f);

    return 0;
}

/*--------------------------------------------------
*   功能:   调用远程函数指针
*
*   参数:
*           nPid            需要注入的进程Pid
*           lpFunctionName  调用的函数名称, 此参数仅作Debug输出用
*           pfnFunctionAddr 调用的函数指针地址
*           lpParamArg      调用的参数
*           nParamCount     调用的参数个数
*           lpRegs          远程进程寄存器信息(ARM前4个参数由r0 ~ r3传递)
*
*   返回值: 失败返回-1
*--------------------------------------------------*/
int CallRemoteFunction(int nPid, const char *lpFunctionName, void *pfnFunctionAddr, long *lpParamArg, int nParamCount, struct pt_regs *lpRegs)
{
    DEBUG_PRINT("[+] Calling %s in target process.\n", lpFunctionName);

    //call
    if (PtraceCallRemoteFunction(nPid, pfnFunctionAddr, lpParamArg, nParamCount, lpRegs) == -1)
    {
        return -1;
    }

    //获取返回值
    if (PtraceGetRegs(nPid, lpRegs) == -1)
    {
        DEBUG_PRINT("[-]CallRemoteFunction::PtraceGetRegs Error\r\n");
        return -1;
    }

    DEBUG_PRINT("[+] Target process returned from %s, return value=%p, pc=%p\r\n",
                lpFunctionName, lpRegs->ARM_r0, lpRegs->ARM_pc);
    return 0;
}

/*--------------------------------------------------
*   功能:   远程注入
*
*   参数:
*           nPid            需要注入的进程Pid
*           lpLibraryPath   需要注入的.so路径
*           lpFunctionName  .so中导出的函数名
*           lpFunctionParam 函数的参数
*
*   返回值: 注入失败返回-1
*--------------------------------------------------*/
int InjectRemoteProcess(int nPid, const char *lpLibraryPath, const char *lpFunctionName, const char *lpFunctionParam)
{
    int nRet = 0;
    void *pfnmmap = NULL;
    void *pfndlopen = NULL;
    void *pfndlsym = NULL;
    void *pfndlclose = NULL;
    void *lpMmapBase = NULL;
    struct pt_regs Regs = {0};
    struct pt_regs OldRegs = {0};
    long ParamArg[10] = {0};
    void *hSo = NULL;
    void *pfnRemoteFunction = NULL;
    void *pfnsleep = NULL;

    DEBUG_PRINT("[+] Injecting process: %d\n", nPid);

    //附加目标进程
    if (PtraceAttach(nPid) == -1)
    {
        DEBUG_PRINT("[-]PtraceAttach Error\r\n");
        return -1;
    }

    //获取保存寄存器信息, 恢复时用
    if (PtraceGetRegs(nPid, &Regs) == -1)
    {
        DEBUG_PRINT("[-]PtraceGetRegs Error\r\n");
        nRet = -1;
        goto SAFE_END;
    }

    //保存
    memcpy(&OldRegs, &Regs, sizeof(Regs));

    pfnmmap = GetRemoteFunctionAddr(nPid, LIBC_PATH, (void *)mmap);
    if (pfnmmap == NULL)
    {
        DEBUG_PRINT("[-]pfnmmap == NULL\r\n");
        nRet = -1;
        goto SAFE_END;
    }

    DEBUG_PRINT("[+] pfnmmap Addr: %p\r\n", pfnmmap);

    //申请远程空间
    //构造参数void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
    ParamArg[0] = 0;
    ParamArg[1] = 0x4000;
    ParamArg[2] = PROT_READ | PROT_WRITE | PROT_EXEC;
    ParamArg[3] = MAP_ANONYMOUS | MAP_PRIVATE;
    ParamArg[4] = 0;
    ParamArg[5] = 0;

    //调用远程函数指针
    if (CallRemoteFunction(nPid, "mmap", pfnmmap, ParamArg, 6, &Regs) == -1)
    {
        nRet = -1;
        goto SAFE_END;
    }

    //远程申请的Buffer首地址
    lpMmapBase = Regs.ARM_r0;

    pfndlopen = GetRemoteFunctionAddr(nPid, LINKER_PATH, (void *)dlopen);
    pfndlsym = GetRemoteFunctionAddr(nPid, LINKER_PATH, (void *)dlsym);
    pfndlclose = GetRemoteFunctionAddr(nPid, LINKER_PATH, (void *)dlclose);
    pfnsleep = GetRemoteFunctionAddr(nPid, LIBC_PATH, (void *)sleep);
    if ((pfndlopen == NULL) || (pfndlsym == NULL) || (pfndlclose == NULL) || (pfnsleep == NULL))
    {
        DEBUG_PRINT("[-]pfndlopen | pfndlsym | pfndlclose | pfnsleep == NULL\r\n");
        nRet = -1;
        goto SAFE_END;
    }

    DEBUG_PRINT("[+] Get imports: dlopen: %p, dlsym: %p, dlclose: %p\r\n",
                pfndlopen, pfndlsym, pfndlclose);

    printf("lpLibraryPath Length: %d\r\n", strlen(lpLibraryPath) + 1);

    //远程申请的Buffer首地址写入需要注入的so路径
    if (PtraceWriteProcessMemory(nPid, lpMmapBase, lpLibraryPath, strlen(lpLibraryPath) + 1) == -1)
    {
        DEBUG_PRINT("[-]InjectRemoteProcess::PtraceWriteProcessMemory Error\r\n");
        nRet = -1;
        goto SAFE_END;
    }

    //传递参数, 准备调用dlopen  void * dlopen(const char * pathname, int mode);
    ParamArg[0] = lpMmapBase;
    ParamArg[1] = RTLD_NOW| RTLD_GLOBAL;

    if (CallRemoteFunction(nPid, "dlopen", pfndlopen, ParamArg, 2, &Regs) == -1)
    {
        nRet = -1;
        goto SAFE_END;
    }

    printf("Fun:%s, Line:%d\r\n", __FUNCTION__, __LINE__ );

    hSo = Regs.ARM_r0;

    //传递参数, 准备调用pfndlsym  void* dlsym( void* handle, const char* name );
    ParamArg[0] = hSo;

#define FUNCTION_NAME_OFFSET    0x100
    //lpFunctionName需要写入远程的buffer中才能使用
    if (PtraceWriteProcessMemory(nPid, lpMmapBase + FUNCTION_NAME_OFFSET, lpFunctionName, strlen(lpFunctionName) + 1) == -1)
    {
        DEBUG_PRINT("[-]InjectRemoteProcess::PtraceWriteProcessMemory Error\r\n");
        nRet = -1;
        goto SAFE_END;
    }
    printf("Fun:%s, Line:%d\r\n", __FUNCTION__, __LINE__ );

    ParamArg[1] = lpMmapBase + FUNCTION_NAME_OFFSET;

    if (CallRemoteFunction(nPid, "dlsym", pfndlsym, ParamArg, 2, &Regs) == -1)
    {
        nRet = -1;
        goto SAFE_END;
    }

    printf("Fun:%s, Line:%d\r\n", __FUNCTION__, __LINE__ );
    pfnRemoteFunction = Regs.ARM_r0;
    DEBUG_PRINT("hook_entry_addr = %p\r\n", pfnRemoteFunction);

    //传递参数, 准备调用注入模块中的函数  void MyHook(void)
    if (CallRemoteFunction(nPid, lpFunctionName, pfnRemoteFunction, ParamArg, 0, &Regs) == -1)
    {
        nRet = -1;
        goto SAFE_END;
    }

    printf("Fun:%s, Line:%d\r\n", __FUNCTION__, __LINE__ );

    printf("Press enter to dlclose and detach\r\n");
//    getchar();

    //传递参数, 准备调用dlclose
    ParamArg[0] = hSo;

    if (CallRemoteFunction(nPid, "dlclose", pfndlclose, ParamArg, 1, &Regs) == -1)
    {
        nRet = -1;
    }


SAFE_END:
    //恢复原始寄存器信息
    if (PtraceSetRegs(nPid, &OldRegs) == -1)
    {
        DEBUG_PRINT("[-]PtraceSetRegs Error\r\n");
    }

    //取消附加
    if (PtraceDetach(nPid) == -1)
    {
        DEBUG_PRINT("[-]PtraceDetach Error\r\n");
    }
    printf("Fun:%s, Line:%d, ret=%d\r\n", __FUNCTION__, __LINE__, nRet );
    return nRet;
}

int main(int argc, char* argv[])
{
     DEBUG_PRINT("[+]enter main\r\n");
    int nProId = FindProIdByProName("./hello");
    if (nProId == -1)
    {
         DEBUG_PRINT("[+]FindProIdByProName Failed\r\n");
        return -1;
    }

    if (InjectRemoteProcess(nProId, "/data/local/tmp/libhello.so", "MyHook", NULL) == -1)
    {
        printf("[+]InjectRemoteProcess Failed\r\n");
    }
    else
    {
        printf("[+]InjectRemoteProcess Success\r\n");
    }

    return 0;
}