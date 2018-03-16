LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

subdirs += $(LOCAL_PATH)/demo/jni/Android.mk
subdirs += $(LOCAL_PATH)/inject/jni/Android.mk
subdirs += $(LOCAL_PATH)/hellolib/jni/Android.mk

include $(subdirs)
