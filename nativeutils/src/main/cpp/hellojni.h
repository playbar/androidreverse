#ifndef __HELLOJNI_H__
#define __HELLOJNI_H__

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_utils_HelloJni_nativeMsg(JNIEnv* env, jobject thiz);
JNIEXPORT jstring JNICALL Java_com_utils_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz );
JNIEXPORT jstring JNICALL Java_com_utils_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz );
#ifdef __cplusplus
}
#endif

#endif