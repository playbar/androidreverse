#include <string.h>
#include <android/log.h>

#define LOG_TAG "DEBUG"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

int Hook(void* new_func,char* so_path,void* old_func);