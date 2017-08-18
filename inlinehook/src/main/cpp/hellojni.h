#ifndef __HELLOJNI_H__
#define __HELLOJNI_H__

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*FP_old_puts)(const char *);

JNIEXPORT void JNICALL Java_com_inlinehook_HelloJni_nativeMsg(JNIEnv* env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_inlinehook_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );
JNIEXPORT jstring JNICALL Java_com_inlinehook_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz );
jint JNI_OnLoad( JavaVM* vm, void *reserved);

#ifdef __cplusplus
}
#endif

#endif

