cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 \
            -DGOOGLE_PROTOBUF_NO_RTTI -fexceptions -Wno-reserved-user-defined-literal")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden -ffunction-sections -fdata-sections -DGOOGLE_PROTOBUF_NO_RTTI")

set(CMAKE_CXX_STANDARD 14)

ADD_DEFINITIONS(-DEGL_EGLEXT_PROTOTYPES)
ADD_DEFINITIONS(-DGL_GLEXT_PROTOTYPES)


add_library(glhook SHARED
        abort_message.cpp
       inlineHook.c
       hookutils.c
       relocate.c
       eglexthook.c
       gl2exthook.c
       eglhook.c
       gleshook.c
       hellojni.c
       threadhook.c
       callstack.c
       elf_common.cpp
       elf_hooker.cpp
       elf_main.cpp
       elf_module.cpp
       #exporthook.c
       #exportutil.c
       #andhook.c
       #syscallstack.c
       )

target_include_directories(glhook PRIVATE
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          )

target_link_libraries(glhook
                     log
                     android
 #                   lib_EH
 #                   vulkan
                     GLESv2
                     EGL)

