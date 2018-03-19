cmake_minimum_required(VERSION 3.4.1)

add_library(triangle SHARED
            android_native_app_glue.c
            esShader.c
            esShapes.c
            esTransform.c
            esUtil.c
            esUtil_Android.c
            Hello_Triangle.c)

# Include libraries needed for hello-jni lib
target_link_libraries(triangle
                      android
                      log
                      EGL
                      GLESv3)
