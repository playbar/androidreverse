#include <stdint.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "callstack.h"
#include <android/log.h>
#include <string.h>
#include <EGL/egl.h>
#include <pthread.h>
#include <EGL/eglext.h>
#include <dlfcn.h>
#include "hookutils.h"
#include "hooklog.h"
#include "exporthook.h"
#include "andhook.h"

//egl

extern int rendertid;
extern int gismaligpu;

/////////////////////////////
//gles
void (*old_glEnable)(GLenum cap) = NULL;
void mj_glEnable(GLenum cap)
{
    LOGITAG("nexusnote", "mj_glEnable, cap=%d, tid=%d", cap, gettid());
    if( cap == 0x0BE2)
    {
        cap = 0x0BE2;
    }
    return old_glEnable(cap);
//    return;
}

void (*old_glDisable )(GLenum cap) = NULL;
void mj_glDisable (GLenum cap)
{
    LOGITAG("nexusnote", "mj_glDisable, cap=%d, tid=%d", cap, gettid());
    return old_glDisable(cap);
}

void (*old_glBlendFunc)(GLenum sfactor, GLenum dfactor) = NULL;
void mj_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    LOGITAG("nexusnote", "mj_glBlendFunc, sfactor=%d, dfactor=%d, tid=%d", sfactor, dfactor, gettid());
//    sfactor = dfactor = GL_ONE;
//    if( sfactor == 0 )
//    {
//        sfactor = 0.5;
//    }
    return old_glBlendFunc(sfactor, dfactor);
//    return;
}


void (*old_glShaderSource) (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) = NULL;
void mj_glShaderSource (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    char name[256] = {0};
    static int index = 1;
    sprintf(name, "/sdcard/shader_%d_%d.txt", gettid(), index);
    ++index;
    LOGITAG("nexusnote","mj_glShaderSource, shader=%d, name=%s, count=%d, tid=%d", shader, name, count, gettid());
//    FILE *pfile = fopen(name, "wb");
//    for(int i = 0; i < count; ++i){
//        int len = strlen(*string);
//        fwrite(*string, len, 1, pfile);
////        LOGITAG("nexusnote","shader: %s", *string);
//    }
//    fflush(pfile);
//    fclose(pfile);
    return old_glShaderSource(shader, count, string, length);
}

void  (*old_glBindBuffer) (GLenum target, GLuint buffer) = NULL;
void mj_glBindBuffer (GLenum target, GLuint buffer)
{
    LOGITAG("nexusnote","mj_glBindBuffer, bufferid=%d, tid=%d", buffer, gettid());
    return old_glBindBuffer(target, buffer);
}

void (*old_glGenFramebuffers)(GLsizei n, GLuint *framebuffers) = NULL;
void mj_glGenFramebuffers (GLsizei n, GLuint *framebuffers)
{
    LOGITAG("nexusnote", "mj_glGenFramebuffers, tid=%d", gettid());
    return old_glGenFramebuffers(n, framebuffers);
}

void (*old_glGenTextures)(GLsizei n, GLuint *textures) = NULL;
void mj_glGenTextures (GLsizei n, GLuint *textures)
{
    char tmp[1024] = {0};
    old_glGenTextures(n, textures);
    for( int i = 0; i < n; ++i ){
        sprintf(tmp, "%s, texid=%d", tmp, i, textures[i] );
    }
    LOGITAG("nexusnote", "mj_glGenTextures, %s, tid=%d", tmp,  gettid());
    return;
}

void (*old_glBindTexture)(GLenum target, GLuint texture) = NULL;
void mj_glBindTexture (GLenum target, GLuint texture)
{
    LOGITAG("nexusnote", "mj_glBindTexture, texid=%d, tid=%d", texture, gettid());
    return old_glBindTexture(target, texture);
}

