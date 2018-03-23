LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := hook
LOCAL_SRC_FILES := hook.c gleshook.c inlineHook.c relocate.c hookutils.c
LOCAL_LDLIBS:=-L$(SYSROOT)/usr/lib -llog -lEGL -lOpenSLES -lGLESv2
LOCAL_LDFLAGS += -shared

include $(BUILD_SHARED_LIBRARY)
