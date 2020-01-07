cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++14 -Wno-narrowing -fexceptions")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fvisibility=hidden -Wno-narrowing -fexceptions")


add_definitions(-DVGPV_arm_linux_android=1)

#file(GLOB TCMALLOC_BASE_SRC
#        tcmalloc/base/*.cc
#        tcmalloc/base/*.c)
#
#file(GLOB TCMALLOC_SRC
#        tcmalloc/*.cc)
#
#set(CPU_PROFILE_SRCS
#        tcmalloc/profiler.cc
#        )
#
set(MEMCHECK_SRCS
        valgrind/memcheck/mc_errors.c
        valgrind/memcheck/mc_leakcheck.c
        valgrind/memcheck/mc_main.c
        valgrind/memcheck/mc_malloc_wrappers.c
        valgrind/memcheck/mc_translate.c
        )

add_library(memcheck STATIC
        ${MEMCHECK_SRCS})
target_include_directories(memcheck PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/valgrind/include
        ${CMAKE_CURRENT_SOURCE_DIR}/valgrind/memcheck
        )

#############

add_library(tcmalloctest SHARED
        tcmalloctest.cc
        lib/library.cpp
        lib/test_library.cpp
        ${MEMCHECK_SRCS}
#        ${TCMALLOC_SRCS}
            )

target_link_libraries(tcmalloctest
                      android
                      log)

target_include_directories(tcmalloctest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/valgrind/include
        ${CMAKE_CURRENT_SOURCE_DIR}/valgrind/memcheck
        ${CMAKE_CURRENT_SOURCE_DIR}/valgrind/VEX/pub
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        )
