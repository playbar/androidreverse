/*
 *  Collin's Binary Instrumentation Tool/Framework for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *  http://www.mulliner.org/android/
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <android/log.h>

#include <jni.h>
#include <stdlib.h>

#include "exportutil.h"
#include "exporthook.h"

//void __attribute__ ((constructor)) my_init(void);
//系统调用__ARM_NR_cacheflush，因为cpu有指令缓存功能，所以当我们改了一段地址的指令以后要
//用该系统调用来刷新这一段地址的缓存，否则修改可能不会生效
//android源码Android\bionic\libc\arch-mips\bionic\cacheflush.c可以查看参数的含义
void inline hook_cacheflush(unsigned int begin, unsigned int end)
{	
	//r0 r1 r2传递前三个参数，r7保存系统调用号
	const int syscall = 0xf0002;
	__asm __volatile (
		"mov	 r0, %0\n"			
		"mov	 r1, %1\n"
		"mov	 r7, %2\n"
		"mov     r2, #0x0\n"
		"svc     0x00000000\n"
		:
		:	"r" (begin), "r" (end), "r" (syscall)
		:	"r0", "r1", "r7"
		);
}

int hook_direct(struct hook_t *h, unsigned int addr, void *hookf)
{
	int i;
	
	log("addr  = %x\n", addr)
	log("hookf = %lx\n", (unsigned long)hookf)

	if ((addr % 4 == 0 && (unsigned int)hookf % 4 != 0) || (addr % 4 != 0 && (unsigned int)hookf % 4 == 0))
		log("addr 0x%x and hook 0x%lx\n don't match!\n", addr, (unsigned long)hookf)
	
	//log("ARM\n")
	h->thumb = 0;
	h->patch = (unsigned int)hookf;
	h->orig = addr;
	log("orig = %x\n", h->orig)
	h->jump[0] = 0xe59ff000; // LDR pc, [pc, #0]
	h->jump[1] = h->patch;
	h->jump[2] = h->patch;
	for (i = 0; i < 3; i++)
		h->store[i] = ((int*)h->orig)[i];
	for (i = 0; i < 3; i++)
		((int*)h->orig)[i] = h->jump[i];
	
	hook_cacheflush((unsigned int)h->orig, (unsigned int)h->orig+sizeof(h->jumpt));
	return 1;
}

int exporthook(struct hook_t *h, int pid, char *libname, char *funcname, void *hook_arm, void *hook_thumb)
{
	unsigned long int addr;
	int i;

	if (find_name(pid, funcname, libname, &addr) < 0) {
		log("can't find: %s\n", funcname)
		return 0;
	}
	
	log("hooking:   %s = 0x%lx ", funcname, addr)
	strncpy(h->name, funcname, sizeof(h->name)-1);

	if (addr % 4 == 0) {
		log("ARM using 0x%lx\n", (unsigned long)hook_arm)
		h->thumb = 0;
		h->patch = (unsigned int)hook_arm;
		h->orig = addr;
		//LDR pc, [pc, #0]，既把h->jump[1]的内容写入PC，则发生了跳转；
		//并且本地函数与被hook的函数参数一致，所以可以正确得到栈里面的参数
		h->jump[0] = 0xe59ff000; 
		h->jump[1] = h->patch;
		h->jump[2] = h->patch;
		for (i = 0; i < 3; i++)
			h->store[i] = ((int*)h->orig)[i];
		for (i = 0; i < 3; i++)
			((int*)h->orig)[i] = h->jump[i];
	}
	else {
		if ((unsigned long int)hook_thumb % 4 == 0)
			log("warning hook is not thumb 0x%lx\n", (unsigned long)hook_thumb)
		h->thumb = 1;
		log("THUMB using 0x%lx\n", (unsigned long)hook_thumb)
		h->patch = (unsigned int)hook_thumb;
		h->orig = addr;	
		h->jumpt[1] = 0xb4;
		h->jumpt[0] = 0x60; // push {r5,r6}
		h->jumpt[3] = 0xa5;
		h->jumpt[2] = 0x03; // add r5, pc, #12
		h->jumpt[5] = 0x68;
		h->jumpt[4] = 0x2d; // ldr r5, [r5]
		h->jumpt[7] = 0xb0;
		h->jumpt[6] = 0x02; // add sp,sp,#8
		h->jumpt[9] = 0xb4;
		h->jumpt[8] = 0x20; // push {r5}
		h->jumpt[11] = 0xb0;
		h->jumpt[10] = 0x81; // sub sp,sp,#4
		h->jumpt[13] = 0xbd;
		h->jumpt[12] = 0x20; // pop {r5, pc}
		h->jumpt[15] = 0x46;
		h->jumpt[14] = 0xaf; // mov pc, r5 ; just to pad to 4 byte boundary
		memcpy(&h->jumpt[16], (unsigned char*)&h->patch, sizeof(unsigned int));
		unsigned int orig = addr - 1; // sub 1 to get real address
		for (i = 0; i < 20; i++) {
			h->storet[i] = ((unsigned char*)orig)[i];
			//log("%0.2x ", h->storet[i])
		}
		//log("\n")
		for (i = 0; i < 20; i++) {
			((unsigned char*)orig)[i] = h->jumpt[i];
			//log("%0.2x ", ((unsigned char*)orig)[i])
		}
	}
	hook_cacheflush((unsigned int)h->orig, (unsigned int)h->orig+sizeof(h->jumpt));
	return 1;
}

int hook_lwp(int pid, char *libname, char *funcname, void *hook_arm, void **origFunc) {
	unsigned long int addr;
	struct hook_t hookT;
	struct hook_t *h = &hookT;
	int i;

	if (find_name(pid, funcname, libname, &addr) < 0) {
		log("can't find: %s\n", funcname)
		return 0;
	}
	
	log("hooking:   %s = 0x%lx ", funcname, addr)
	strncpy(h->name, funcname, sizeof(h->name)-1);

	if (addr % 4 == 0) {
		log("ARM using 0x%lx\n", (unsigned long)hook_arm)
		h->thumb = 0;
		h->patch = (unsigned int)hook_arm;
		h->orig = addr;
		//LDR pc, [pc, #0]，既把h->jump[1]的内容写入PC，则发生了跳转；
		//并且本地函数与被hook的函数参数一致，所以可以正确得到栈里面的参数
		h->jump[0] = 0xe59ff000; 
		h->jump[1] = h->patch;
		h->jump[2] = h->patch;
		for (i = 0; i < 3; i++)
			h->store[i] = ((int*)h->orig)[i];
		for (i = 0; i < 3; i++)
			((int*)h->orig)[i] = h->jump[i];

		//mmap memory to replace origFunc entry
		void *mmap_base = mmap(0, 0x20, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		log("mmap_base:0x%lx\n", (unsigned long)mmap_base);

		//write instrucment
		int i;
		for (i = 0; i < 3; ++i)
		{
			((int*)mmap_base)[i] = h->store[i];
		}


		//jump
		((int*)mmap_base)[3] = 0xe59ff000;
		((int*)mmap_base)[4] = h->orig + 12;
		((int*)mmap_base)[5] = h->orig + 12;
		*origFunc = mmap_base;
		log("origFunc:0x%lx, jump:0x%lx\n", *origFunc, h->orig + 12);
	}
	else {
		log("//!!!TODO THUMB!!!\n")
		// if ((unsigned long int)hook_thumb % 4 == 0)
		// 	log("warning hook is not thumb 0x%lx\n", (unsigned long)hook_thumb)
		// h->thumb = 1;
		// log("THUMB using 0x%lx\n", (unsigned long)hook_thumb)
		// h->patch = (unsigned int)hook_thumb;
		// h->orig = addr;	
		// h->jumpt[1] = 0xb4;
		// h->jumpt[0] = 0x60; // push {r5,r6}
		// h->jumpt[3] = 0xa5;
		// h->jumpt[2] = 0x03; // add r5, pc, #12
		// h->jumpt[5] = 0x68;
		// h->jumpt[4] = 0x2d; // ldr r5, [r5]
		// h->jumpt[7] = 0xb0;
		// h->jumpt[6] = 0x02; // add sp,sp,#8
		// h->jumpt[9] = 0xb4;
		// h->jumpt[8] = 0x20; // push {r5}
		// h->jumpt[11] = 0xb0;
		// h->jumpt[10] = 0x81; // sub sp,sp,#4
		// h->jumpt[13] = 0xbd;
		// h->jumpt[12] = 0x20; // pop {r5, pc}
		// h->jumpt[15] = 0x46;
		// h->jumpt[14] = 0xaf; // mov pc, r5 ; just to pad to 4 byte boundary
		// memcpy(&h->jumpt[16], (unsigned char*)&h->patch, sizeof(unsigned int));
		// unsigned int orig = addr - 1; // sub 1 to get real address
		// for (i = 0; i < 20; i++) {
		// 	h->storet[i] = ((unsigned char*)orig)[i];
		// 	//log("%0.2x ", h->storet[i])
		// }
		// //log("\n")
		// for (i = 0; i < 20; i++) {
		// 	((unsigned char*)orig)[i] = h->jumpt[i];
		// 	//log("%0.2x ", ((unsigned char*)orig)[i])
		// }
	}

	hook_cacheflush((unsigned int)h->orig, (unsigned int)h->orig+sizeof(h->jumpt));
	return 1;	
}

//把原始的指令写回去
void hook_precall(struct hook_t *h)
{
	int i;
	
	if (h->thumb) {
		unsigned int orig = h->orig - 1;
		for (i = 0; i < 20; i++) {
			((unsigned char*)orig)[i] = h->storet[i];
		}
	}
	else {
		for (i = 0; i < 3; i++)
			((int*)h->orig)[i] = h->store[i];
	}	
	hook_cacheflush((unsigned int)h->orig, (unsigned int)h->orig+sizeof(h->jumpt));
}

//写入修改后的跳转指令
void hook_postcall(struct hook_t *h)
{
	int i;
	
	if (h->thumb) {
		unsigned int orig = h->orig - 1;
		for (i = 0; i < 20; i++)
			((unsigned char*)orig)[i] = h->jumpt[i];
	}
	else {
		for (i = 0; i < 3; i++)
			((int*)h->orig)[i] = h->jump[i];
	}
	hook_cacheflush((unsigned int)h->orig, (unsigned int)h->orig+sizeof(h->jumpt));	
}

void unhook(struct hook_t *h)
{
	log("unhooking %s = %x  hook = %x ", h->name, h->orig, h->patch)
	hook_precall(h);
}

/*
 *  workaround for blocked socket API when process does not have network
 *  permissions
 *
 *  this code simply opens a pseudo terminal (pty) which gives us a
 *  file descriptor. the pty then can be used by another process to
 *  communicate with our instrumentation code. an example program
 *  would be a simple socket-to-pty-bridge
 *  
 *  this function just creates and configures the pty
 *  communication (read, write, poll/select) has to be implemented by hand
 *
 */
int start_coms(int *coms, char *ptsn)
{
	if (!coms) {
		log("coms == null!\n")
		return 0;
	}

	*coms = open("/dev/ptmx", O_RDWR|O_NOCTTY);
	if (*coms <= 0) {
		log("posix_openpt failed\n")
		return 0;
	}
	//else
	//	log("pty created\n")
	if (unlockpt(*coms) < 0) {
		log("unlockpt failed\n")
		return 0;
	}

	if (ptsn)
		strcpy(ptsn, (char*)ptsname(*coms));

	struct termios  ios;
	tcgetattr(*coms, &ios);
	ios.c_lflag = 0;  // disable ECHO, ICANON, etc...
	tcsetattr(*coms, TCSANOW, &ios);

	return 1;
}
