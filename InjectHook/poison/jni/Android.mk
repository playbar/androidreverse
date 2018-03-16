    LOCAL_PATH := $(call my-dir)  
      
    include $(CLEAR_VARS)  
    LOCAL_MODULE := poison   
    LOCAL_SRC_FILES := poison.c \
		       elf_utils.c \
		       ptrace_utils.c \
		       tools.c
    LOCAL_CFLAGS += -pie -fPIE
    LOCAL_LDFLAGS += -pie -fPIE
    #shellcode.s

    LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

    #LOCAL_FORCE_STATIC_EXECUTABLE := true

    include $(BUILD_EXECUTABLE)