void (*old_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
void mj_glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    LOGITAG("nexusnote", "mj_glFramebufferTexture2D, texid=%d, tid=%d", texture, gettid());
    return old_glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void (*old_glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers) = NULL;
void mj_glGenRenderbuffers (GLsizei n, GLuint *renderbuffers)
{
    LOGITAG("nexusnote", "mj_glGenRenderbuffers, tid=%d", gettid());
    return old_glGenRenderbuffers(n, renderbuffers);
}


void (*old_glBindFramebuffer)(GLenum target, GLuint framebuffer) = NULL;
void mj_glBindFramebuffer (GLenum target, GLuint framebuffer)
{
    rendertid = gettid();
    LOGITAG("nexusnote","mj_glBindFramebuffer, framebuffer=%d, tid=%d", framebuffer, gettid());
    return old_glBindFramebuffer(target, framebuffer);
}



void (*old_glBindRenderbuffer)(GLenum target, GLuint renderbuffer) = NULL;
void mj_glBindRenderbuffer (GLenum target, GLuint renderbuffer)
{
    LOGITAG("nexusnote","mj_glBindRenderbuffer, tid=%d", gettid());
    return old_glBindRenderbuffer(target, renderbuffer);
}

void (*old_glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) = NULL;
void mj_glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    LOGITAG("nexusnote","mj_glBindBufferRange, tid=%d", gettid());
    return old_glBindBufferRange(target, index, buffer, offset, size);
}

void (*old_glBindBufferBase) (GLenum target, GLuint index, GLuint buffer) = NULL;
void mj_glBindBufferBase (GLenum target, GLuint index, GLuint buffer)
{
    LOGITAG("nexusnote","mj_glBindBufferBase, tid=%d", gettid());
    return old_glBindBufferBase(target, index, buffer);
}

void (*old_glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) = NULL;
void mj_glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    LOGITAG("nexusnote","mj_glBufferData, tid=%d", gettid());
//    FILE *pfile = fopen("/sdcard/bufferdata.txt", "wb");
//    fwrite(data, size, 1, pfile);
//    fflush(pfile);
//    fclose(pfile);
    return old_glBufferData(target, size, data, usage);
}

void (*old_glDisableVertexAttribArray) (GLuint index) = NULL;
void mj_glDisableVertexAttribArray (GLuint index)
{
    LOGITAG("nexusnote","mj_glDisableVertexAttribArray, index=%d", index);
    return old_glDisableVertexAttribArray(index);
}

void (*old_glEnableVertexAttribArray) (GLuint index) = NULL;
void mj_glEnableVertexAttribArray (GLuint index)
{
//    LOGITAG("nexusnote","mj_glEnableVertexAttribArray, index=%d", index);
    return old_glEnableVertexAttribArray(index);
}

void (*old_glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) = NULL;
void mj_glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    LOGITAG("nexusnote","mj_glVertexAttribPointer, indx=%d, size=%d, type=%d, stride=%d, ptr=0x%0X, tid=%d", indx, size, type, stride, ptr, gettid());
    return old_glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
}

void (*old_glDrawArrays)(GLenum mode, GLint first, GLsizei count) = NULL;
void mj_glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
    LOGITAG("nexusnote","mj_glDrawArrays, tid=%d", gettid());
    return old_glDrawArrays(mode, first, count);
}

extern int swapbuffer;
extern EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
//extern EGLenum gtype;
//extern const EGLint *gattrib_list;
//extern EGLSyncKHR (*old_eglCreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list) = NULL;

void (*old_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices) = NULL;
void mj_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    LOGITAG("nexusnote","mj_glDrawElements, tid=%d", gettid());
//    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);  // Transparent background.
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    sys_call_stack();
    old_glDrawElements(mode, count, type, indices);
//    glFlush();
//    glFinish();

//    sleep(1);
    static int scount = 0;
//    glFlush();
    if( swapbuffer)
    {
//        glFinish();
//        glFlush();
//        old_eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
        ++scount;
//        if(scount == 2 )
        {
            old_eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
//            old_eglCreateSyncKHR(eglGetCurrentDisplay(), gtype, gattrib_list);
            swapbuffer = 0;
            scount = 0;
        }
    }

    return;
}

void (*old_glUseProgram) (GLuint program) = NULL;
void mj_glUseProgram (GLuint program)
{
    const unsigned char *version = glGetString(GL_VERSION);
//    const unsigned char *strexten = glGetString(GL_EXTENSIONS) ;
    LOGITAG("nexusnote","mj_glUseProgram, programid=%d, tid=%d", program, gettid());
//    LOGITAG("nexusnote","mj_glUseProgram, extension=%s, tid=%d",  strexten, gettid());
    return old_glUseProgram(program);
}

