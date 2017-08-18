#ifndef __ELFHOOK_H__
#define __ELFHOOK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dlfcn.h>
#include <jni.h>
#include "elf_hooker.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEnv* __getEnv(bool* attached);
void __releaseEnv(bool attached);
int __elfhooker_init(JavaVM* vm, JNIEnv* env);
void __elfhooker_deinit(void);

void *__nativehook_impl_dlopen(const char *filename, int flag);
int __nativehook_impl_connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
void *__nativehook_impl_android_dlopen_ext(const char *filename, int flags, const void *extinfo) ;

JNIEXPORT int JNICALL Java_com_wadahana_testhook_ElfHooker_setHook(JNIEnv* env, jobject thiz);
JNIEXPORT int JNICALL Java_com_wadahana_testhook_ElfHooker_test(JNIEnv* env, jobject thiz);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved);

#ifdef __cplusplus
}
#endif

#endif


