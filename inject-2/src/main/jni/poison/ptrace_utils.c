/*
 * ptrace_utils.c
 *
 *  Created on: 2013-6-26
 *      Author: boyliang
 */

#include <sys/wait.h>\
#include <pthread.h>

#include "ptrace_utils.h"

#include "tools.h"
#include "log.h"
#include "elf_utils.h"

/**
 * read registers' status
 */
int ptrace_getregs(pid_t pid, struct pt_regs* regs) {

	if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0) {

		perror("ptrace_getregs: Can not get register values");
		return -1;
	}

	return 0;
}

/**
 * set registers' status
 */
int ptrace_setregs(pid_t pid, struct pt_regs* regs) {

	if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0) {
		perror("ptrace_setregs: Can not set register values");
		return -1;
	}

	return 0;
}



/**
 * attach to target process 附加目标进程
 */
int ptrace_attach(pid_t pid, int zygote) {

	if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {

		LOGE("ptrace_attach");
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);

	/*
	 * Restarts  the stopped child as for PTRACE_CONT, but arranges for
	 * the child to be stopped at the next entry to or exit from a sys‐
	 * tem  call,  or  after execution of a single instruction, respec‐
	 * tively.
	 */
	if (ptrace(PTRACE_SYSCALL, pid, NULL, 0) < 0) {

		LOGE("ptrace_syscall");
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED);

	// 针对zygote进程的特殊处理
	if (zygote) {

		// 当进程为zygote时，需要考虑为zygote进程解除阻塞状态，使进程注入得以进行
		connect_to_zygote(NULL);
	}

	// 当目标进程在下次进/出系统调用时被附加调试
	if (ptrace(PTRACE_SYSCALL, pid, NULL, NULL ) < 0) {
		LOGE("ptrace_syscall");
		return -1;
	}

	// 等待进程附加操作返回
	waitpid(pid, NULL, WUNTRACED);

	return 0;
}

/**
 * detach from target process
 */
int ptrace_detach( pid_t pid )
{
    if ( ptrace( PTRACE_DETACH, pid, NULL, 0 ) < 0 )
    {
    	LOGE( "ptrace_detach" );
        return -1;
    }

    return 0;
}


int ptrace_continue(pid_t pid) {

	if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {

		LOGE("ptrace_cont");
		return -1;
	}

	return 0;
}

int ptrace_syscall(pid_t pid) {

	return ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
}

/**
 * write data to dest 向目标pid进程中写入数据（4字节对齐）
 */
int ptrace_write(pid_t pid, uint8_t *dest, uint8_t *data, size_t size) {

	uint32_t i, j, remain;
	uint8_t *laddr;

	union u {
		long val;
		char chars[sizeof(long)];
	} d;

	j = size / 4;
	remain = size % 4;

	laddr = data;

	for (i = 0; i < j; i++) {

		memcpy(d.chars, laddr, 4);
		ptrace(PTRACE_POKETEXT, pid, (void *)dest, (void *)d.val);

		dest += 4;
		laddr += 4;
	}

	if (remain > 0) {

		d.val = ptrace(PTRACE_PEEKTEXT, pid, (void *)dest, NULL);
		for (i = 0; i < remain; i++) {

			d.chars[i] = *laddr++;
		}

		ptrace(PTRACE_POKETEXT, pid, (void *)dest, (void *)d.val);

	}

	return 0;
}

// 从目标pid进程中读取数据（4字节对齐）
int ptrace_read( pid_t pid,  uint8_t *src, uint8_t *buf, size_t size ) {
    uint32_t i, j, remain;
    uint8_t *laddr;

    union u {
        long val;
        char chars[sizeof(long)];
    } d;

    j = size / 4;
    remain = size % 4;

    laddr = buf;

    for ( i = 0; i < j; i ++ )
    {
        d.val = ptrace( PTRACE_PEEKTEXT, pid, src, 0 );
        memcpy( laddr, d.chars, 4 );
        src += 4;
        laddr += 4;
    }

    if ( remain > 0 )
    {
        d.val = ptrace( PTRACE_PEEKTEXT, pid, src, 0 );
        memcpy( laddr, d.chars, remain );
    }

    return 0;
}

