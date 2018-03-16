LOCAL_PATH := $(call my-dir)

# prebuilt start
include $(CLEAR_VARS)
LOCAL_MODULE := android_runtime
LOCAL_SRC_FILES := prebuilt/libandroid_runtime.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
include $(PREBUILT_SHARED_LIBRARY)
# prebuilt end

include $(CLEAR_VARS)

LOCAL_MODULE:= importdex
LOCAL_SRC_FILES :=  importdex.cpp
LOCAL_LDLIBS += -llog
LOCAL_SHARED_LIBRARIES = libandroid_runtime
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS  := -fPIE
include $(BUILD_SHARED_LIBRARY)