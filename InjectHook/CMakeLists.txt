cmake_minimum_required(VERSION 3.4.1)

ADD_DEFINITIONS (-DENABLE_DEBUG)

add_library( injectso SHARED src/main/cpp/injectso.cpp )
add_executable( inject  src/main/cpp/inject.c )
find_library( log-lib log )

target_link_libraries( injectso ${log-lib} )
target_link_libraries( inject ${log-lib} )
