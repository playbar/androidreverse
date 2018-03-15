LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
 
#LOCAL_CFLAGS += -fPIE
#LOCAL_LDFLAGS += -fPIE

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -lEGL

#LOCAL_ARM_MODE := arm  
LOCAL_MODULE    := hello  
LOCAL_SRC_FILES := myhook.c  
include $(BUILD_SHARED_LIBRARY)