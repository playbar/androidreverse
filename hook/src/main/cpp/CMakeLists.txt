cmake_minimum_required(VERSION 3.4.1)

add_library(hook SHARED
            hellojni.c
            load.c
            base.c
            hook.c
            util.c
            epoll.c
            epoll_arm.c)

# Include libraries needed for hello-jni lib
target_link_libraries(hook
                      android
                      log)
