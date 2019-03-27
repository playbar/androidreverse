cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 \
            -DGOOGLE_PROTOBUF_NO_RTTI -fexceptions -Wno-reserved-user-defined-literal")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden  -ffunction-sections -fdata-sections")

#-fsanitize=address

set(CMAKE_CXX_STANDARD 14)



add_library(inlinehook SHARED
            hellojni.c
            inlineHook.c
            relocate.c
            )

target_link_libraries(inlinehook
                      android
                      log)
