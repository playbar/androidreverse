#include <stdint.h>
#include <GLES2/gl2.h>
#include "callstack.h"
#include <android/log.h>
#include <string.h>
#include <EGL/egl.h>
#include <pthread.h>
//#include <glresource.h>
#include <EGL/eglext.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hookutils.h"
#include "hooklog.h"

int rendertid = 0;
int reprojectiontid = 0;
int viewport = 0;
int swapbuffer = 0;
int gismaligpu = 0;

void* get_module_base_1(pid_t pid,const char* module_name)
{
    FILE* fp;
    long addr = 0;
    char *pch;
    char filename[32];
    char line[1024];

    if(pid<0){
        snprintf(filename,sizeof(filename),"/proc/self/maps");
    }else{
        snprintf(filename,sizeof(filename),"/proc/%d/maps",pid);
    }
    fp = fopen(filename,"r");
    if(fp!=NULL){
        while(fgets(line,sizeof(line),fp)){
            if(strstr(line,module_name)){
                pch = strtok(line,"-");
                addr = strtoul(pch,NULL,16);
                if(addr==0x8000)
                    addr = 0;
                break;
            }
        }
        fclose(fp);
    }
    return (void*)addr;
}

bool HookToFunctionBase(int base, void * fpReplactToFunction, void ** fpOutRealFunction)
{
    bool bRet = false;
    void *pModule = get_module_base_1(getpid(), "libEGL.so");
    void *pFunc = (void*)((int)pModule + base + 1);
    if (registerInlineHook((uint32_t)pFunc, (uint32_t)fpReplactToFunction, (uint32_t **)fpOutRealFunction) == 0)
    {
        if (inlineHook((uint32_t)pFunc) == 0)
        {
            bRet = true;
        }
    }
    else
    {
        LOGE("Try registerInlineHook error!!, tid=%d", gettid());
    }

    return bRet;
}

EGLint (*old_eglGetError)(void) = NULL;
EGLint mj_eglGetError(void)
{
//    LOGITAG("nexusnote","mj_eglGetError, tid=%d", gettid());
    EGLint re = old_eglGetError();
    return re;
}
EGLDisplay (*old_eglGetDisplay)(EGLNativeDisplayType display_id) = NULL;
EGLDisplay mj_eglGetDisplay(EGLNativeDisplayType display_id)
{
    LOGITAG("nexusnote","mj_eglGetDisplay, tid=%d", gettid());
    return old_eglGetDisplay(display_id);
}

EGLBoolean (*old_eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor) = NULL;
EGLBoolean mj_eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    LOGITAG("nexusnote","mj_eglInitialize, tid=%d", gettid());
    return old_eglInitialize(dpy, major, minor);
}

EGLBoolean (*old_eglTerminate)(EGLDisplay dpy) = NULL;
EGLBoolean mj_eglTerminate(EGLDisplay dpy)
{
    LOGITAG("nexusnote","mj_eglTerminate");
    return old_eglTerminate(dpy);
}

const char * (*old_eglQueryString)(EGLDisplay dpy, EGLint name) = NULL;
const char * mj_eglQueryString(EGLDisplay dpy, EGLint name)
{
    const char *str = old_eglQueryString(dpy, name);
    LOGITAG("nexusnote","mj_eglQueryString, name=%s, tid=%d", str, gettid());
    return str;
}

EGLBoolean (*old_eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLBoolean mj_eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    LOGITAG("nexusnote","mj_eglGetConfigs");
    return old_eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLBoolean (*old_eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLBoolean mj_eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    LOGITAG("nexusnote","mj_eglChooseConfig, tid=%d", gettid());
    int i = 0;
    while(attrib_list[i] != EGL_NONE){
        LOGITAG("nexusnote", "attr:%0X, value:%d", attrib_list[i], attrib_list[i+1]);
        i = i+2;
    }
    return old_eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean (*old_eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) = NULL;
EGLBoolean mj_eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    EGLBoolean re = old_eglGetConfigAttrib(dpy, config, attribute, value);
    LOGITAG("nexusnote","mj_eglGetConfigAttrib, attribute=%d, value=%d, tid=%d", attribute, *value, gettid());
    return re;
}

EGLSurface (*old_eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) = NULL;
EGLSurface mj_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    EGLSurface surface = old_eglCreateWindowSurface(dpy, config, win, attrib_list);
    LOGITAG("nexusnote","mj_eglCreateWindowSurface surface=%X, tid=%d", surface, gettid());
    return surface;
}

