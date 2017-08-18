cmake_minimum_required(VERSION 3.4.1)

add_library(lib_utils SHARED IMPORTED)
set_target_properties(lib_utils PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libutils.so)

add_library(lib_android_runtime SHARED IMPORTED)
set_target_properties(lib_android_runtime PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libandroid_runtime.so)


add_library(lib_art SHARED IMPORTED)
set_target_properties(lib_art PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libart.so)

add_library(lib_binder SHARED IMPORTED)
set_target_properties(lib_binder PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libbinder.so)

add_library(lib_cutils SHARED IMPORTED)
set_target_properties(lib_cutils PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libcutils.so)

add_library(lib_dvm SHARED IMPORTED)
set_target_properties(lib_dvm PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libdvm.so)

add_library(lib_nativehelper SHARED IMPORTED)
set_target_properties(lib_nativehelper PROPERTIES IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libnativehelper.so)

add_library(SwatHookV2 SHARED
            hookutils.cpp
            DalvikMethodHook.cpp
            JavaMethodHook.cpp
            )

target_include_directories(SwatHookV2 PRIVATE
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/include
                           )

# Include libraries needed for hello-jni lib
target_link_libraries(SwatHookV2
                    android
                    lib_android_runtime
                    #lib_art
                    #lib_utils
                    #lib_binder
                    #lib_cutils
                    lib_dvm
                    #lib_nativehelper
                    log)
