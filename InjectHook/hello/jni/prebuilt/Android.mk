LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := android_runtime
LOCAL_SRC_FILES := libandroid_runtime.so 
include $(PREBUILT_SHARED_LIBRARY)