EGLSurface (*old_eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,const EGLint *attrib_list) = NULL;
EGLSurface mj_eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    EGLSurface surface = old_eglCreatePbufferSurface(dpy, config, attrib_list);
    LOGITAG("nexusnote","mj_eglCreatePbufferSurface pbuffer=%X, tid=%d", surface, gettid());
    return surface;
}

EGLSurface (*old_eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) = NULL;
EGLSurface mj_eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    LOGITAG("nexusnote","mj_eglCreatePixmapSurface");
    EGLSurface surface = old_eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
    LOGITAG("nexusnote","eglcreate pixmap=%x", surface);
    return surface;
}

EGLBoolean (*old_eglDestroySurface)(EGLDisplay dpy, EGLSurface surface) = NULL;
EGLBoolean mj_eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    LOGITAG("nexusnote","mj_eglDestroySurface");
    return old_eglDestroySurface(dpy, surface);
}

EGLBoolean (*old_eglQuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) = NULL;
EGLBoolean mj_eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    LOGITAG("nexusnote","mj_eglQuerySurface, surface=%X, tid=%d", surface, gettid());
    return old_eglQuerySurface(dpy, surface, attribute, value);
}

EGLBoolean (*old_eglBindAPI)(EGLenum api) = NULL;
EGLBoolean mj_eglBindAPI(EGLenum api)
{
    LOGITAG("nexusnote","mj_eglBindAPI");
    return old_eglBindAPI(api);
}

EGLenum (*old_eglQueryAPI)(void) = NULL;
EGLenum mj_eglQueryAPI(void)
{
    LOGITAG("nexusnote","mj_eglQueryAPI");
    return old_eglQueryAPI();
}

EGLBoolean (*old_eglWaitClient)(void) = NULL;
EGLBoolean mj_eglWaitClient(void)
{
    LOGITAG("nexusnote","mj_eglWaitClient");
    return old_eglWaitClient();
}

EGLBoolean (*old_eglReleaseThread)(void) = NULL;
EGLBoolean mj_eglReleaseThread(void)
{
    LOGITAG("nexusnote","mj_eglReleaseThread");
    return old_eglReleaseThread();
}

EGLSurface (*old_eglCreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
                                                   EGLConfig config, const EGLint *attrib_list) = NULL;
EGLSurface mj_eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    LOGITAG("nexusnote","mj_eglCreatePbufferFromClientBuffer");
    return old_eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean (*old_eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) = NULL;
EGLBoolean mj_eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    LOGITAG("nexusnote","mj_eglSurfaceAttrib, surface=%X, tid=%d", surface, gettid());
    return old_eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean (*old_eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer) = NULL;
EGLBoolean mj_eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    LOGITAG("nexusnote","mj_eglBindTexImage");
    return old_eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean (*old_eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer) = NULL;
EGLBoolean mj_eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    LOGITAG("nexusnote","mj_eglReleaseTexImage");
    return old_eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean (*old_eglSwapInterval)(EGLDisplay dpy, EGLint interval) = NULL;
EGLBoolean mj_eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    LOGITAG("nexusnote","mj_eglSwapInterval");
    return old_eglSwapInterval(dpy, interval);
}

EGLContext (*old_eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) = NULL;
EGLContext mj_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    void *baseadd = get_module_base_1(getpid(), "libEGL.so");
    EGLContext context = old_eglCreateContext(dpy, config, share_context, attrib_list);
    LOGITAG("nexusnote","mj_eglCreateContext context=%0X, share_context=%0X, pid=%d", context, share_context, getpid());
    return context;
}

EGLBoolean (*old_eglDestroyContext)(EGLDisplay dpy, EGLContext ctx) = NULL;
EGLBoolean mj_eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    LOGITAG("nexusnote","mj_eglDestroyContext");
    return old_eglDestroyContext(dpy, ctx);
}

EGLBoolean (*old_eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) = NULL;
EGLBoolean mj_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    LOGITAG("nexusnote","mj_eglMakeCurrent draw=%0X, read=%0X, context=%X, tid=%d", draw, read, ctx, gettid());
    EGLBoolean re = old_eglMakeCurrent(dpy, draw, read, ctx);
    const char *glrender = glGetString(GL_RENDERER);
    if(glrender && strstr(glrender, "Mali") != NULL ){
//        gismaligpu = true;
    }
    return re;
}

EGLContext (*old_eglGetCurrentContext)(void) = NULL;
EGLContext mj_eglGetCurrentContext(void)
{
    EGLContext context = old_eglGetCurrentContext();
//    LOGITAG("nexusnote","mj_eglGetCurrentContext, context=%x, tid=%d", context, gettid());
    return context;
}

