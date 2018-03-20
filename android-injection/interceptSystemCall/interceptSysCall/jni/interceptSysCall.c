#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>

#define UNIT_SIZE 4
union unit
{
	uint32_t int_val;
	char char_array[UNIT_SIZE];
};

void reverse(uint8_t *buf, uint32_t len)
{
	uint32_t i = 0, j = 0;
	uint8_t temp;
	for (i = 0, j = len - 1; i <= j; ++i, --j)
	{
		temp = buf[i];
		buf[i] = buf[j];
		buf[j] = temp;
	}
}

void peek_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len) //从起始位置为addr处，取len个字节存入缓冲区buf。
{
	uint32_t i = 0;
	uint32_t j = len / UNIT_SIZE;
	uint8_t *tmp_buf_ptr = buf;
	union unit peek_data;

	for (i = 0; i < j; i++)
	{
		peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL); //从指定地址取出4个字节
		memcpy(tmp_buf_ptr, peek_data.char_array, UNIT_SIZE);
		tmp_buf_ptr += UNIT_SIZE;
	}

	j = len % UNIT_SIZE;
	if (j != 0)
	{
		peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL);
		memcpy(tmp_buf_ptr, peek_data.char_array, j);
	}
}
void poke_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len) //将buf写入起始位置为addr处的内存中，长度为len个字节。
{

	uint32_t i = 0;
	uint32_t j = len / UNIT_SIZE;
	uint8_t *tmp_buf_ptr = buf;
	union unit poke_data;

	for (i = 0; i < j; i++)
	{
		memcpy(poke_data.char_array, tmp_buf_ptr, UNIT_SIZE);
		ptrace(PTRACE_POKEDATA, pid, addr + i * UNIT_SIZE, poke_data.int_val); //向指定地址写入4个字节
		tmp_buf_ptr += UNIT_SIZE;
	}

	j = len % UNIT_SIZE;
	if (j != 0)
	{
		union unit tmp_peek_data;
		tmp_peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL);
		memcpy(poke_data.char_array, tmp_buf_ptr, j);
		memcpy(poke_data.char_array + j, tmp_peek_data.char_array + j, UNIT_SIZE - j);
		ptrace(PTRACE_POKEDATA, pid, addr + i * UNIT_SIZE, poke_data.int_val);
	}
}

int get_syscall_number(pid_t pid, struct pt_regs *regs)
{
	ptrace(PTRACE_GETREGS, pid, NULL, regs); //get registers
	uint32_t swi = ptrace(PTRACE_PEEKTEXT, pid, (void *) (regs->ARM_pc - 4), NULL);
	if (swi == 0)
		return 0;
	int syscall_number = -1;
	//swi指令高4位是条件码，27~24位为f，23~0位是调用号.
	//对于EABI指令，条件码为e,调用号存在r7,用法如下：
	//mov r7, #num
	//swi 0x0
	if (swi == 0xef000000) //Android only supports eabi
	{
		syscall_number = regs->ARM_r7;
	}
	return syscall_number;
}

int flags = 0;
void intercept_syscall(pid_t pid)
{
	struct pt_regs regs;
	int syscall_number = get_syscall_number(pid, &regs);
	if (syscall_number <= 0)
	{
		return;
	}

	if (syscall_number == __NR_write)
	{ //printf() eventually call write(fd,str,size)
	  //fd=regs.ARM_r0
	  //str<-regs.ARM_r1
	  //size=regs.ARM_r2
		if (flags == 0) // when entering write()
		{
			//read write string to write_buf
			size_t write_size = (regs.ARM_r2 + 1) * sizeof(char);
			uint8_t *write_buf = (uint8_t *) malloc(write_size);
			peek_data(pid, regs.ARM_r1, write_buf, write_size); //get the string
			write_buf[write_size - 1] = '\0';

			reverse(write_buf, write_size - 1); //reverse the string

			poke_data(pid, regs.ARM_r1, write_buf, write_size); //reset the string

			printf("Reversed str.\n");

			free(write_buf);
			flags = 1;
		} else
		{ // when write() returns
			//return value = regs.ARM_r0
			flags = 0;
		}
		return;
	}
}

int main(int argc, char*argv[])
{
	if (argc != 2)
	{
		printf("Usage:%s <pid to be traced>\n", argv[0]);
		return -1;
	}
	pid_t target_pid = atoi(argv[1]);

	//attach process
	if (0 != ptrace(PTRACE_ATTACH, target_pid, NULL, NULL))
	{
		printf("Attach process failed:%s.\n", strerror(errno));
		return -1;
	}

	//intercept system call
	int status; //子进程状态
	while (1)
	{
		int ret = wait(&status); //暂停父进程，知道被调试的进程切换为停止状态或者子进程结束
		if (-1 == ret)
		{
			printf("wait() error:%s.\n", strerror(errno));
			return -1;
		}
		if (WIFEXITED(status)) //returns true if the child terminated nomally.
			break;
		intercept_syscall(target_pid);
		//continue subprocess until system call is called
		ptrace(PTRACE_SYSCALL, target_pid, NULL, NULL); //当被调试进程调用系统调用开始或结束时，被调试进程暂停
	}

	//detach process
	if (0 != ptrace(PTRACE_DETACH, target_pid, NULL, NULL))
	{
		printf("Detach error:%s.\n", strerror(errno));
		return -1;
	}
	return 0;
}
