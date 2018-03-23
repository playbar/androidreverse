cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -frtti -fexceptions")


add_definitions(-DTEST_printf)
#add_definitions(-DTEST_send)
add_definitions(-DTEST_sendto)
add_definitions(-DTEST_connect)
add_definitions(-DTEST_freeaddrinfo)
add_definitions(-DTEST_getaddrinfo)
add_definitions(-DTEST_case_001)


add_library(hook SHARED
            hellojni.c
            native-lib.cpp
            load.c
            allocator.c
            interceptor.c
            memory.c
            stack.c
            tools.c
            trampoline.c
            platforms/backend-linux/interceptor-linux.c
            platforms/backend-linux/memory-linux.c
            platforms/backend-posix/thread-posix.c
            platforms/arch-arm/instructions.c
            platforms/arch-arm/reader-arm.c
            platforms/arch-arm/reader-thumb.c
            platforms/arch-arm/regs-arm.c
            platforms/arch-arm/relocator-arm.c
            platforms/arch-arm/relocator-thumb.c
            platforms/arch-arm/writer-arm.c
            platforms/arch-arm/writer-thumb.c
            platforms/backend-arm/backend-arm-helper.c
            platforms/backend-arm/interceptor-arm.c
            platforms/backend-arm/thunker-arm.c
            kitzz/CommonKit/memory/common_memory_kit.c
            kitzz/LinuxKit/memory/linux_memory_kit.c
            kitzz/PosixKit/memory/posix_memory_kit.c
            kitzz/PosixKit/thread/posix_thread_kit.c
         )

target_include_directories(hook PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/kitzz
        ${CMAKE_CURRENT_SOURCE_DIR}/kitzz/include
        )

# Include libraries needed for hello-jni lib
target_link_libraries(hook
                      android
                      log)
