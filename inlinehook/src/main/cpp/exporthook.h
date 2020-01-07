/*
 *  Collin's Binary Instrumentation Tool/Framework for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#include <android/log.h>

void (*log_function)(char *logmsg);

#define log(...) __android_log_print(3, "LWP", __VA_ARGS__);

struct hook_t {
	unsigned int jump[3];
	unsigned int store[3];
	unsigned char jumpt[20];
	unsigned char storet[20];
	unsigned int orig;
	unsigned int patch;
	unsigned char thumb;
	unsigned char name[128];
	void *data;
};

int start_coms(int *coms, char *ptsn);

void hook_cacheflush(unsigned int begin, unsigned int end);	
void hook_precall(struct hook_t *h);
void hook_postcall(struct hook_t *h);
int exporthook(struct hook_t *h, int pid, char *libname, char *funcname, void *hook_arm, void *hook_thumb);
int hook_lwp(int pid, char *libname, char *funcname, void *hookFunc, void **origFunc);
void unhook(struct hook_t *h);
