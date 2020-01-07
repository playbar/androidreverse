#ifndef __tcmalloctest_H__
#define __tcmalloctest_H__
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include <errno.h>
#include "my_log.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/system_properties.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_valgrind_test_HelloJni_nativeMsg(JNIEnv* env,jobject thiz);
JNIEXPORT jstring JNICALL Java_com_valgrind_test_HelloJni_stringFromJNI(JNIEnv * env, jobject thiz );
JNIEXPORT jstring JNICALL Java_com_valgrind_test_HelloJni_stringFromJNI_11(JNIEnv * env, jobject thiz );
jint JNI_OnLoad(JavaVM *vm, void *reserved);

#ifdef __cplusplus
}
#endif


#endif


