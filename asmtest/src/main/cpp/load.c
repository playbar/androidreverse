//
// Created by mac on 16/10/13.
//

#include <jni.h>
#include <android/log.h>

JavaVM *gs_jvm=0;
jint JNI_OnLoad( JavaVM* vm, void *reserved){
    gs_jvm = vm;

    jint result = JNI_VERSION_1_4;
    return  result;

}