EGLSurface (*old_eglGetCurrentSurface)(EGLint readdraw) = NULL;
EGLSurface mj_eglGetCurrentSurface(EGLint readdraw)
{
    EGLSurface  surface = old_eglGetCurrentSurface(readdraw);
    LOGITAG("nexusnote","mj_eglGetCurrentSurface, surface=%X, tid=%d", surface, gettid());
    return surface;
}

EGLDisplay (*old_eglGetCurrentDisplay)(void) = NULL;
EGLDisplay mj_eglGetCurrentDisplay(void)
{
    LOGITAG("nexusnote","mj_eglGetCurrentDisplay");
    return old_eglGetCurrentDisplay();
}

EGLBoolean (*old_eglQueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) = NULL;
EGLBoolean mj_eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    LOGITAG("nexusnote","mj_eglQueryContext");
    return old_eglQueryContext(dpy, ctx, attribute, value);
}

EGLBoolean (*old_eglWaitGL)(void) = NULL;
EGLBoolean mj_eglWaitGL(void)
{
    LOGITAG("nexusnote","mj_eglWaitGL");
    return old_eglWaitGL();
}

EGLBoolean (*old_eglWaitNative)(EGLint engine) = NULL;
EGLBoolean mj_eglWaitNative(EGLint engine)
{
    LOGITAG("nexusnote","mj_eglWaitNative");
    return old_eglWaitNative(engine);
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = NULL;
EGLBoolean mj_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    LOGITAG("nexusnote","mj_eglSwapBuffers, oldSwapBuffer=%X surface=%X, begin tid=%d",
            old_eglSwapBuffers, surface, gettid());
//    sys_call_stack();

//    glViewport(0, 0, 300, 1440);
//    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
//    glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    EGLBoolean re =false;
//    re = eglWaitGL();

    re = old_eglSwapBuffers(dpy, surface);

    LOGITAG("nexusnote","mj_eglSwapBuffers, surface=%X, end tid=%d", surface, gettid());

    return re;
}

EGLBoolean (*old_eglSwapBuffersRuntime)(EGLDisplay dpy, EGLSurface surface) = NULL;
EGLBoolean mj_eglSwapBuffersRuntime(EGLDisplay dpy, EGLSurface surface)
{
    LOGITAG("nexusnote","mj_eglSwapBuffersRuntime, oldSwapBuffer=%X, surface=%X, begin tid=%d",
            old_eglSwapBuffersRuntime, surface, gettid());
//    sys_call_stack();

//    glViewport(0, 0, 300, 1440);
//    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
//    glClear(  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    EGLBoolean re =false;
    re = eglWaitGL();
    if(re) {
        re = old_eglSwapBuffersRuntime(dpy, surface);
    }

    LOGITAG("nexusnote","mj_eglSwapBuffersRuntime, surface=%X, end tid=%d", surface, gettid());

    return re;
}

EGLBoolean (*old_eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) = NULL;
EGLBoolean mj_eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    LOGITAG("nexusnote","mj_eglCopyBuffers");
    return mj_eglCopyBuffers(dpy, surface, target);
}

void (*pfun_gImageTargetTexture2DOES) (GLenum target, void *image);
void mjImageTargetTexture2DOES(GLenum target, void *image)
{
    LOGITAG("nexusnote","mjImageTargetTexture2DOES, image=%X, tid=%d", image, gettid());
    pfun_gImageTargetTexture2DOES(target, image);
//    if( gMJTexture[gIndex].eglImage == image && gMJTexture[gIndex].tid == gettid())
//    {
//        gMJTexture[gIndex].texStatus = TEX_STATUS_END;
//        ++gIndex;
//    }
    return;
}

