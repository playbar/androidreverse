cmake_minimum_required(VERSION 3.4.1)

#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -std=gnu++11 -fPIC  -shared -fomit-frame-pointer")

add_library(doHook SHARED
          exportutil.c
          exporthook.c
          hook_libc.c
          hook_libc_arm.c
          dohook.c
            )

target_link_libraries(doHook
                      log)


