LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
 
#LOCAL_CFLAGS += -fPIE
#LOCAL_LDFLAGS += -fPIE

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -lEGL

LOCAL_MODULE    := hello  
LOCAL_SRC_FILES := lib_origin.c
include $(BUILD_SHARED_LIBRARY)