void (*pfun_glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
void mjBindRenderbuffer (GLenum target, GLuint renderbuffer)
{
    LOGITAG("nexusnote","mjBindRenderbuffer, renderbuffer=%d, tid=%d", renderbuffer, gettid());
    return pfun_glBindRenderbuffer(target, renderbuffer);
}

void (*pfun_glEGLImageTargetRenderbufferStorageOES)(GLenum target, void *image) = NULL;
void mjEGLImageTargetRenderbufferStorageOES(GLenum target, void *image)
{
    LOGITAG("nexusnote","mjEGLImageTargetRenderbufferStorageOES, image=%0X, tid=%d", image, gettid());
    return pfun_glEGLImageTargetRenderbufferStorageOES(target, image);
}

void (*pfun_glBindTexture)(GLenum target, GLuint texture) = NULL;
void nexusnoteBindTexture (GLenum target, GLuint texture)
{
//    sys_call_stack();
//    if( gettid() == gRendThread)
//    {
//        static int icout = 0;
//        if( ++icout > 20)
//            texture = gTexture;
//    }
    LOGITAG("nexusnote", "nexusnoteBindTexture, texid=%d, tid=%d", texture, gettid());
    return pfun_glBindTexture(target, texture);
}

void (*pfun_glBindFramebuffer)(GLenum target, GLuint framebuffer) = NULL;
void mjBindFramebuffer (GLenum target, GLuint framebuffer)
{
//    sys_call_stack();
    LOGITAG("nexusnote", "mjBindFramebuffer, framebuffer=%d, tid=%d", framebuffer, gettid());
    return pfun_glBindFramebuffer(target, framebuffer);
}

void (*pfun_glGenTextures)(GLsizei n, GLuint *textures) = NULL;
void nexusnoteGenTextures (GLsizei n, GLuint *textures)
{
//    sys_call_stack();
//    if(gettid() == gRendThread )
//    {
//        if(gTexture == 0 ) {
//            gTexture = CreateSimpleTexture2D();
//        }
//    }
    pfun_glGenTextures(n, textures);
    LOGITAG("nexusnote", "nexusnoteGenTextures, texid=%d, tid=%d", textures[0], gettid());
//    if(gMJTexture[gIndex].texStatus == TEX_STATUS_BEGIN && gMJTexture[gIndex].tid == gettid())
//    {
//        gMJTexture[gIndex].textureid = textures[0];
//    }
    return;
}

void (*pfun_glDeleteTextures)(GLsizei n, const GLuint *textures) = NULL;
void nexusnoteDeleteTextures (GLsizei n, const GLuint *textures)
{
    pfun_glDeleteTextures(n, textures);
    LOGITAG("nexusnote", "nexusnoteDeleteTextures, texid=%d, tid=%d", textures[0], gettid());
    return;
}

void (*pfun_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
void nexusnoteFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    LOGITAG("nexusnote", "nexusnoteFramebufferTexture2D, texid=%d, tid=%d", texture, gettid());
    pfun_glFramebufferTexture2D(target, attachment, textarget, texture, level);
    return;
}

void (*pfun_glGenFramebuffers)(GLsizei n, GLuint *framebuffers) = NULL;
void nexusnoteGenFramebuffers (GLsizei n, GLuint *framebuffers)
{
    pfun_glGenFramebuffers(n, framebuffers);
    LOGITAG("nexusnote", "nexusnoteGenFramebuffers, framebuffers=%d, tid=%d", framebuffers[0], gettid());
    return;
}

void (*pfun_glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers) = NULL;
void nexusnoteGenRenderbuffers (GLsizei n, GLuint *renderbuffers)
{
    pfun_glGenRenderbuffers(n, renderbuffers);
    LOGITAG("nexusnote", "nexusnoteGenRenderbuffers, texid=%d, tid=%d", renderbuffers[0], gettid());
    return;
}

void (*pfun_glDrawArrays)(GLenum mode, GLint first, GLsizei count) = NULL;
void nexusnoteDrawArrays (GLenum mode, GLint first, GLsizei count)
{
    LOGITAG("nexusnote", "nexusnoteDrawArrays, tid=%d", gettid());
    pfun_glDrawArrays(mode, first, count);
    return;
}

void (*pfun_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices) = NULL;
void nexusnoteDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    LOGITAG("nexusnote", "nexusnoteDrawElements, count=%d, indices=%0X, tid=%d", count, indices, gettid());
    pfun_glDrawElements(mode, count, type, indices);
//    if(gRendThread == gettid()) {
//        unsigned char *pdata = malloc(960 * 1080 * 4);
//        glReadPixels(0, 0, 960, 1080, GL_RGB, GL_UNSIGNED_BYTE, pdata);
//        pic_data data;
//        data.width = 980;
//        data.height = 1080;
//        data.bit_depth = 8;
//        data.flag = 0;
//        data.rgba = pdata;
//        write_png_file("/sdcard/test_vr.png", &data);
//        free(pdata);
//    }
    return;
}

void (*pfun_glDrawBuffers)(GLsizei n, const GLenum *bufs) = NULL;
void nexusnoteDrawBuffers (GLsizei n, const GLenum *bufs)
{
    LOGITAG("nexusnote", "nexusnoteDrawBuffers, tid=%d", gettid());
    return pfun_glDrawBuffers(n, bufs);
}

void (*pfun_glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) = NULL;
void nexusnoteDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    LOGITAG("nexusnote", "nexusnoteDrawArraysInstanced, tid=%d", gettid());
    return pfun_glDrawArraysInstanced(mode, first, count, instancecount);
}


