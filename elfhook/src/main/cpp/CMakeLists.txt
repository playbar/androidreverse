cmake_minimum_required(VERSION 3.4.1)

add_library(elfhook SHARED
            elf_common.cpp
            elf_hooker.cpp
            elf_module.cpp
            elfhook.cpp)

target_link_libraries(elfhook
                      android
                      log)
