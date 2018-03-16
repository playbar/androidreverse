LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

subdirs += $(LOCAL_PATH)/gothook/Android.mk
subdirs += $(LOCAL_PATH)/libhook/Android.mk
subdirs += $(LOCAL_PATH)/hookall/Android.mk

include $(subdirs)