void (*pfun_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) = NULL;
void nexusnoteDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
    LOGITAG("nexusnote", "nexusnoteDrawElementsInstanced, tid=%d", gettid());
    return pfun_glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

void (*pfun_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                          GLint border, GLenum format, GLenum type, const void *pixels) = NULL;
void nexusnoteTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                   GLenum format, GLenum type, const void *pixels)
{
    LOGITAG("nexusnote", "nexusnoteTexImage2D, width=%d, height=%d, tid=%d", width, height, gettid());
    return pfun_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void (*pfun_glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) = NULL;
void nexusnoteBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    LOGITAG("nexusnote", "nexusnoteBindImageTexture, tid=%d", gettid());
    return pfun_glBindImageTexture(unit, texture, level, layered, layer, access, format);
}

void (*pfun_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) = NULL;
void  nexusnoteVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    LOGITAG("nexusnote", "nexusnoteVertexAttribPointer, index=%d, pointer=%0X, tid=%d", index, pointer, gettid());
    return pfun_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void (*pfun_glUseProgram)(GLuint program) = NULL;
void nexusnoteUseProgram (GLuint program)
{
    LOGITAG("nexusnote", "nexusnoteUseProgram, program=%d, tid=%d", program, gettid());
    return pfun_glUseProgram(program);
}

void (*pfun_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void nexusnoteViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
    LOGITAG("nexusnote", "nexusnoteViewport, x=%d, y=%d, w=%d, h=%d, tid=%d", x, y, width, height, gettid());
//    sys_call_stack();
//    if( rendertid != gettid())
//    {
//        glFinish();
//    }
//    if( gismaligpu && rendertid != gettid() && x == 0 )
//    {
//        swapbuffer = 1;
//    }
    return pfun_glViewport(x, y, width, height);

}

void (*pfun_glBindBuffer)(GLenum target, GLuint buffer) = NULL;
void nexusnoteBindBuffer (GLenum target, GLuint buffer)
{
    LOGITAG("nexusnote", "nexusnoteBindBuffer, buffer=%d, tid=%d", buffer, gettid());
    return pfun_glBindBuffer(target, buffer);
}

void (*pfun_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void nexusnoteRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    LOGITAG("nexusnote", "nexusnoteRenderbufferStorage, internalformat=%d, w=%d, h=%d, tid=%d", internalformat, width, height, gettid());
    return pfun_glRenderbufferStorage(target, internalformat, width, height);
}

void (*pfun_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void nexusnoteFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    LOGITAG("nexusnote", "nexusnoteFramebufferRenderbuffer, renderBuffer=%d, tid=%d", renderbuffer, gettid());
    return pfun_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}


void (*pfun_glClear)(GLbitfield mask) = NULL;
void nexusnoteClear (GLbitfield mask)
{
    LOGITAG("nexusnote", "nexusnoteClear, mask=%d, tid=%d", mask, gettid());
    return pfun_glClear(mask);
}

void (*pfun_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void nexusnoteClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    LOGITAG("nexusnote", "nexusnoteClearColor, tid=%d",  gettid());
    return pfun_glClearColor(red, green, blue, alpha);
}

void (*pfun_glClearDepthf)(GLfloat depth) = NULL;
void nexusnoteClearDepthf (GLfloat depth)
{
    LOGITAG("nexusnote", "glClearDepthf, tid=%d",  gettid());
    return pfun_glClearDepthf(depth);
}

void (*pfun_glClearStencil)(GLint s) = NULL;
void nexusnoteClearStencil (GLint s)
{
    LOGITAG("nexusnote", "nexusnoteClearStencil, tid=%d",  gettid());
    return pfun_glClearStencil(s);
}

int ImageWidth;
int ImageHeight;

EGLImageKHR (*pfun_eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) = NULL;
EGLImageKHR mjeglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
//    EGLDisplay display = eglGetCurrentDisplay();
    int i = 0;
    while(attrib_list[i] != EGL_NONE){
        LOGITAG("nexusnote", "attr:%0X, value:%d", attrib_list[i], attrib_list[i+1]);
        i = i+2;
    }

    EGLint eglImageAttributes[] = {
//            EGL_WIDTH, ImageWidth,
//            EGL_HEIGHT, ImageHeight,
//            EGL_MATCH_FORMAT_KHR,  EGL_FORMAT_RGBA_8888_KHR,
            EGL_GL_TEXTURE_LEVEL_KHR, 0,
            EGL_IMAGE_PRESERVED_KHR, EGL_FALSE,
            EGL_NONE};

    EGLint eglImgAttrs[] = { EGL_GL_TEXTURE_LEVEL_KHR, 0, EGL_IMAGE_PRESERVED_KHR,EGL_FALSE, EGL_NONE };
    EGLImageKHR img = pfun_eglCreateImageKHR(dpy, eglGetCurrentContext(),
                                             EGL_GL_TEXTURE_2D_KHR,
//                                             EGL_GL_RENDERBUFFER_KHR,
                                             buffer, eglImageAttributes);
    LOGITAG("nexusnote", "mjeglCreateImageKHR, image=%0X, context=%0X tid=%d", img, eglGetCurrentContext(), gettid());
    return img;

}

