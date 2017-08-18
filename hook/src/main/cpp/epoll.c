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
#include <sys/time.h>
#include <time.h>

#include <jni.h>
#include <stdlib.h>
#include <android/log.h>
#include <pthread.h>

#include "hook.h"
#include "base.h"

#undef log

// #define log(...) \
//         {FILE *fp = fopen("/data/local/tmp/adbi_example.log", "a+"); if (fp) {\
//         fprintf(fp, __VA_ARGS__);\
//         fclose(fp);}}

#define DEBUG 1
#if DEBUG
	#define log(...) __android_log_print(ANDROID_LOG_DEBUG, "LWP", __VA_ARGS__);
#else
 	#define log(...)
#endif
//#define gettid() syscall(__NR_gettid)

// this file is going to be compiled into a thumb mode binary

//void __attribute__ ((constructor)) my_init(void);

static struct hook_t eph;
static struct hook_t eph2;
static struct hook_t eph3;

// for demo code only
static int counter;

// arm version of hook
extern int my_epoll_wait_arm(int epfd, struct epoll_event *events, int maxevents, int timeout);
extern int gettimeofday_new_arm(struct timeval *tv, struct timezone *tz);
extern int clock_gettime_new_arm(clockid_t clk_id, struct timespec *tp);
/*  
 *  log function to pass to the hooking library to implement central loggin
 *
 *  see: set_logfunction() in base.h
 */
static void my_log(char *msg)
{
	log("%s", msg)
}

int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	log("my_epoll_wait called\n")
	hook_precall(&eph);
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);
	if (1) {
		hook_postcall(&eph);
		log("epoll_wait() called\n")
		counter--;
		if (!counter)
			log("removing hook for epoll_wait()\n")
	}
        
	return res;
}

#define velocity 1
//local gettimeofday
int gettimeofday_new(struct timeval *tv, struct timezone *tz) {
	//old gettimeofday
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mutex);
	int (*gettimeofday_old)(struct timeval *tv, struct timezone *tz);
	gettimeofday_old = (void*)eph2.orig;
	log("gettimeofday_new begin %d\n", gettid());
	hook_precall(&eph2);
	int ret = gettimeofday_old(tv, tz);
	if (ret == 0) {
		tv->tv_sec *= velocity;
		tv->tv_usec *= velocity;
	}
	hook_postcall(&eph2);
	log("gettimeofday_end end %d\n", gettid());
	pthread_mutex_unlock(&mutex);
	return ret;
}

int (*gettimeofday_old_elf)(struct timeval *tv, struct timezone *tz);

int gettimeofday_new_elf(struct timeval *tv, struct timezone *tz) {
	//old gettimeofday
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&mutex);
	//log("gettimeofday_new_elf begin %d\n", gettid());
	int ret = gettimeofday_old_elf(tv, tz);
	if (ret == 0) {
		tv->tv_sec *= velocity;
		tv->tv_usec *= velocity;
	}
	//log("gettimeofday_new_elf_end %d\n", gettid());
	pthread_mutex_unlock(&mutex);
	return ret;
}

//用substrate库来进行hook，自己看我把so放在手机什么位置的
void hookSubstrate(void) {
	log("substrate book begin\n")
	void *substrateSub = dlopen("/data/libsubstrate.so", RTLD_NOW);
	if (!substrateSub)
	{
		log("open libsubstrate fail\n");
		return;
	}

	void (*MSHookFunction)(void *symbol, void *replace, void **result);
	MSHookFunction = dlsym(substrateSub, "MSHookFunction");
	void *libcSub = dlopen("/system/lib/libc.so", RTLD_NOW);
	if (!libcSub) {
		log("open libc fail\n");
		return;
	}

	void *target = dlsym(libcSub, "gettimeofday");
	if (!MSHookFunction || !target) {
		log("function sym can't found (tool:%d, target:%d)\n", MSHookFunction, target);
		return;
	}

	MSHookFunction((void *)target, (void *)&gettimeofday_new_elf, (void **)&gettimeofday_old_elf);
	log("substrate hook end\n")
}


int (*gettimeofday_old_directly)(struct timeval *tv, struct timezone *tz) = 0;
int gettimeofday_new_directly(struct timeval *tv, struct timezone *tz) {
	log("gettimeofday_new_directly begin\n");
	int ret = 0;
	if (gettimeofday_old_directly) {
		log("call old\n")
		ret = gettimeofday_old_directly(tv, tz);
	}
	log("gettimeofday_new_directly end:%d\n", ret);
	return ret;	
}

//通过修改指令的方式来直接hook
void hook_directly() {
	log("hook_directly begin\n");
	hook_lwp(getpid(), "libc.", "gettimeofday", gettimeofday_new_directly, (void **)&gettimeofday_old_directly);

	log("hook_directly end :0x%lx\n", (unsigned long)gettimeofday_old_directly);
}

int hook_entry(char * a)
{
    log("Hook success,%d, pid = %d\n", 5, getpid())
    //hookSubstrate();
    hook_directly();
    log("Hook end\n")
    struct  timeval    tv;
    struct  timezone   tz;
    gettimeofday(&tv,&tz);
    log("%d", tv.tv_sec);
    return 0;
}

