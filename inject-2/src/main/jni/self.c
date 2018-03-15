//
// Created by syang on 2017/7/9.
//
#include <jni.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include "log.h"

void mmapNative(){
    LOGI("self mmap fun addr: %x", mmap);

    void *addr = 0;  // addr
    size_t size = 0x1000; // size
    int prot = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
    int flags = MAP_ANONYMOUS | MAP_PRIVATE; // flags
    int fd = 0; //fd
    int offset = 0; //offset

    int *ar = (int *) (mmap(addr, size, prot, flags, fd, offset));
    LOGI("self mmap addr: %x", ar);

    munmap(ar, size);
    return;
}

void dlNative(const char *path, const char *function, const char *param){
    LOGI("dlopen: %x; dlsym: %x; dlclose: %x; dlerror: %x", (size_t) dlopen, (size_t) dlsym, (size_t) dlclose, (size_t) dlerror);
    void *handle;
    void (*hook)(const char *);
    char *error;

    handle = dlopen(path, RTLD_LAZY | RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        LOGI("Can't open the so file, error: %s\n", dlerror());
    } else {
        LOGI("handle is %x\n", (size_t) handle);
    }

    hook = dlsym(handle, function);
    if (!hook) {
        LOGI("Can't find the funciton, error: %s\n", dlerror());
    } else {
        LOGI("hook is %x\n", (size_t) hook);
    }

    (*hook)(param);
    dlclose(handle);
    LOGI("Close");
    return;
}

// MainActivity
JNIEXPORT void JNICALL
Java_com_android_inject_MainActivity_mmapNative(JNIEnv *env, jobject instance) {
   mmapNative();
}

JNIEXPORT void JNICALL
Java_com_android_inject_MainActivity_dlNatvie(JNIEnv *env, jobject instance,
                                                          jstring path_, jstring function_,
                                                          jstring param_) {
    const char *path = (*env)->GetStringUTFChars(env, path_, 0);
    const char *function = (*env)->GetStringUTFChars(env, function_, 0);
    const char *param = (*env)->GetStringUTFChars(env, param_, 0);

    dlNative(path, function, param);

    (*env)->ReleaseStringUTFChars(env, path_, path);
    (*env)->ReleaseStringUTFChars(env, function_, function);
    (*env)->ReleaseStringUTFChars(env, param_, param);
    return;
}

// ChildService
JNIEXPORT void JNICALL
Java_com_android_inject_ChildService_mmapNative(JNIEnv *env, jobject instance) {
    mmapNative();
}

JNIEXPORT void JNICALL
Java_com_android_inject_ChildService_dlNatvie(JNIEnv *env, jobject instance,
                                                          jstring path_, jstring function_,
                                                          jstring param_) {
    const char *path = (*env)->GetStringUTFChars(env, path_, 0);
    const char *function = (*env)->GetStringUTFChars(env, function_, 0);
    const char *param = (*env)->GetStringUTFChars(env, param_, 0);

    dlNative(path, function, param);

    (*env)->ReleaseStringUTFChars(env, path_, path);
    (*env)->ReleaseStringUTFChars(env, function_, function);
    (*env)->ReleaseStringUTFChars(env, param_, param);
}