EGLClientBuffer (*pfun_eglCreateNativeClientBufferANDROID)(const EGLint *attrib_list) = NULL;
EGLClientBuffer mjeglCreateNativeClientBufferANDROID (const EGLint *attrib_list)
{
    int width = attrib_list[1];
    int height = attrib_list[3];
    ImageWidth = width;
    ImageHeight = height;
    eglMakeCurrent(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), eglGetCurrentSurface(EGL_READ), eglGetCurrentContext());
    GLuint textureId;
    glGenTextures ( 1, &textureId );
    glBindTexture ( GL_TEXTURE_2D, textureId );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    LOGITAG("nexusnote", "mjeglCreateNativeClientBufferANDROID, buffer=%0X, tid=%d",textureId , gettid());
    return textureId;

//    ///////////////////
//    GLuint renderBuffer, frameBuffer;
//    glGenFramebuffers(1, &frameBuffer);
//    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
//
//    glGenRenderbuffers(1, &renderBuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);
//    return renderBuffer;
}

//typedef void (*__eglMustCastToProperFunctionPointerType)(void);
EGLAPI __eglMustCastToProperFunctionPointerType (*old_eglGetProcAddress)(const char *procname) = NULL;
EGLAPI __eglMustCastToProperFunctionPointerType mj_eglGetProcAddress(const char *procname)
{
//    void *baseadd = get_module_base_1(getpid(), "libEGL.so");
//    const char *glrender = glGetString(GL_RENDERER);
//    sys_call_stack();
//    old_eglGetProcAddress(procname);
    __eglMustCastToProperFunctionPointerType pfun = old_eglGetProcAddress(procname);
    LOGITAG("nexusnote","mj_eglGetProcAddress, procename=%s, tid=%d", procname, gettid());
    if(strcmp(procname, "eglCreateImageKHR") == 0)
    {
        pfun_eglCreateImageKHR = pfun;
        pfun = mjeglCreateImageKHR;
    }
    if(strcmp(procname, "eglCreateNativeClientBufferANDROID") == 0 )
    {
        pfun_eglCreateNativeClientBufferANDROID = pfun;
        pfun = mjeglCreateNativeClientBufferANDROID;
    }
    if( strcmp(procname, "glEGLImageTargetTexture2DOES") == 0)
    {
        pfun_gImageTargetTexture2DOES = pfun;
        pfun = mjImageTargetTexture2DOES;
    }
    if(strcmp(procname, "glBindRenderbuffer") == 0)
    {
        pfun_glBindRenderbuffer = pfun;
        pfun = mjBindRenderbuffer;
    }
    if( strcmp(procname, "glEGLImageTargetRenderbufferStorageOES") == 0 )
    {
        pfun_glEGLImageTargetRenderbufferStorageOES = pfun;
        pfun = mjEGLImageTargetRenderbufferStorageOES;
    }
    if( strcmp(procname, "glBindTexture") == 0 )
    {
        pfun_glBindTexture = pfun;
        pfun = nexusnoteBindTexture;
    }
    if(strcmp(procname, "glBindFramebuffer") == 0 )
    {
        pfun_glBindFramebuffer = pfun;
        pfun = mjBindFramebuffer;
    }
    if(strcmp(procname, "glGenTextures") == 0 )
    {
        pfun_glGenTextures = pfun;
        pfun = nexusnoteGenTextures;
    }
    if(strcmp(procname, "glDeleteTextures") == 0)
    {
        pfun_glDeleteTextures = pfun;
        pfun = nexusnoteDeleteTextures;
    }
    if(strcmp(procname, "glFramebufferTexture2D") == 0 )
    {
        pfun_glFramebufferTexture2D = pfun;
        pfun = nexusnoteFramebufferTexture2D;
    }
    if(strcmp(procname, "glGenFramebuffers") == 0 )
    {
        pfun_glGenFramebuffers = pfun;
        pfun = nexusnoteGenFramebuffers;
    }
    if(strcmp(procname, "glGenRenderbuffers") == 0)
    {
        pfun_glGenRenderbuffers = pfun;
        pfun = nexusnoteGenRenderbuffers;
    }
    if(strcmp(procname, "glDrawArrays") == 0)
    {
        pfun_glDrawArrays = pfun;
        pfun = nexusnoteDrawArrays;
    }
    if(strcmp(procname, "glDrawElements") == 0)
    {
        pfun_glDrawElements = pfun;
        pfun = nexusnoteDrawElements;
    }
    if(strcmp(procname, "glDrawBuffers") == 0)
    {
        pfun_glDrawBuffers = pfun;
        pfun = nexusnoteDrawBuffers;
    }
    if(strcmp(procname, "glDrawArraysInstanced") == 0)
    {
        pfun_glDrawArraysInstanced = pfun;
        pfun = nexusnoteDrawArraysInstanced;
    }
    if(strcmp(procname, "glDrawElementsInstanced") == 0)
    {
        pfun_glDrawElementsInstanced = pfun;
        pfun = nexusnoteDrawElementsInstanced;
    }
    if(strcmp(procname, "glTexImage2D") == 0)
    {
        pfun_glTexImage2D = pfun;
        pfun = nexusnoteTexImage2D;
    }
    if(strcmp(procname, "glBindImageTexture") == 0)
    {
        pfun_glBindImageTexture = pfun;
        pfun = nexusnoteBindImageTexture;
    }
    if(strcmp(procname, "glVertexAttribPointer") == 0)
    {
        pfun_glVertexAttribPointer = pfun;
        pfun = nexusnoteVertexAttribPointer;
    }
    if(strcmp(procname, "glUseProgram") == 0)
    {
        pfun_glUseProgram = pfun;
        pfun = nexusnoteUseProgram;
    }
    if(strcmp(procname, "glViewport") == 0)
    {
        pfun_glViewport = pfun;
        pfun = nexusnoteViewport;
    }
    if(strcmp(procname, "glBindBuffer") == 0)
    {
        pfun_glBindBuffer = pfun;
        pfun = nexusnoteBindBuffer;
    }
    if(strcmp(procname, "glRenderbufferStorage") == 0)
    {
        pfun_glRenderbufferStorage = pfun;
        pfun = nexusnoteRenderbufferStorage;
    }
    if(strcmp(procname, "glFramebufferRenderbuffer") == 0)
    {
        pfun_glFramebufferRenderbuffer = pfun;
        pfun = nexusnoteFramebufferRenderbuffer;
    }
    if(strcmp(procname, "glClear") == 0 )
    {
        pfun_glClear = pfun;
        pfun = nexusnoteClear;
    }
    if(strcmp(procname, "glClearColor") == 0 )
    {
        pfun_glClearColor = pfun;
        pfun = nexusnoteClearColor;
    }
    if(strcmp(procname, "glClearDepthf") == 0) {
        pfun_glClearDepthf = pfun;
        pfun = nexusnoteClearDepthf;
    }
    if(strcmp(procname, "glClearStencil") ==0)
    {
        pfun_glClearStencil = pfun;
        pfun = nexusnoteClearStencil;
    }



    return pfun;
}


