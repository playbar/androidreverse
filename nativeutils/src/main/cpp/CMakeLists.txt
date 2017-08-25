cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fPIC  -shared")

add_library(nativeutils SHARED
            hellojni.cc
            load.cc
            libutils/CallStack.cpp
            libbacktrace/Backtrace.cpp
            )

target_include_directories(nativeutils PRIVATE
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/base/include
                          ${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include)

# Include libraries needed for hello-jni lib
target_link_libraries(nativeutils
                      android
                      log)
