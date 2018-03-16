#include <android/log.h>
#include <stdarg.h>
#include <stdio.h>
#define TAG "Hook Library"

//int my_printf(const char *format, ...) {
//  va_list args;
//  va_start(args, format);
//  int ret = __android_log_vprint(ANDROID_LOG_DEBUG, TAG, format, args);
//  va_end(args);
//  return ret;
//}

#define LOG_TAG "test"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

int (*old_strcmp)(const char* c1, const char* c2) = -1;

int new_strcmp(const char* c1, const char* c2)
{
    LOGE("[+]new_strcmp called [+]\n");
    LOGE("[+] s1 = %s [+]\n", c1);
    LOGE("[+] s2 = %s [+]\n", c2);
//    if (old_strcmp == 0)
//        printf("[+] error:old_strcmp = -1 [+]\n");
//    return old_strcmp(c1, c2);
    return 0;
}

