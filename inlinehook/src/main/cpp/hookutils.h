#ifndef __HOOKUTILS_H__
#define __HOOKUTILS_H__

#include <stdint.h>
#include <jni.h>
#include "inlineHook.h"

#ifdef __cplusplus
extern "C" {
#endif

int hook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);

int unHook(uint32_t target_addr);

void hookgl2extFun();
void hookEglextFun();
void hookEGLFun();
void hookESFun();
void hookExportHook();
void hookGLESFun();
void unhookAllFun();

void hookImportFunInit();
void hookImportFun(const char *modulename, const char *funname, void *myEglGetProcAddress, void **oldEglSwapBuffers);

void hookThreadFun();
void hookVulkanFun();
void hookMemFun();
void hookBadAlloc();


JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_printMapInfo(JNIEnv *env, jobject obj);

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_initHook(JNIEnv *env, jobject obj);

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_hookTest(JNIEnv *env, jobject obj);

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_unInitHook(JNIEnv *env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif

