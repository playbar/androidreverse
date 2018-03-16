#include <android/log.h>
#include "unistd.h"

#define LOG_TAG "360Inject"
#define LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

int inject_log_message(char *msg) {
  LOGD("Inject success, pid = %d\n", getpid());
  LOGD("Inject message by 360AnFu: %s\n", msg);
  return 0;
}
