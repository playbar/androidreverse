cmake_minimum_required(VERSION 3.4.1)

add_library(inlinehook SHARED
            hellojni.c
            inlineHook.c
            relocate.c
            )

target_link_libraries(inlinehook
                      android
                      log)
