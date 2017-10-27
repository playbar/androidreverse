cmake_minimum_required(VERSION 3.4.1)

add_library(gothook SHARED
           elf_utils.c
           injector.c
           main.c
           ptrace.c
           utils.c
           hellojni.c
            )

target_link_libraries(gothook
                      android
                      log)


add_library(hook SHARED
            hook.c
            )

target_link_libraries(hook
                      log)