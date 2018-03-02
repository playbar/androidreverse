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


int (*orig_printf)(const char *format, ...);
int fake_printf(const char *format, ...) {
    puts("call printf");

    char *stack[16];
    va_list args;
    va_start(args, format);
    // *(void **)&args for android
    memcpy(stack, *(void **)&args, sizeof(char *) * 16);
    va_end(args);

    // how to hook variadic function? fake a original copy stack.
    // [move to
    // detail-1](http://jmpews.github.io/2017/08/29/pwn/%E7%9F%AD%E5%87%BD%E6%95%B0%E5%92%8C%E4%B8%8D%E5%AE%9A%E5%8F%82%E6%95%B0%E7%9A%84hook/)
    // [move to detail-2](https://github.com/jmpews/HookZzModules/tree/master/AntiDebugBypass)
    int x = orig_printf(format, stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7],
                        stack[8], stack[9], stack[10], stack[11], stack[12], stack[13], stack[14], stack[15]);
    return x;
}

void printf_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    puts((char *)rs->general.regs.r0);
    STACK_SET(cs, "format", rs->general.regs.r0, char *);
    puts("printf-pre-call");
}

void printf_post_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    if (STACK_CHECK_KEY(cs, "format")) {
        char *format = STACK_GET(cs, "format", char *);
        puts(format);
    }
    puts("printf-post-call");
}

void test_hook_printf() {
    void *printf_ptr = (void *)printf;

    ZzEnableDebugMode();
    ZzHook((void *)printf_ptr, (void *)fake_printf, (void **)&orig_printf, printf_pre_call, printf_post_call, false);
    printf("HookZzzzzzz, %d, %p, %d, %d, %d, %d, %d, %d, %d\n", 1, (void *)2, 3, (char)4, (char)5, (char)6, 7, 8, 9);
}

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

JNIEXPORT void JNICALL Java_com_hook_project_HelloJni_nativeMsg(JNIEnv* env, jobject thiz)
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

JNIEXPORT jstring JNICALL Java_com_hook_project_HelloJni_stringFromJNI( JNIEnv* env, jobject thiz )
{

//    testProperties();
//    testcode6();
    test_hook_printf();

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

JNIEXPORT jstring JNICALL Java_com_hook_project_HelloJni_stringFromJNI_11( JNIEnv* env, jobject thiz )
{
    return (*env)->NewStringUTF(env, "stringFromJNI_11");
}

