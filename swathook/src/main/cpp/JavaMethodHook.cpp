#include "JavaMethodHook.h"

extern int dalvik_java_method_hook(JNIEnv*, HookInfo *,jobject );


static bool isArt() {
	return false;
}

int java_method_hook(JNIEnv* env, HookInfo *info,jobject callback) {
	if (!isArt()) {
		return dalvik_java_method_hook(env, info, callback);
	}

	return -1;
}

