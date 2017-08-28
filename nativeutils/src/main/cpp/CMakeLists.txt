cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fPIC  -shared")

add_definitions(-DHAVE_ENDIAN_H)
add_definitions(-DUNW_REMOTE_ONLY)
#add_definitions(-DHAVE_LZMA)

add_library(nativeutils SHARED
            hellojni.cc
            load.cc
            libutils/CallStack.cpp
            libutils/String8.cpp
            libutils/Printer.cpp
            libutils/VectorImpl.cpp
            libutils/SharedBuffer.cpp
            libutils/Unicode.cpp
            libutils/String16.cpp
            libutils/Static.cpp
            base/stringprintf.cpp
            libbacktrace/Backtrace.cpp
            libbacktrace/UnwindMap.cpp
            libbacktrace/BacktraceMap.cpp
            libbacktrace/UnwindCurrent.cpp
            libbacktrace/UnwindPtrace.cpp
            libbacktrace/BacktraceCurrent.cpp
            libbacktrace/ThreadEntry.cpp
            libunwind/src/mi/map.c
            libunwind/src/mi/Lmap.c
            libunwind/src/Los-common.c
            libunwind/src/os-linux.c
            libunwind/src/elfxx.c
            )

target_include_directories(nativeutils PRIVATE
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/libunwind/src
                          ${CMAKE_CURRENT_SOURCE_DIR}/base/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/safe-iop/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include/tdep-arm)

# Include libraries needed for hello-jni lib
target_link_libraries(nativeutils
                      android
                      log)
