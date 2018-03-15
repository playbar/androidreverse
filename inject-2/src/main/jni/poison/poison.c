#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <asm/ptrace.h>

#include "ptrace_utils.h"
#include "elf_utils.h"
#include "log.h"
#include "tools.h"

struct process_hook {
    // 被注入的目标进程
    pid_t pid;
    // 注入到目标进程中so文件的路径
    char *dso;
//	void		*dlopen_addr;
//	void 		*dlsym_addr;
//	void		*mmap_addr;
} process_hook = {0, "", NULL, NULL, NULL};


// 主函数
int main(int argc, char *argv[]) {

    // 对传入的参数的个数进行判断（要求3个参数）
    if (argc < 2)
        exit(0);

    // 保存寄存器的状态信息
    struct pt_regs regs;

    // 获取注入到目标进程中的so的文件的路径
    process_hook.dso = strdup(argv[1]);
    // 获取注入的目标进程的pid
    process_hook.pid = atoi(argv[2]);

//	process_hook.dlopen_addr = (void *)atol(argv[3]);
//	process_hook.dlsym_addr = (void *)atol(argv[4]);
//	process_hook.mmap_addr = (void *)atol(argv[5]);

    // 判断注入到目标进程中so是否存在并且具有可读可执行权限
    if (access(process_hook.dso, R_OK | X_OK) < 0) {

        LOGE("[-] so file must chmod rx\n");
        return 1;
    }

    // 获取指定pid进程的名称
    const char *process_name = get_process_name(process_hook.pid);

    // 附加目标进程
    ptrace_attach(process_hook.pid, strstr(process_name, "zygote"));

    // 打印附加目标进程的信息
    LOGI("[+] ptrace attach to [%d] %s\n", process_hook.pid, get_process_name(process_hook.pid));

    // 读取此时目标进程中所有的寄存器的状态信息
    if (ptrace_getregs(process_hook.pid, &regs) < 0) {

        LOGE("[-] Can't get regs %d\n", errno);

        // 读取失败跳转
        goto DETACH;
    }

    // 打印目标进程的寄存器pc和R7的信息
#if defined(__arm__)

    LOGI("[+] pc: %x, r7: %d", regs.ARM_pc, regs.ARM_r7);
#elif defined(__i386__)
    LOGI("[+] pc: %x, r7: %d", regs.eip;, regs.ARM_r7);
#else
#endif

    // dlsym参数为当前进程中的调用地址，获取目标pid进程中dlsy函数的调用地址
    void *remote_dlsym_addr = get_remote_address(process_hook.pid, (void *) dlsym);
    // 获取目标pid进程中dlopen函数的调用地址
    void *remote_dlopen_addr = get_remote_address(process_hook.pid, (void *) dlopen);

    // 打印目标进程的函数dlopen和dlsym的调用地址
    LOGI("[+] remote_dlopen address %p\n", remote_dlopen_addr);
    LOGI("[+] remote_dlsym  address %p\n", remote_dlsym_addr);

    // 调用目标pid进程的dlopen函数加载指定的so库文件，获取返回的加载的模块的基址
    if (ptrace_dlopen(process_hook.pid, remote_dlopen_addr, process_hook.dso) == NULL) {

        LOGE("[-] Ptrace dlopen fail. %s\n", dlerror());
    }

    // 针对此时不同的模式，设置目标pid进程的CPSR寄存器的值
    if (regs.ARM_pc & 1) {
        // thumb
        regs.ARM_pc &= (~1u);
        regs.ARM_cpsr |= CPSR_T_MASK;
    } else {
        // arm
        regs.ARM_cpsr &= ~CPSR_T_MASK;
    }

    // 恢复目标pid进程的寄存器的状态即恢复到注入前的运行状态
    if (ptrace_setregs(process_hook.pid, &regs) == -1) {

        LOGE("[-] Set regs fail. %s\n", strerror(errno));
        // 失败进行跳转
        goto DETACH;
    }

    // 打印注入成功的消息
    LOGI("[+] Inject success!\n");

    DETACH:
    // 结束对目标pid进程的附加
    ptrace_detach(process_hook.pid);

    // 打印注入工作完成的消息
    LOGI("[+] Inject done!\n");

    return 0;
}
