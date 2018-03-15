//
// Created by syang on 2017/7/6.
//
#include <jni.h>
#include <unistd.h>
#include "log.h"

int hook_entry(const char *param){
    LOGI("The process is pid = %d\n", getpid());
    LOGD("Hello, the param is: %s\n", param);
    return 0;
}

JNIEXPORT void JNICALL
Java_cn_syang2forever_androidinject_MainActivity_sayHello(JNIEnv *env, jobject instance) {
    hook_entry("Hello World!");
}