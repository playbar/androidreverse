cmake_minimum_required(VERSION 3.4.1)

add_library(reverse SHARED
            hellojni.cc
            load.cc)

target_include_directories(gvrhook PRIVATE
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/gvrhook )

# Include libraries needed for hello-jni lib
target_link_libraries(reverse
                      android
                      log)
