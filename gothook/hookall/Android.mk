#库文件
#LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE := inject
#LOCAL_SRC_FILES :=  inlineHook.c relocate.c inject.c 
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lEGL
#include $(BUILD_SHARED_LIBRARY)

#库文件
#LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE := hook
#LOCAL_SRC_FILES :=  main.c hook.c mylib.c
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lEGL
#include $(BUILD_SHARED_LIBRARY)


#可执行文件
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE :=  hookall
LOCAL_SRC_FILES := hook.c main.c
LOCAL_LDLIBS := -lm -llog
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)

