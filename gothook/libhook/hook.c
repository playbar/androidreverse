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

int (*old_strcmp)(const char* c1, const char* c2) = -1;

int new_strcmp(const char* c1, const char* c2)
{
    printf("[+]new_strcmp called [+]\n");
    printf("[+] s1 = %s [+]\n", c1);
    printf("[+] s2 = %s [+]\n", c2);
//    if (old_strcmp == 0)
//        printf("[+] error:old_strcmp = -1 [+]\n");
//    return old_strcmp(c1, c2);
    return 0;
}