void hookEGLFun()
{
    LOGITAG("nexusnote", "hookEGLFun, tid=%d",  gettid());
    void *baseadd = get_module_base_1(getpid(), "libEGL.so");
    hook((uint32_t) eglGetError, (uint32_t)mj_eglGetError, (uint32_t **) &old_eglGetError);
    hook((uint32_t) eglGetDisplay, (uint32_t)mj_eglGetDisplay, (uint32_t **) &old_eglGetDisplay);
    hook((uint32_t) eglInitialize, (uint32_t)mj_eglInitialize, (uint32_t **) &old_eglInitialize);
    hook((uint32_t) eglTerminate, (uint32_t)mj_eglTerminate, (uint32_t **) &old_eglTerminate);
    hook((uint32_t) eglQueryString, (uint32_t)mj_eglQueryString, (uint32_t **) &old_eglQueryString);
    hook((uint32_t) eglGetConfigs, (uint32_t)mj_eglGetConfigs, (uint32_t **) &old_eglGetConfigs);
    hook((uint32_t) eglChooseConfig, (uint32_t)mj_eglChooseConfig, (uint32_t **) &old_eglChooseConfig);
    hook((uint32_t) eglGetConfigAttrib, (uint32_t)mj_eglGetConfigAttrib, (uint32_t **) &old_eglGetConfigAttrib);
    hook((uint32_t) eglCreateWindowSurface, (uint32_t)mj_eglCreateWindowSurface, (uint32_t **) &old_eglCreateWindowSurface);
    hook((uint32_t) eglCreatePbufferSurface, (uint32_t)mj_eglCreatePbufferSurface, (uint32_t **) &old_eglCreatePbufferSurface);
    hook((uint32_t) eglCreatePixmapSurface, (uint32_t)mj_eglCreatePixmapSurface, (uint32_t **) &old_eglCreatePixmapSurface);
    hook((uint32_t) eglDestroySurface, (uint32_t)mj_eglDestroySurface, (uint32_t **) &old_eglDestroySurface);
    hook((uint32_t) eglQuerySurface, (uint32_t)mj_eglQuerySurface, (uint32_t **) &old_eglQuerySurface);
    hook((uint32_t) eglBindAPI, (uint32_t)mj_eglBindAPI, (uint32_t **) &old_eglBindAPI);
    hook((uint32_t) eglQueryAPI, (uint32_t)mj_eglQueryAPI, (uint32_t **) &old_eglQueryAPI);
    hook((uint32_t) eglWaitClient, (uint32_t)mj_eglWaitClient, (uint32_t **) &old_eglWaitClient);
    hook((uint32_t) eglReleaseThread, (uint32_t)mj_eglReleaseThread, (uint32_t **) &old_eglReleaseThread);
    hook((uint32_t) eglCreatePbufferFromClientBuffer, (uint32_t)mj_eglCreatePbufferFromClientBuffer, (uint32_t **) &old_eglCreatePbufferFromClientBuffer);
    hook((uint32_t) eglSurfaceAttrib, (uint32_t)mj_eglSurfaceAttrib, (uint32_t **) &old_eglSurfaceAttrib);
    hook((uint32_t) eglBindTexImage, (uint32_t)mj_eglBindTexImage, (uint32_t **) &old_eglBindTexImage);
    hook((uint32_t) eglReleaseTexImage, (uint32_t)mj_eglReleaseTexImage, (uint32_t **) &old_eglReleaseTexImage);
    hook((uint32_t) eglSwapInterval, (uint32_t)mj_eglSwapInterval, (uint32_t **) &old_eglSwapInterval);
    hook((uint32_t) eglCreateContext, (uint32_t)mj_eglCreateContext, (uint32_t **) &old_eglCreateContext);
    hook((uint32_t) eglDestroyContext, (uint32_t)mj_eglDestroyContext, (uint32_t **) &old_eglDestroyContext);
    hook((uint32_t) eglMakeCurrent, (uint32_t)mj_eglMakeCurrent, (uint32_t **) &old_eglMakeCurrent);
    hook((uint32_t) eglGetCurrentContext, (uint32_t)mj_eglGetCurrentContext, (uint32_t **) &old_eglGetCurrentContext);
    hook((uint32_t) eglGetCurrentSurface, (uint32_t)mj_eglGetCurrentSurface, (uint32_t **) &old_eglGetCurrentSurface);
    hook((uint32_t) eglGetCurrentDisplay, (uint32_t)mj_eglGetCurrentDisplay, (uint32_t **) &old_eglGetCurrentDisplay);
    hook((uint32_t) eglQueryContext, (uint32_t)mj_eglQueryContext, (uint32_t **) &old_eglQueryContext);
    hook((uint32_t) eglWaitGL, (uint32_t)mj_eglWaitGL, (uint32_t **) &old_eglWaitGL);
    hook((uint32_t) eglWaitNative, (uint32_t)mj_eglWaitNative, (uint32_t **) &old_eglWaitNative);
    hook((uint32_t) eglSwapBuffers, (uint32_t)mj_eglSwapBuffers, (uint32_t **) &old_eglSwapBuffers);
    hook((uint32_t) eglCopyBuffers, (uint32_t)mj_eglCopyBuffers, (uint32_t **) &old_eglCopyBuffers);
//    hook((uint32_t) eglGetProcAddress, (uint32_t)mj_eglGetProcAddress, (uint32_t **) &old_eglGetProcAddress);


    hookImportFunInit();
    hookImportFun("libandroid_runtime.so", "eglSwapBuffers", (void *) mj_eglSwapBuffersRuntime, (void **) &old_eglSwapBuffersRuntime);
//    hookImportFun("libunity.so", "eglSwapBuffers", (void *) mj_eglSwapBuffers, (void **) &old_eglSwapBuffers);
//    hookImportFun("libgvr.so", "eglGetProcAddress", (void *) mj_eglGetProcAddress, (void **) &old_eglGetProcAddress);

//        HookToFunctionBase((uint32_t) 0x00012144, (uint32_t)mj_eglGetProcAddress, (uint32_t **) &old_eglGetProcAddress);

    return;
}


