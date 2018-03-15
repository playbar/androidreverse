/*
 * tool.c
 *
 *  Created on: 2013-7-5
 *      Author: boyliang
 */


#include <stdio.h>
#include <dlfcn.h>
#include <stddef.h>

// 获取指定内存加载模块的导出函数的地址
void *get_method_address(const char *soname, const char *methodname) {

	void *handler = dlopen(soname, RTLD_NOW | RTLD_GLOBAL);

	return dlsym(handler, methodname);
}


// 获取目标pid进程的名称字符串
const char* get_process_name(pid_t pid) {

	static char buffer[255];
	FILE* f;
	char path[255];

	// 格式化得到字符串"/proc/pid/cmdline"
	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

	// 读取文件"/proc/pid/cmdline"的内容，获取进程的命令行参数
	if ((f = fopen(path, "r")) == NULL) {

		return NULL;
	}

	// 读取文件"/proc/pid/cmdline"的第1行字符串内容--进程的名称
	if (fgets(buffer, sizeof(buffer), f) == NULL) {

		return NULL;
	}

	// 关闭文件
	fclose(f);

	return buffer;
}
