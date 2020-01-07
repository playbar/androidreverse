#include <android/log.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <malloc.h>
#include "hooklog.h"
#include "callstack.h"
#include "hookutils.h"
#include "abort_message.h"

int (*old__android_log_print)(int prio, const char *tag, const char *fmt, ...) = NULL;
int mj__android_log_print(int prio, const char *tag,  const char *fmt, ...)
{
    va_list ap;
    char buf[1024];
    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);

    return __android_log_write(prio, tag, buf);
//    return old__android_log_print(prio, tag, fmt);
}

void* (*old_dlopen)(const char*  filename, int flag) = NULL;
void* mj_dlopen(const char*  filename, int flag)
{
    LOGITAG("mjhook", "mj_dlopen, filename=%s", filename);
    return old_dlopen(filename, flag);
}

FILE *(*old_fopen)(const char * name , const char * mode) = NULL;
FILE *mj_fopen(const char * name , const char * mode)
{
    LOGITAG("mjhook", "mj_fopen, filename=%s", name);
    FILE *pfile = old_fopen(name, mode);
    return pfile;
}

//void* (*old_dlsym)(void*  handle, const char*  symbol) = NULL;
//void* mj_dlsym(void*  handle, const char*  symbol)
//{
//    LOGITAG("mjhook", "mj_dlsym, symbol=%s", symbol);
//    return old_dlsym(handle, symbol);
//}

int (*old_dlclose)(void* handle) = NULL;
int mj_dlclose(void*  handle)
{
    LOGITAG("mjhook", "mj_dlclose");
    return old_dlclose(handle);
}

//////////////////////

void* (*old_malloc)(size_t byte_count) = NULL;
void* mj_malloc(size_t byte_count)
{
    void *re = old_malloc(byte_count);
    LOGITAG("mjmem", "mj_malloc, bytecount=%d, re=%x", byte_count, re);
    memset(re, 0, byte_count);
    return re;
}

void* (*old_calloc)(size_t item_count, size_t item_size) = NULL;
void* mj_calloc(size_t item_count, size_t item_size)
{
    void *re = old_calloc(item_count, item_size);
    LOGITAG("mjmem", "mj_calloc, re=%x, item_cout=%d, item_size=%d",  re, item_count, item_size);
    return re;
}
void* (*old_realloc)(void* p, size_t byte_count) = NULL;
void* mj_realloc(void* p, size_t byte_count)
{
    void *re = old_realloc(p, byte_count);
    LOGITAG("mjmem", "mj_realloc, bytecount=%d, re=%x", byte_count, re);
    return re;
}

void (*old_free)(void* p) = NULL;
void mj_free(void* p)
{
    if( p == NULL )
        return;
    LOGITAG("mjmem", "mj_free, p=%x", p);
    return old_free(p);
}

// thread

int (*old_pthread_attr_init)(pthread_attr_t * attr) = NULL;
int mj_pthread_attr_init(pthread_attr_t * attr)
{
    LOGITAG("mjhook", "mj_pthread_attr_init");
    return old_pthread_attr_init(attr);
}

int (*old_pthread_create)(pthread_t *thread, pthread_attr_t const * attr, void *(*start_routine)(void *), void * arg) = NULL;
int mj_pthread_create(pthread_t *thread, pthread_attr_t const * attr, void *(*start_routine)(void *), void * arg)
{
    LOGITAG("mjhook", "mj_pthread_create");
//    print_callstack();
    int re = 0;
    re = old_pthread_create(thread, attr, start_routine, arg);
    return re;
}

void (*old_abort_message)(const char *format, ...) = NULL;
void mj_abort_message(const char *format, ...)
{
    LOGITAG("mjhook", "mj_abort_message");
    va_list ap;
    char buf[1024];
    va_start(ap, format);
    vsnprintf(buf, 1024, format, ap);
    va_end(ap);
    return old_abort_message(buf);

}


void hookThreadFun()
{
//    hook((uint32_t) __android_log_print, (uint32_t)mj__android_log_print, (uint32_t **) &old__android_log_print);
    hook((uint32_t) fopen, (uint32_t)mj_fopen, (uint32_t **) &old_fopen);
    hook((uint32_t) dlopen, (uint32_t)mj_dlopen, (uint32_t **) &old_dlopen);
//    hook((uint32_t) dlsym, (uint32_t)mj_dlsym, (uint32_t **) &old_dlsym);
    hook((uint32_t) dlclose, (uint32_t)mj_dlclose, (uint32_t **) &old_dlclose);

//    hook((uint32_t) pthread_attr_init, (uint32_t)mj_pthread_attr_init, (uint32_t **) &old_pthread_attr_init);
    hook((uint32_t) pthread_create, (uint32_t)mj_pthread_create, (uint32_t **) &old_pthread_create);
}

void hookMemFun()
{
    hook((uint32_t) malloc, (uint32_t)mj_malloc, (uint32_t **) &old_malloc);
    hook((uint32_t) calloc, (uint32_t)mj_calloc, (uint32_t **) &old_calloc);
    hook((uint32_t) realloc, (uint32_t)mj_realloc, (uint32_t **) &old_realloc);
    hook((uint32_t) free, (uint32_t)mj_free, (uint32_t **) &old_free);
}

void hookBadAlloc()
{
    hook((uint32_t) abort_message, (uint32_t)mj_abort_message, (uint32_t **) &old_abort_message);

}

