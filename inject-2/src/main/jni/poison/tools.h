/*
 * tool.h
 *
 *  Created on: 2013-7-5
 *      Author: boyliang
 */

#ifndef TOOL_H_
#define TOOL_H_

#include <stdio.h>
#include <dlfcn.h>

// 获取指定内存加载模块的导出函数的地址
void *get_method_address(const char *soname, const char *methodname);

// 获取目标pid进程的名称字符串
const char* get_process_name(pid_t pid);

#endif /* TOOL_H_ */
