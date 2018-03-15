/*
 * elf_utils.c
 *
 *  Created on: 2013-6-25
 *      Author: boyliang
 */


#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>

#include "tools.h"
#include "elf_utils.h"
#include "log.h"


// 获取目标pid进程中指定so模块的加载基址
void* get_module_base(pid_t pid, const char* module_name) {

	FILE *fp;
	long addr = 0;
	char *pch;
	char filename[32];
	char line[1024];

	if (pid < 0) {

		/* self process */
		snprintf(filename, sizeof(filename), "/proc/self/maps");
	} else {

		snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
	}

	fp = fopen(filename, "r");

	if (fp != NULL) {

		while (fgets(line, sizeof(line), fp)) {

			// 判断是否是在目标pid进程的内存中要查找到的so模块
			if (strstr(line, module_name)) {

				pch = strtok(line, "-");
				// 获取目标pid进程中指定模块的基址
				addr = strtoul(pch, NULL, 16);

				if (addr == 0x8000)
					addr = 0;

				break;
			}
		}

		fclose(fp);
	}

	return (void *) addr;
}

// 在目标pid进程的内存空间中申请内存，申请成功返回的内存地址保存在r0中
void* find_space_by_mmap(int target_pid, int size) {

	struct pt_regs regs;

	// 获取目标pid进程的寄存器的状态
	if (ptrace_getregs(target_pid, &regs) == -1)
		return 0;

	long parameters[10];

	/* call mmap */
	parameters[0] = 0;  // addr
	parameters[1] = size; // size
	parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
	parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE; // flags
	parameters[4] = 0; //fd
	parameters[5] = 0; //offset

	// 获取目标pid进程中的mmap函数的调用地址
	void *remote_mmap_addr = get_remote_address(target_pid, get_method_address("/system/lib/libc.so", "mmap"));
	LOGI("[+] Calling mmap in target process. mmap addr %p.\n", remote_mmap_addr);

	if (remote_mmap_addr == NULL) {

		LOGE("[-] Get Remote mmap address fails.\n");
		return 0;
	}

	// 调用目标pid进程的mmap函数，在目标pid进程的内存中申请内存空间
	if (ptrace_call(target_pid, (uint32_t) remote_mmap_addr, parameters, 6, &regs) == -1)
		return 0;

	// 获取目标pid进程的寄存器的状态
	if (ptrace_getregs(target_pid, &regs) == -1)
		return 0;

	LOGI("[+] Target process returned from mmap, return r0=%x, r7=%x, pc=%x, \n", regs.ARM_r0, regs.ARM_r7, regs.ARM_pc);

	// arm中，函数的返回值保存在寄存器r0中，返回在目标pid进程中申请的内存空间的地址
	return regs.ARM_pc == 0 ? (void *) regs.ARM_r0 : 0;
}

// 分割字符串
static char* nexttok(char **strp) {

	// 以" "为基准分解字符串，将原字符串中第一个" "替换为'\0'
	// 第一个" "前面的字符串返回在p中，第一个" "后面的字符串在strp中
	char *p = strsep(strp, " ");

	// 返回分割的字符串
	return p == NULL ? "" : p;
}

// 在目标pid进程的"/system/lib/libc.so"的内存范围内(从内存结束地址往回的方向)查找内存空间
void* find_space_in_maps(int pid, int size) {

	char statline[1024];
	FILE * fp;
	uint32_t* addr = (uint32_t*) 0x40008000;
	char *address, *proms, *ptr;
	const char* tname = "/system/lib/libc.so";
	const char* tproms = "r-xp";

	// 获取字符串"/system/lib/libc.so"的长度
	int tnaem_size = strlen(tname);
	// 获取字符串"r-xp"的长度
	int tproms_size = strlen(tproms);

	// 内存以4字节对齐
	size = ((size / 4) + 1) * 4;

	// 格式化得到字符串"/proc/pid/maps"
	sprintf(statline, "/proc/%d/maps", pid);

	// 打开文件"/proc/pid/maps"
	fp = fopen(statline, "r");
	if (fp == 0)
		return 0;

	// 读取文件"/proc/pid/maps"中内容（每次读一行）
	while (fgets(statline, sizeof(statline), fp)) {

		// 分割字符串
		ptr = statline;
		// 得到内存模块的起始和结束地址
		address = nexttok(&ptr); // skip address
		// 内存模块的属性
		proms = nexttok(&ptr); // skip proms
		nexttok(&ptr); // skip offset
		nexttok(&ptr); // skip dev
		nexttok(&ptr); // skip inode

		// ptr中最终保存的是加载的内存模块的路径字符串
		while (*ptr != '\0') {
			if (*ptr == ' ')
				ptr++;
			else
				break;
		}

		// 查找目标so模块
		if (ptr && proms && address) {

			// 判断是否是"r-xp"属性的模块
			if (strncmp(tproms, proms, tproms_size) == 0) {

				// 判断是否是"/system/lib/libc.so"模块
				if (strncmp(tname, ptr, tnaem_size) == 0) {

					// address like afe00000-afe3a000
					if (strlen(address) == 17) {

						// 获取内存加载模块/system/lib/libc.so的内存范围的结束地址（方便后面查找内存空间）
						addr = (uint32_t*) strtoul(address + 9, NULL, 16);
						// 在目标pid进程的/system/lib/libc.so的内存范围内查找到size大小内存空间
						addr -= size;

						printf("proms=%s address=%s name=%s", proms, address, ptr);
						break;
					}
				}
			}
		}
	}

	// 关闭文件
	fclose(fp);

	// 返回在目标进程中查找到的内存空间的地址
	return (void*) addr;
}


