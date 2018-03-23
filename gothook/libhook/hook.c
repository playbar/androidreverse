#include <android/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <SLES/OpenSLES.h>
#include "hookutils.h"
#include "log.h"



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


int hook_strcmp(const char* c1, const char* c2)
{
    LOGE("[+]hook_strcmp called [+]\n");
    LOGE("[+] s1 = %s [+]\n", c1);
    LOGE("[+] s2 = %s [+]\n", c2);
    LOGE("[+] success:old_strcmp ===============  [+]\n");
    return strcmp(c1, c2);
//    return 1;
}


typedef SLresult (*Fn_slCreateEngine)(
        SLObjectItf             *pEngine,
        SLuint32                numOptions,
        const SLEngineOption    *pEngineOptions,
        SLuint32                numInterfaces,
        const SLInterfaceID     *pInterfaceIds,
        const SLboolean         * pInterfaceRequired);
Fn_slCreateEngine old_slCreateEngine = slCreateEngine;


SLresult  new_slCreateEngine(
        SLObjectItf             *pEngine,
        SLuint32                numOptions,
        const SLEngineOption    *pEngineOptions,
        SLuint32                numInterfaces,
        const SLInterfaceID     *pInterfaceIds,
        const SLboolean         * pInterfaceRequired)
{
    LOGE("New slCreateEngine\n");
    if( old_slCreateEngine ) {
        return old_slCreateEngine(pEngine, numOptions, pEngineOptions,
                                  numInterfaces, pInterfaceIds, pInterfaceRequired);
    } else{
        return 0;
    }
}

typedef EGLBoolean (*Fn_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surf);
Fn_eglSwapBuffers old_eglSwapBuffers = eglSwapBuffers;

EGLBoolean new_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    LOGE("New eglSwapBuffers\n");
    if (old_eglSwapBuffers == -1)
        LOGE("error\n");
    return old_eglSwapBuffers(dpy, surface);
//    return EGL_FALSE;
}

void set_strcmp(fn_strcmp fn)
{
    hookESFun();
    LOGE("Fun:%s, Line:%d, fn=%0x, pid=%d\n", __FUNCTION__, __LINE__, fn, getpid());
    old_strcmp = fn;
}

