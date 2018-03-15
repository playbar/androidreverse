/*
 *  ptrace_utils.h
 *  Author: boyliang
 *
 *  Modify on: 2017-7-7
 *  Modify by syang
 */

#ifndef PTRACE_UTILS_H_
#define PTRACE_UTILS_H_

#include <sys/user.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#if defined(__i386__)
#define pt_regs	user_regs_struct
#endif

#define CPSR_T_MASK		( 1u << 5 )

int ptrace_getregs(pid_t pid, struct pt_regs* regs);

int ptrace_setregs(pid_t pid, struct pt_regs* regs);

int ptrace_attach( pid_t pid , int zygote);

int ptrace_detach( pid_t pid );

int ptrace_continue(pid_t pid);

int ptrace_syscall(pid_t pid);

int ptrace_write(pid_t pid, uint8_t *dest, uint8_t *data, size_t size);

int ptrace_read( pid_t pid,  uint8_t *src, uint8_t *buf, size_t size );

#if defined(__arm__)
int ptrace_call(pid_t pid, uint32_t addr, long *params, int num_params, struct pt_regs* regs);
#elif defined(__i386__)
long ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs);
#else
#error "Not supported"
#endif

void* ptrace_dlopen(pid_t target_pid, void* remote_dlopen_addr, const char*  filename);

// 对x86和arm模式的函数的返回值兼容处理（获取函数调用的返回值）
long ptrace_retval(struct pt_regs *regs);
// 对x86和arm模式的指令指针的处理（获取函数调用完后的，指令指针值）
long ptrace_ip(struct pt_regs *regs);

#endif /* PTRACE_UTILS_H_ */
