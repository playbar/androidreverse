#include<jni.h>
#include<string.h>
#include<stdio.h>
#include "common.h"
#include "JavaMethodHook.h"

int s_ButtonPressCounter = 0;

static inline void get_cstr_from_jstring(JNIEnv* env, jstring jstr,
		char **out) {
	jboolean iscopy = JNI_TRUE;
	const char *cstr = env->GetStringUTFChars(jstr, &iscopy);
	*out = strdup(cstr);
	env->ReleaseStringUTFChars(jstr, cstr);
}

//test
extern "C" jstring JNICALL Java_com_swathook_HookUtils_stringFromJNI(
		JNIEnv* env, jobject thiz) {
	char szBuf[512];
	sprintf(szBuf, "You have pressed this huge button %d times",
			s_ButtonPressCounter++);

	jstring str = env->NewStringUTF(szBuf);
	return str;
}

extern "C" jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved) {
	return JNI_VERSION_1_6;
}

extern "C" jint  Java_com_swathook_HookUtils_hookMethodNative(JNIEnv *env,
		jobject thiz, jstring cls, jstring methodname, jstring methodsig, jobject callback) {

	HookInfo *info = (HookInfo *) malloc(sizeof(HookInfo));

	get_cstr_from_jstring(env, cls, &info->classDesc);
	get_cstr_from_jstring(env, methodname, &info->methodName);
	get_cstr_from_jstring(env, methodsig, &info->methodSig);

	return java_method_hook(env, info,callback);
}