void (*old_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void mj_glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    LOGITAG("nexusnote","mj_glRenderbufferStorage, tid=%d", gettid());
    return old_glRenderbufferStorage(target, internalformat, width, height);
}


void (*old_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void mj_glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    LOGITAG("nexusnote","mj_glFramebufferRenderbuffer, target=%0X, attachment=%0X, renderbuffertarget=%d, tid=%d",
            target, attachment, renderbuffer, gettid());
    return old_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void (*old_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format,
                         GLenum type, const void *pixels) = NULL;
void mj_glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format,
                      GLenum type, const void *pixels)
{
    LOGITAG("nexusnote","mj_glTexImage2D, width=%d, height=%d, tid=%d", width, height, gettid());
    return old_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void (*old_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void mj_glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    LOGITAG("nexusnote","mj_glCopyTexSubImage2D, tid=%d", gettid());
    return old_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void (*old_glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                              GLbitfield mask, GLenum filter) = NULL;
void mj_glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                           GLbitfield mask, GLenum filter)
{
    LOGITAG("nexusnote","mj_glBlitFramebuffer, tid=%d", gettid());
    return old_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void (*old_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void mj_glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
    LOGITAG("nexusnote", "mj_glViewport, x=%d, y=%d, w=%d, h=%d, tid=%d", x, y, width, height, gettid());
//    if( x == 1031)
    {
//        x = 1031;
        width = width;
    }
//    if( x == 960)
//    {
//        x = 0;
//        width = width * 2;
//    }
    if( gismaligpu && rendertid != gettid() && x == 0 )
    {
        swapbuffer = 1;
    }
    return old_glViewport(x, y, width, height);
}

void (*old_glClear)(GLbitfield mask) = NULL;
void mj_glClear (GLbitfield mask)
{
    LOGITAG("nexusnote", "mj_glClear, mask=%d, tid=%d", mask, gettid());
    return old_glClear(mask);
}

void (*old_glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void mj_glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    LOGITAG("nexusnote", "mj_glClearColor, r=%06.5f, g=%06.5f, b=%06.5f, a=%06.5f tid=%d", red, green, blue, alpha,  gettid());
    return old_glClearColor(red, green, blue, alpha);
}

void* (*old_dlsym)(void*  handle, const char*  symbol) = NULL;
void* mj_dlsym(void*  handle, const char*  symbol)
{
    LOGITAG("mjhook", "mj_dlsym, symbol=%s", symbol);
//    sys_call_stack();
    void *pfun = old_dlsym(handle, symbol);
    if(strcmp(symbol, "glDrawElements") == 0)
    {
        old_glDrawElements = pfun;
        pfun = mj_glDrawElements;
    }
    return pfun;
}

void hookESFun()
{
//    hook((uint32_t) dlsym, (uint32_t)mj_dlsym, (uint32_t **) &old_dlsym);
//    hook((uint32_t) glDrawElements, (uint32_t)mj_glDrawElements, (uint32_t **) &old_glDrawElements);
    hook((uint32_t) glEnable, (uint32_t)mj_glEnable, (uint32_t **) &old_glEnable);
    hook((uint32_t) glBlendFunc, (uint32_t)mj_glBlendFunc, (uint32_t **) &old_glBlendFunc);
    hook((uint32_t) glShaderSource, (uint32_t)mj_glShaderSource, (uint32_t **) &old_glShaderSource);
    hook((uint32_t) glBindFramebuffer, (uint32_t)mj_glBindFramebuffer, (uint32_t **) &old_glBindFramebuffer);
    hook((uint32_t) glBindRenderbuffer, (uint32_t)mj_glBindRenderbuffer, (uint32_t **) &old_glBindRenderbuffer);
    hook((uint32_t) glBindBuffer, (uint32_t)mj_glBindBuffer, (uint32_t **) &old_glBindBuffer);
    hook((uint32_t) glGenFramebuffers, (uint32_t)mj_glGenFramebuffers, (uint32_t**)&old_glGenFramebuffers);
    hook((uint32_t) glGenTextures, (uint32_t)mj_glGenTextures, (uint32_t**)&old_glGenTextures);
    hook((uint32_t) glBindTexture, (uint32_t)mj_glBindTexture, (uint32_t**)&old_glBindTexture);
    hook((uint32_t) glFramebufferTexture2D, (uint32_t)mj_glFramebufferTexture2D, (uint32_t**)&old_glFramebufferTexture2D);

    hook((uint32_t) glGenRenderbuffers, (uint32_t)mj_glGenRenderbuffers, (uint32_t**)&old_glGenRenderbuffers);
//    hook((uint32_t) glBindBufferRange, (uint32_t)mj_glBindBufferRange, (uint32_t **) &old_glBindBufferRange);
//    hook((uint32_t) glBindBufferBase, (uint32_t)mj_glBindBufferBase, (uint32_t **) &old_glBindBufferBase);
    hook((uint32_t) glBufferData, (uint32_t)mj_glBufferData, (uint32_t **) &old_glBufferData);
    hook((uint32_t) glDisableVertexAttribArray, (uint32_t)mj_glDisableVertexAttribArray, (uint32_t **) &old_glDisableVertexAttribArray);
    hook((uint32_t) glEnableVertexAttribArray, (uint32_t)mj_glEnableVertexAttribArray, (uint32_t **) &old_glEnableVertexAttribArray);
    hook((uint32_t) glVertexAttribPointer, (uint32_t)mj_glVertexAttribPointer, (uint32_t **) &old_glVertexAttribPointer);
    hook((uint32_t) glDrawArrays, (uint32_t)mj_glDrawArrays, (uint32_t **) &old_glDrawArrays);
    hook((uint32_t) glDrawElements, (uint32_t)mj_glDrawElements, (uint32_t **) &old_glDrawElements);
    hook((uint32_t) glUseProgram, (uint32_t)mj_glUseProgram, (uint32_t **) &old_glUseProgram);
//    hook((uint32_t) glRenderbufferStorage, (uint32_t)mj_glRenderbufferStorage, (uint32_t **) &old_glRenderbufferStorage);
//    hook((uint32_t) glFramebufferRenderbuffer, (uint32_t)mj_glFramebufferRenderbuffer, (uint32_t **) &old_glFramebufferRenderbuffer);
//    hook((uint32_t) glTexImage2D, (uint32_t)mj_glTexImage2D, (uint32_t **) &old_glTexImage2D);
//    hook((uint32_t) glCopyTexSubImage2D, (uint32_t)mj_glCopyTexSubImage2D, (uint32_t **) &old_glCopyTexSubImage2D);
//    hook((uint32_t) glViewport, (uint32_t)mj_glViewport, (uint32_t **) &old_glViewport);
//
////    hook((uint32_t) glBlitFramebuffer, (uint32_t)mj_glBlitFramebuffer, (uint32_t **) &old_glBlitFramebuffer);
//    hook((uint32_t) glClear, (uint32_t)mj_glClear, (uint32_t **) &old_glClear);
//    hook((uint32_t) glClearColor, (uint32_t)mj_glClearColor, (uint32_t **) &old_glClearColor);

}

void hookExportHook()
{
//    hook_lwp(getpid(), "libGLESv2.", "glDrawElements", mj_glDrawElements, (void **)&old_glDrawElements);
//    hook_lwp(getpid(), "libgvr.", "dlsym", mj_dlsym, (void **)&old_dlsym);
//    hook((uint32_t) dlsym, (uint32_t)mj_dlsym, (uint32_t **) &old_dlsym);
//    void and_hook(void *orig_fcn, void* new_fcn, void **orig_fcn_ptr);
//    and_hook(glDrawElements, mj_glDrawElements, &old_glDrawElements);
    return;
}

void hookGLESFun()
{
    hookEGLFun();
//    hookEglextFun();
//    hookgl2extFun();
    hookESFun();
//    hookExportHook();
    return;
}

void unhookAllFun()
{
    inlineUnHookAll();
//    unHook((uint32_t)glShaderSource);
//    unHook((uint32_t)glBindBuffer);
//    unHook((uint32_t)glBindBufferRange);
//    unHook((uint32_t)glBindBufferBase);
//    unHook((uint32_t)glBufferData);
//    unHook((uint32_t)glEnableVertexAttribArray);
//    unHook((uint32_t)glVertexAttribPointer);
}
