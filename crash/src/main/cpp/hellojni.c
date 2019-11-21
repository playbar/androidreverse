/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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

#define LOG_TAG "test"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ABI "armeabi-v7a"

extern JavaVM *gs_jvm;

void testcode6()
{
    FILE *stream;
    stream = popen("pwd", "r");
    char ch[1024];
    fgets(ch, 1024, stream);
    LOGW("pwd: %s",ch);
    pclose(stream);
    stream = popen("ls", "r");
    if( NULL == stream )
    {
        LOGE("Unable to execute the command");
    }
    else
    {
        char buffer[1024];
        int status;
        while( NULL != fgets(buffer, 1024, stream))
        {
            LOGE("read: %s", buffer);
        }
        status = pclose(stream);
        LOGW("process exited with status %d", status);
    }
}

void testProperties()
{
    char value[PROP_VALUE_MAX];
    if(0 == __system_property_get("ro.product.model", value))
    {
        LOGE("error");
    }
    else
    {
        LOGW("product model: %s", value);
    }

    const prop_info *property;
    property = __system_property_find("ro.product.model");
    if( NULL == property )
    {
        LOGE("error");
    }
    else
    {
        char name[PROP_NAME_MAX];
        char value[PROP_VALUE_MAX];
        if( 0 == __system_property_read(property, name, value))
        {
            LOGE("is empty");
        }
        else
        {
            LOGW("%s, %s", name, value);
        }
    }
    return;
}

static void testNoPara()
{
    LOGE(" %s", __FUNCTION__ );
}

void testOnePara(int a1 )
{
    LOGE("%s, %d, ", __FUNCTION__, a1);
}

void testParameter(int a1,  int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10,
          int a11,  int a12, int a13, int a14, int a15, int a16, int a17, int a18, int a19, int a20,
          int a21, int a22, int a23, int a24, int a25, int a26, int a27, int a28, int a29, int a30,
          int a31, int a32, int a33, int a34, int a35, int a36, int a37, int a38, int a39, int a40,
          int a41, int a42, int a43, int a44, int a45, int a46, int a47, int a48, int a49, int a50)
{
    LOGE("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
         a1,   a2, a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
         a11,  a12, a13,  a14,  a15,  a16,  a17,  a18,  a19,  a20,
         a21,  a22, a23,  a24,  a25,  a26,  a27,  a28,  a29,  a30,
         a31,  a32, a33,  a34,  a35,  a36,  a37,  a38,  a39,  a40,
         a41,  a42, a43,  a44,  a45,  a46,  a47,  a48,  a49,  a50);
    return;
}

//void testparam(int ai)
//{
//    int i = ai;
//    LOGE("Fun : %s, ai : %d", __FUNCTION__, i );
//}


void willCrash()
{
    int *p = NULL;
    *p = 10;
    LOGE("Fun : %s, Line : %d", __FUNCTION__, __LINE__ );
}

void testString(char *pdata )
{
    pdata[0] = 'a';
    LOGE("Fun : %s, Line : %d", __FUNCTION__, __LINE__ );
}

JNIEXPORT void JNICALL
Java_com_crash_test_HelloJni_nativeMsg(JNIEnv* env, jobject thiz)
{
    int result = 0;
//    system("pwd");
    result = system("mkdir /data/data/com.bar.hellojni/temp");
    if( -1 == result || 127 == result )
    {
        LOGE("error");
    }

    testString(NULL);
    willCrash();

//    testparam( 5);

    pid_t pid = getpid();
    uid_t uid = getuid();

    testNoPara();
    testOnePara(2);
    testParameter(1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                  31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                  41, 42, 43, 44, 45, 46, 47, 48, 49 ,50);

    char *username = getlogin();
    LOGE("F:%s,%s", __FUNCTION__, username);
}

JNIEXPORT jstring JNICALL
Java_com_crash_test_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz )
{

    LOGE(__FUNCTION__);

//    testProperties();
//    testcode6();

//    MY_LOG_VERBOSE("The stringFromJNI is called");
//    LOGE( "The stringFromJNI is called");
//    MY_LOG_DEBUG("env=%p thiz=%p", env, thiz);
//    MY_LOG_DEBUG("%s", "=========>test");
//    MY_LOG_ASSERT(0!=env, "JNIEnv cannot be NULL");
//    MY_LOG_INFO("REturning a new string");

    if( JNI_OK == (*env)->MonitorEnter(env, thiz)){
        LOGE("MonitorEnterr");
    }

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
    LOGE("%s", username);

//    char *buffer;
//    size_t i;
//    buffer = (char*)malloc(4);
//    for(i = 0; i < 5; ++i )
//    {
//        buffer[i] = 'a';
//    }
//    free(buffer);

//    if( 0 != errno )
//    {
//        __android_log_assert("0!=errno","hello-jni", "There is an error.");
//    }

    if(JNI_OK == (*env)->MonitorExit(env, thiz)){
        LOGE("MonitorExit");
    }
//    (*env)->ExceptionClear(env);
    return (*env)->NewStringUTF(env, "Hello from JNI !  Compiled with ABI " ABI ".");
}

JNIEXPORT jstring JNICALL
Java_com_crash_test_HelloJni_stringFromJNI_11(JNIEnv* env, jobject thiz )
{
    return (*env)->NewStringUTF(env, "stringFromJNI_11");
}

