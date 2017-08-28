#ifndef __HELLOJNI_H__
#define __HELLOJNI_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void sysCallstack();

int func2(int a, int b);

JNIEXPORT void JNICALL Java_com_reverse_HelloJni_nativeMsg(JNIEnv* env, jobject thiz);
JNIEXPORT void JNICALL Java_com_reverse_HelloJni_nativeStructRevers(JNIEnv* env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_reverse_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );
JNIEXPORT jstring JNICALL Java_com_reverse_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz );
jint JNI_OnLoad( JavaVM* vm, void *reserved);

#ifdef __cplusplus
}
#endif

#endif
