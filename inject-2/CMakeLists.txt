cmake_minimum_required(VERSION 3.4.1)

add_library( hello SHARED src/main/jni/hello.c )
add_library( self SHARED src/main/jni/self.c )
add_library( inject SHARED src/main/jni/inject/inject.c )

#add_library( inject SHARED src/main/cpp/inject.cpp )

#add_executable( inject ${SOURCE_INJECT_FILES})
#add_executable( inject2 src/main/cpp/inject/inject2.c)
find_library( log-lib log )

target_link_libraries( hello ${log-lib} EGL )
target_link_libraries( self ${log-lib} )
target_link_libraries( inject ${log-lib} )
