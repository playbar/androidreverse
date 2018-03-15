//
// Created by syang on 2017/7/6.
//

#ifndef ANDROIDINJECT_LOG_H
#define ANDROIDINJECT_LOG_H

#include <android/log.h>
// 调试模式

#define  LOG_TAG "hook"
#define  LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, fmt, ##args)
#define  LOGI(fmt, args...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG, fmt, ##args)
#define  LOGE(fmt, args...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, fmt, ##args)
#define  SUCC(fmt, args...)  __android_log_print(ANDROID_LOG_INFO,"Success!", fmt, ##args)
#define  FAIL(fmt, args...)  __android_log_print(ANDROID_LOG_INFO,"Fail!", fmt, ##args)
#define  DEBUG_PRINT(format,args...) LOGI(format, ##args)

#endif //ANDROIDINJECT_LOG_H
