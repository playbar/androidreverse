LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

subdirs += $(LOCAL_PATH)/payload/jni/Android.mk
subdirs += $(LOCAL_PATH)/injector/jni/Android.mk


include $(subdirs)
