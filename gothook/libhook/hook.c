#include <android/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <EGL/egl.h>

#define TAG "Hook Library"

//int my_printf(const char *format, ...) {
//  va_list args;
//  va_start(args, format);
//  int ret = __android_log_vprint(ANDROID_LOG_DEBUG, TAG, format, args);
//  va_end(args);
//  return ret;
//}

#define LOG_TAG "test"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef int (*fn_strcmp)(const char* c1, const char* c2);
fn_strcmp old_strcmp = strcmp;

int new_strcmp(const char* c1, const char* c2)
{
    LOGE("[+]new_strcmp called [+]\n");
    LOGE("[+] s1 = %s [+]\n", c1);
    LOGE("[+] s2 = %s [+]\n", c2);
    if (old_strcmp == 0) {
        LOGE("[+] error:old_strcmp = 0 [+]\n");
        return 0;
    }
    else{
        LOGE("[+] success:old_strcmp ===============  [+]\n");
        return old_strcmp(c1, c2);
    }
//    return 1;
}



typedef EGLBoolean (*Fn_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surf);
Fn_eglSwapBuffers old_eglSwapBuffers = eglSwapBuffers;

EGLBoolean new_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    LOGE("New eglSwapBuffers\n");
    if (old_eglSwapBuffers == -1)
        LOGE("error\n");
    return old_eglSwapBuffers(dpy, surface);
    return EGL_FALSE;
}

void set_strcmp(fn_strcmp fn)
{
    LOGE("Fun:%s, Line:%d, fn=%0x, pid=%d\n", __FUNCTION__, __LINE__, fn, getpid());
    old_strcmp = fn;
}