#if defined(__arm__)
int ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs){
    uint32_t i;

    // 设置目标pid进程中被调用的函数的参数（arm的函数调用中前4个函数参数，通过r0-r3寄存器传递）
    for (i = 0; i < num_params && i < 4; i ++) {
        regs->uregs[i] = params[i];
    }

    //
    // push remained params onto stack
    // 设置目标pid进程中被调用的函数的超过4个参数的参数（arm的函数调用中超过4个参数之后的函数参数，通过栈进行传递）
    if (i < num_params) {
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;
        ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)params[i], (num_params - i) * sizeof(long));
    }

    // 设置将被调用的函数的调用地址（pc为指令指针寄存器，控制着进程的具体执行）
    regs->ARM_pc = addr;

    // 根据当前进程的运行模式，设置进程的状态寄存器cpsr的值
    if (regs->ARM_pc & 1) {
        /* thumb */
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= CPSR_T_MASK;
    } else {
        /* arm */
        regs->ARM_cpsr &= ~CPSR_T_MASK;
    }

    // 设置函数调用完的返回地址为0，触发地址0异常，程序的控制权又从目标pid进程回到了当前进程中
    regs->ARM_lr = 0;

    // 设置目标pid进程的寄存器的状态值--实现在目标pid进程中调用指定的目标函数
    if (ptrace_setregs(pid, regs) == -1
        // 让目标pid进程继续执行代码指令
        || ptrace_continue(pid) == -1) {

        printf("error\n");
        return -1;
    }

    int stat = 0;

    // 等待在目标pid进程中，调用指定的目标函数完成
    waitpid(pid, &stat, WUNTRACED);

    /***
     WUNTRACED告诉waitpid，如果子进程进入暂停状态，那么就立即返回。
     如果是被ptrace的子进程，那么即使不提供WUNTRACED参数，也会在子进程进入暂停状态的时候立即返回。
     对于使用PTRACE_CONT运行的子进程，它会在3种情况下进入暂停状态：①下一次系统调用；②子进程退出；③子进程的执行发生错误。
     这里的0xb7f就表示子进程进入了暂停状态，且发送的错误信号为11(SIGSEGV)，它表示试图访问未分配给自己的内存, 或试图往没有写权限的内存地址写数据。
     那么什么时候会发生这种错误呢？
     显然，当子进程执行完注入的函数后，由于我们在前面设置了regs->ARM_lr = 0，它就会返回到0地址处继续执行，这样就会产生SIGSEGV。
     ***/
    while (stat != 0xb7f) {
        if (ptrace_continue(pid) == -1) {

            printf("error\n");
            return -1;
        }

        // 进程等待
        waitpid(pid, &stat, WUNTRACED);
    }

    return 0;
}

// x86模式下，在目标pid进程中调用指定目标函数
#elif defined(__i386__)
long ptrace_call(pid_t pid, uint32_t addr, long *params, uint32_t num_params, struct pt_regs* regs) {
	// x86模式下，C函数调用约定一般是通过栈进行传递参数
	// 抬高目标pid进程的栈顶，用于保存函数调用需要的参数
	regs->esp -= (num_params) * sizeof(long) ;
	// 将指定目标函数调用需要的函数参数，写入到函数栈中
	ptrace_write(pid, (void *)regs->esp, (uint8_t *)params, (num_params) * sizeof(long));

	// 无效的地址
	long tmp_addr = 0x00;
	// 再次抬高栈顶，用于保存指定目标函数调用完后的函数的返回地址，此处设置为 0x00，将触发无效0地址访问异常
	// 目标pid进程中指定目标函数调用完成以后，进程的控制权从目标pid进程又回到了当前进程中
	regs->esp -= sizeof(long);
	// 将无效的0地址值，写入到指定目标函数的函数栈中
	ptrace_write(pid, regs->esp, (char *)&tmp_addr, sizeof(tmp_addr));

	// x86模式下，控制进程的执行流程的是eip寄存器
	// 设置eip为将被调用的函数的调用地址
	regs->eip = addr;

	// 设置目标pid进程的寄存器状态值，用以调用目标pid进程中的指定目标函数
	if (ptrace_setregs(pid, regs) == -1
		// 让目标pid进程继续执行指令，调用目标函数
		|| ptrace_continue( pid) == -1) {
		printf("error\n");
		return -1;
	}

	int stat = 0;
	// 等待调用目标函数完成
	waitpid(pid, &stat, WUNTRACED);
	// 对0地址异常的处理
	while (stat != 0xb7f) {
		if (ptrace_continue(pid) == -1) {
			printf("error\n");
			return -1;
		}
		// 等待操作的完成
		waitpid(pid, &stat, WUNTRACED);
	}
	return 0;
}
#else
#error "Not supported"
#endif

//static void* thread_connect_to_zygote(void* arg){
//	int s, len;
//	struct sockaddr_un remote;
//
//	LOGI("[+] wait 2s...");
//	sleep(2);
//
//	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) != -1) {
//		remote.sun_family = AF_UNIX;
//		strcpy(remote.sun_path, "/dev/socket/zygote");
//		len = strlen(remote.sun_path) + sizeof(remote.sun_family);
//		LOGI("[+] start to connect zygote socket");
//		connect(s, (struct sockaddr *) &remote, len);
//		LOGI("[+] close socket");
//		close(s);
//	}
//
//	return NULL ;
//}


