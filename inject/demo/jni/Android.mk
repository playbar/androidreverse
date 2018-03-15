LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
LOCAL_MODULE := hello   
LOCAL_SRC_FILES := demo.c   
  
#shellcode.s  

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
  
#LOCAL_FORCE_STATIC_EXECUTABLE := true  
  
include $(BUILD_EXECUTABLE) 