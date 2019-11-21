cmake_minimum_required(VERSION 3.4.1)

add_library(crash SHARED
        tcmalloctest.cc
            )

# Include libraries needed for hello-jni lib
target_link_libraries(crash
                      android
                      log)
