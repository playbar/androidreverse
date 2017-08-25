cmake_minimum_required(VERSION 3.4.1)

add_subdirectory(lib)

add_library(reverse SHARED
            hellojni.cc
            assembly.s
            load.cc
            structreverse.cc)

# Include libraries needed for hello-jni lib
target_link_libraries(reverse
                      android
                      log)
