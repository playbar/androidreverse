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

#include "tcmalloctest.h"


JNIEXPORT void JNICALL Java_com_tcmalloc_test_HelloJni_nativeMsg(JNIEnv* env, jobject thiz)
{
    int result = 0;
//    system("pwd");
    result = system("mkdir /data/data/com.bar.hellojni/temp");
    if( -1 == result || 127 == result )
    {
        LOGE("error");
    }

    pid_t pid = getpid();
    uid_t uid = getuid();

    char *username = getlogin();
    LOGE("F:%s,%s", __FUNCTION__, username);
}

JNIEXPORT jstring JNICALL Java_com_tcmalloc_test_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz )
{

    LOGE(__FUNCTION__);


    if( JNI_OK == env->MonitorEnter(thiz))
    {
        LOGE("MonitorEnterr");
    }


    if(JNI_OK == env->MonitorExit(thiz)){
        LOGE("MonitorExit");
    }
    return env->NewStringUTF("Hello from JNI !  Compiled with ABI");
}

JNIEXPORT jstring JNICALL Java_com_tcmalloc_test_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz )
{
    return env->NewStringUTF("stringFromJNI_11");
}



jint JNI_OnLoad( JavaVM* vm, void *reserved){

    jint result = JNI_VERSION_1_4;
    return  result;

}