// 当目标pid进程为zygote时，加载so库文件之前，需要的测试处理
static int zygote_special_process(pid_t target_pid){

	LOGI("[+] zygote process should special take care. \n");

	struct pt_regs regs;

	// 获取目标pid进程的寄存器的状态值
	if (ptrace_getregs(target_pid, &regs) == -1)
		return -1;

	// 获取目标pid进程的getpid函数的调用地址
	void* remote_getpid_addr = get_remote_address(target_pid, getpid);
	LOGI("[+] Remote getpid addr %p.\n", remote_getpid_addr);

	// 判断获取目标pid进程的getpid函数的调用地址是否成功
	if(remote_getpid_addr == NULL){

		return -1;
	}

	pthread_t tid = 0;
	// 创建线程再次调用connect_to_zygote解除zygote进程的阻塞状态
	pthread_create(&tid, NULL, connect_to_zygote, NULL);
	// 释放线程
	pthread_detach(tid);

	// 调用目标pid进程中的getpid函数
	if (ptrace_call(target_pid, remote_getpid_addr, NULL, 0, &regs) == -1) {

		LOGE("[-] Call remote getpid fails");
		return -1;
	}

	// 获取上面的函数调用完后目标pid进程的寄存器的状态，主要是为了获取getpid函数的返回值
	if (ptrace_getregs(target_pid, &regs) == -1)
		return -1;

	// 打印调用getpid函数完后，目标pid进程的寄存器的状态
	LOGI("[+] Call remote getpid result r0=%x, r7=%x, pc=%x, \n", regs.ARM_r0, regs.ARM_r7, regs.ARM_pc);

	return 0;
}

// 调用目标pid进程的dlopen函数加载指定的so库文件，并返回加载的模块的基址
void* ptrace_dlopen(pid_t target_pid, void* remote_dlopen_addr, const char*  filename){

	struct pt_regs regs;

	// 获取目标pid进程的寄存器的状态值
	if (ptrace_getregs(target_pid, &regs) == -1)
		return NULL ;

	// 判断目标pid进程是否是zygote进程;如果是，加载so库文件之前，进行相应的测试处理
	if (strcmp("zygote", get_process_name(target_pid)) == 0 && zygote_special_process(target_pid) != 0) {

		return NULL ;
	}

	// 在目标pid进程中调用dlopen函数需要的参数
	long mmap_params[2];

	// filename为将要加载到目标pid进程中的so的路径字符串
	// 要将filename字符串写入到目标pid进程中，filename_len即为需要分配的内存空间的大小
	size_t filename_len = strlen(filename) + 1;

	// 调用目标pid进程的mmap函数申请内存空间，用以保存filename字符串（即将要加载的so文件的路径）
	void* filename_addr = find_space_by_mmap(target_pid, filename_len);
	// 判断在目标pid进程是否调用mmap函数分配内存空间成功
	if (filename_addr == NULL ) {

		LOGE("[-] Call Remote mmap fails.\n");
		return NULL ;
	}

	// 将filename字符串（即将要加载的so文件的路径）写入到目标pid进程的内存地址filename_addr中
	ptrace_write(target_pid, (uint8_t *)filename_addr, (uint8_t *)filename, filename_len);

	// dlopen函数的参数--需要加载的so文件的路径字符串
	mmap_params[0] = (long)filename_addr;
	// dlopen函数的参数--flag，加载的要求
	mmap_params[1] = RTLD_NOW | RTLD_GLOBAL;

	// 获取目标pid进程中的dlopen函数的调用地址（调用参数已经准备好）
	remote_dlopen_addr = (remote_dlopen_addr == NULL) ? get_remote_address(target_pid, (void *)dlopen) : remote_dlopen_addr;
	if (remote_dlopen_addr == NULL) {

		LOGE("[-] Get Remote dlopen address fails.\n");
		return NULL;
	}

	// 在目标pid进程调用dlopen函数，加载filename_addr指定的so库文件
	if (ptrace_call(target_pid, (uint32_t) remote_dlopen_addr, mmap_params, 2, &regs) == -1)
		return NULL;

	// 获取目标pid进程的寄存器的状态值，主要是为了获取上面 dlopen函数调用的返回值
	if (ptrace_getregs(target_pid, &regs) == -1)
		return NULL;

	LOGI("[+] Target process returned from dlopen, return r0=%x, r7=%x, pc=%x, \n", regs.ARM_r0, regs.ARM_r7, regs.ARM_pc);

	// 返回目标pid进程中调用dlopen函数的返回的内存加载的模块基址
	return ptrace_ip(&regs) == 0 ? (void *)ptrace_retval(&regs):NULL;
}

// 对x86和arm模式的函数的返回值兼容处理（获取函数调用的返回值）
long ptrace_retval(struct pt_regs * regs) {
#if defined(__arm__)
	return regs->ARM_r0;
#elif defined(__i386__)
	return regs->eax;
#else
#error "Not supported"
#endif
}

// 对x86和arm模式的指令指针的处理（获取函数调用完后的，指令指针值）
long ptrace_ip(struct pt_regs * regs) {
#if defined(__arm__)
	return regs->ARM_pc;
#elif defined(__i386__)
	return regs->eip;
#else
#error "Not supported"
#endif
}


