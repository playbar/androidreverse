LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := interceptSysCall
LOCAL_SRC_FILES := interceptSysCall.c

LOCAL_LDLIBS+= -L$(SYSROOT)/usr/lib -llog

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE

include $(BUILD_EXECUTABLE)