// 通过系统函数的地址查找到该函数所在的模块的名称
int find_module_info_by_address(pid_t pid, void* addr, char *module, void** start, void** end) {

	char statline[1024];
	FILE *fp;
	char *address, *proms, *ptr, *p;

	// 格式化字符串得到"/proc/pid/maps"
	if ( pid < 0 ) {

		/* self process */
		snprintf( statline, sizeof(statline), "/proc/self/maps");
	} else {

		snprintf( statline, sizeof(statline), "/proc/%d/maps", pid );
	}

	// 打开文件 /proc/pid/maps
	fp = fopen( statline, "r" );
	if ( fp != NULL ) {

		// 每次一行，读取文件/proc/pid/maps中内容
		while ( fgets( statline, sizeof(statline), fp ) ) {

			// 解析读取为一行字符串信息
			ptr = statline;
			// 获取模块的起始和结束地址
			address = nexttok(&ptr); // skip address
			proms = nexttok(&ptr); // skip proms
			nexttok(&ptr); // skip offset
			nexttok(&ptr); // skip dev
			nexttok(&ptr); // skip inode

			while(*ptr != '\0') {
				if(*ptr == ' ')
					ptr++;
				else
					break;
			}

			p = ptr;
			while(*p != '\0') {
				if(*p == '\n')
					*p = '\0';
				p++;
			}

			// 4016a000-4016b000
			if(strlen(address) == 17) {

				address[8] = '\0';

				// 获取内存加载模块的起始地址
				*start = (void*)strtoul(address, NULL, 16);
				// 获取内存加载模块的结束地址
				*end   = (void*)strtoul(address+9, NULL, 16);

				// printf("[%p-%p] %s | %p\n", *start, *end, ptr, addr);

				// 判断该系统函数的地址是否在该模块的内存范围内
				if(addr > *start && addr < *end) {

					// 找到该系统函数所在的内存模块
					// 保存该内存加载的so模块的文件路径
					strcpy(module, ptr);

					fclose( fp );
					return 0;
				}
			}
		}

		fclose( fp ) ;
	}

	return -1;
}

// 通过指定的内存模块so的路径字符串，获取该内存模块的在目标进程pid中起始地址和结束地址
int find_module_info_by_name(pid_t pid, const char *module, void** start, void** end) {

	char statline[1024];
	FILE *fp;
	char *address, *proms, *ptr, *p;

	if ( pid < 0 ) {

		/* self process */
		snprintf( statline, sizeof(statline), "/proc/self/maps");
	} else {

		snprintf( statline, sizeof(statline), "/proc/%d/maps", pid );
	}

	fp = fopen( statline, "r" );

	if ( fp != NULL ) {

		while ( fgets( statline, sizeof(statline), fp ) ) {

			ptr = statline;
			address = nexttok(&ptr); // skip address
			proms = nexttok(&ptr); // skip proms
			nexttok(&ptr); // skip offset
			nexttok(&ptr); // skip dev
			nexttok(&ptr); // skip inode

			while(*ptr != '\0') {

				if(*ptr == ' ')
					ptr++;
				else
					break;
			}

			p = ptr;
			while(*p != '\0') {

				if(*p == '\n')
					*p = '\0';
				p++;
			}

			// 4016a000-4016b000
			if(strlen(address) == 17) {

				address[8] = '\0';

				*start = (void*)strtoul(address, NULL, 16);
				*end   = (void*)strtoul(address+9, NULL, 16);

				// printf("[%p-%p] %s | %p\n", *start, *end, ptr, addr);

				// 通过内存模块的路径字符串，判读是否是要查找的目标内存so模块
				if(strncmp(module, ptr, strlen(module)) == 0) {

					fclose( fp ) ;

					return 0;
				}
			}
		}

		fclose( fp ) ;
	}

	return -1;
}


// 获取目标pid进程中指定函数的调用地址
void* get_remote_address(pid_t pid, void *local_addr) {

	// 保存加载的内存so模块的文件路径字符串
	char buf[256];
	// 当前进程中指定模块的起始地址
	void* local_start = 0;
	// 当前进程中指定模块的结束地址
	void* local_end = 0;
	// 目标pid进程中指定模块的起始地址
	void* remote_start = 0;
	// 目标pid进程中指定模块的结束地址
	void* remote_end = 0;

	// 获取当前进程中指定系统函数所在的模块的文件路径字符串buf
	if(find_module_info_by_address(-1, local_addr, buf, &local_start, &local_end) < 0) {

		LOGI("[-] find_module_info_by_address FAIL");
		return NULL;
	}

	LOGI("[+] the local module is %s", buf);

	// 通过指定的内存模块so的路径字符串，获取该内存模块的在目标进程pid中起始地址和结束地址
	if(find_module_info_by_name(pid, buf, &remote_start, &remote_end) < 0) {


		LOGI("[-] find_module_info_by_name FAIL");
		return NULL;
	}

	// 目标pid进程的local_addr函数的调用地址
	return (void *)( (uint32_t)local_addr + (uint32_t)remote_start - (uint32_t)local_start );
}


