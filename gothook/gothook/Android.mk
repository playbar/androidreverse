LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gothook
LOCAL_SRC_FILES := elf_utils.c injector.c main.c ptrace.c utils.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_LDFLAGS += -pie

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -lEGL -lOpenSLES  -L$(LOCAL_PATH)

include $(BUILD_EXECUTABLE)
