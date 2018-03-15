/*
 * elf_utils.h
 *
 *  Created on: 2013-6-19
 *      Author: boyliang
 */

#ifndef ELF_UTILS_H_
#define ELF_UTILS_H_

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>

// 获取目标pid进程中指定so模块的加载基址
void* get_module_base(pid_t pid, const char* module_name);

// 在目标pid进程的内存空间中申请内存，申请成功返回的内存地址保存在r0中
void* find_space_by_mmap(int target_pid, int size);

// 在目标pid进程的"/system/lib/libc.so"的内存范围内(从内存结束地址往回的方向)查找内存空间
void* find_space_in_maps(int pid, int size);

// 通过系统函数的地址查找到该函数所在的模块的名称
int find_module_info_by_address(pid_t pid, void* addr, char *module, void** start, void** end);

// 通过指定的内存模块so的路径字符串，获取该内存模块的在目标进程pid中起始地址和结束地址
int find_module_info_by_name(pid_t pid, const char *module, void** start, void** end);

// 获取目标pid进程中指定函数的调用地址
void* get_remote_address(pid_t pid, void *local_addr);

#endif /* ELF_UTILS_H_ */
