#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include <errno.h>
#include "my_log.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/system_properties.h>
#include <stdlib.h>                     // for free, malloc, realloc
#include <algorithm>

#include "tcmalloctest.h"
#include "lib/test_library.hpp"


using std::min;

void printftime()
{
    struct timeval tv;
    struct tm *tmp_ptr = NULL;
    gettimeofday(&tv,NULL);
    tmp_ptr = localtime(&tv.tv_sec);

    printf("%d-%02d-%02d %02d:%02d:%02d.%.04d\n",
           tmp_ptr->tm_year + 1900,
           tmp_ptr->tm_mon + 1,
           tmp_ptr->tm_mday,
           tmp_ptr->tm_hour,
           tmp_ptr->tm_min,
           tmp_ptr->tm_sec,
           tv.tv_usec);
    return;
}

static void Fill(unsigned char* buffer, int n) {
    for (int i = 0; i < n; i++) {
        buffer[i] = (i & 0xff);
    }
}

static bool Valid(unsigned char* buffer, int n) {
    for (int i = 0; i < n; i++) {
        if (buffer[i] != (i & 0xff)) {
            return false;
        }
    }
    return true;
}


void testTcMalloc()
{
    int src_size = 9;
    int dst_size = 25;

    unsigned char* src = (unsigned char*) malloc(src_size);
    Fill(src, src_size);
    unsigned char* dst = (unsigned char*) realloc(src, dst_size);
    if(!Valid(dst, min(src_size, dst_size)))
    {
        abort();
    }
    Fill(dst, dst_size);

//    new char(10) ; //char 申请一个char空间，被初始化为10
//    new char[10] ; //char数组，申请10个char的空间
    unsigned char *pdata = new unsigned char[src_size];
    Fill(pdata, src_size );
    delete []pdata;
    return;

}


JNIEXPORT void JNICALL Java_com_valgrind_test_HelloJni_nativeMsg(JNIEnv* env, jobject thiz)
{
    int result = 0;

//    system("pwd");
    result = system("mkdir /data/data/com.bar.hellojni/temp");
    if( -1 == result || 127 == result )
    {
        LOGE("error");
    }

    printftime();

    testTcMalloc();

    test_library_::test_library_1();
    test_library_::test_library_2();
    test_library_::test_library_3();

    pid_t pid = getpid();
    uid_t uid = getuid();

    char *username = getlogin();
    LOGE("F:%s,%s", __FUNCTION__, username);
}

JNIEXPORT jstring JNICALL Java_com_valgrind_test_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz )
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

JNIEXPORT jstring JNICALL Java_com_valgrind_test_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz )
{
    return env->NewStringUTF("stringFromJNI_11");
}



jint JNI_OnLoad( JavaVM* vm, void *reserved){

    jint result = JNI_VERSION_1_4;
    return  result;

}

