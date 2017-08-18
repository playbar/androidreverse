cmake_minimum_required(VERSION 3.4.1)

add_library(inject SHARED
            hellojni.c
            load.c
            inject.c)

target_link_libraries(inject
                      android
                      log)
