//
// Created by mac on 16/10/13.
//

#ifndef HELLO_JNI_MY_LOG_H
#define HELLO_JNI_MY_LOG_H

#include <android/log.h>

#define MY_LOG_LEVEL_VERBOSE    1
#define MY_LOG_LEVEL_DEBUG      2
#define MY_LOG_LEVEL_INFO       3
#define MY_LOG_LEVEL_WARNING    4
#define MY_LOG_LEVEL_ERROR      5
#define MY_LOG_LEVEL_FATAL      6
#define MY_LOG_LEVEL_SILENT     7

#ifndef MY_LOG_TAG
#define MY_LOG_TAG "test"
#endif

#ifndef MY_LOG_LEVEL
#define MY_LOG_LEVEL MY_LOG_LEVEL_VERBOSE
#endif

#define MY_LOG_NOOP (void)0

#define LOG_TAG "test"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define MY_LOG_PRINT(level, fmt, ...) \
    __android_log_print(level, MY_LOG_TAG, fmt, __VA_ARGS__ )

#if MY_LOG_LEVEL_VERBOSE >= MY_LOG_LEVEL
#define MY_LOG_VERBOSE(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_VERBOSE(...) MY_LOG_NOOP
#endif


#if MY_LOG_LEVEL_DEBUG >= MY_LOG_LEVEL
#define MY_LOG_DEBUG(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_DEBUG(...) MY_LOG_NOOP
#endif


#if MY_LOG_LEVEL_INFO >= MY_LOG_LEVEL
#define MY_LOG_INFO(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_INFO, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_INFO(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_WARNING >= MY_LOG_LEVEL
#define MY_LOG_WARNING(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_WARNING, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_WARNING(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_ERROR >= MY_LOG_LEVEL
#define MY_LOG_ERROR(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_ERROR(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_FATAL >= MY_LOG_LEVEL
#define MY_LOG_FATAL(fmt, ...)\
    MY_LOG_PRINT(ANDROID_LOG_FATAL, fmt, ##__VA_ARGS__)
#else
#define MY_LOG_FATAL(...) MY_LOG_NOOP
#endif

#if MY_LOG_LEVEL_SILENT >= MY_LOG_LEVEL
#define MY_LOG_ASSERT(expression, fmt, ...)\
    if(!expression)\
    {\
        __android_log_assert(#expression, MY_LOG_TAG, \
            fmt, ##__VA_ARGS__);\
    }
#else
#define MY_LOG_ASSERT(...) MY_LOG_NOOP
#endif

#endif //HELLO_JNI_MY_LOG_H
