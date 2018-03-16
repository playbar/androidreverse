LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := hello
LOCAL_SRC_FILES := \
                hello.c

LOCAL_LDLIBS := -llog

LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS  := -fPIE

include $(BUILD_SHARED_LIBRARY)