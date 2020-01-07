#include <stdint.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "callstack.h"
#include <android/log.h>
#include <string.h>
#include <pthread.h>
#include "hookutils.h"
#include "hooklog.h"

void (*old_glTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border,
                            GLenum format, GLenum type, const void *pixels) = NULL;
void mj_glTexImage3DOES (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border,
                         GLenum format, GLenum type, const void *pixels)
{
    LOGITAG("mjgl","mj_glTexImage3DOES, tid=%d", gettid());
    return old_glTexImage3DOES(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void (*old_glTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
                               GLsizei depth, GLenum format, GLenum type, const void *pixels) = NULL;
void mj_glTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
                            GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    LOGITAG("mjgl","mj_glTexImage3DOES, tid=%d", gettid());
    return mj_glTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}


void (*old_glDebugMessageControlKHR)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled) = NULL;
void mj_glDebugMessageControlKHR (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
{
    LOGITAG("mjgl","mj_glDebugMessageControlKHR, tid=%d", gettid());
    return old_glDebugMessageControlKHR(source, type, severity, count, ids, enabled);
}

void (*old_glCompressedTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
                                      GLint border, GLsizei imageSize, const void *data) = NULL;
void mj_glCompressedTexImage3DOES (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
                                   GLint border, GLsizei imageSize, const void *data)
{
    LOGITAG("mjgl","mj_glCompressedTexImage3DOES, tid=%d", gettid());
    return old_glCompressedTexImage3DOES(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void (*old_glCompressedTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
                                         GLsizei depth, GLenum format, GLsizei imageSize, const void *data) = NULL;
void mj_glCompressedTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height,
                                      GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    LOGITAG("mjgl","mj_glCompressedTexSubImage3DOES, tid=%d", gettid());
    return old_glCompressedTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void (*old_glCopyTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void mj_glCopyTexSubImage3DOES (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    LOGITAG("mjgl","mj_glCopyTexSubImage3DOES, tid=%d", gettid());
    return old_glCopyTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

//void (*old_glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image) = NULL;
//void mj_glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image)
//{
//    LOGITAG("mjgl","mj_glEGLImageTargetRenderbufferStorageOES, tid=%d", gettid());
//    return old_glEGLImageTargetRenderbufferStorageOES(target, image);
//}
//
//void (*old_glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image) = NULL;
//void mj_glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
//{
//    LOGITAG("mjgl","mj_glEGLImageTargetTexture2DOES, tid=%d", gettid());
//    return old_glEGLImageTargetTexture2DOES(target, image);
//}

void (*old_glFramebufferTexture3DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) = NULL;
void mj_glFramebufferTexture3DOES (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    LOGITAG("mjgl","mj_glEGLImageTargetTexture2DOES, tid=%d", gettid());
    return old_glFramebufferTexture3DOES(target, attachment, textarget, texture, level, zoffset);
}

void (*old_glGetBufferPointervOES)(GLenum target, GLenum pname, void **params) = NULL;
void mj_glGetBufferPointervOES(GLenum target, GLenum pname, void **params)
{
    LOGITAG("mjgl","mj_glGetBufferPointervOES, tid=%d", gettid());
    return old_glGetBufferPointervOES(target, pname, params);
}

void (*old_glGetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) = NULL;
void mj_glGetProgramBinaryOES (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    LOGITAG("mjgl","mj_glGetProgramBinaryOES, tid=%d", gettid());
    return old_glGetProgramBinaryOES(program, bufSize, length, binaryFormat, binary);
}

void (*old_glProgramBinaryOES)(GLuint program, GLenum binaryFormat, const void *binary, GLint length) = NULL;
void mj_glProgramBinaryOES(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
    LOGITAG("mjgl","mj_glProgramBinaryOES, tid=%d", gettid());
    return old_glProgramBinaryOES(program, binaryFormat, binary, length);
}

void *(*old_glMapBufferOES)(GLenum target, GLenum access) = NULL;
void *mj_glMapBufferOES (GLenum target, GLenum access)
{
    LOGITAG("mjgl","mj_glMapBufferOES, tid=%d", gettid());
    return old_glMapBufferOES(target, access);
}

GLboolean (*old_glUnmapBufferOES)(GLenum target) = NULL;
GLboolean mj_glUnmapBufferOES (GLenum target)
{
    LOGITAG("mjgl","mj_glUnmapBufferOES, tid=%d", gettid());
    return old_glUnmapBufferOES(target);
}

void hookgl2extFun()
{
//    hook((uint32_t) glTexImage3DOES, (uint32_t)mj_glTexImage3DOES, (uint32_t **) &old_glTexImage3DOES);
//    hook((uint32_t) glTexSubImage3DOES, (uint32_t)mj_glTexSubImage3DOES, (uint32_t **) &old_glTexSubImage3DOES);
//    hook((uint32_t) glCompressedTexImage3DOES, (uint32_t)mj_glCompressedTexImage3DOES, (uint32_t **) &old_glCompressedTexImage3DOES);
//    hook((uint32_t) glCompressedTexSubImage3DOES, (uint32_t)mj_glCompressedTexSubImage3DOES, (uint32_t **) &old_glCompressedTexSubImage3DOES);
//    hook((uint32_t) glCopyTexSubImage3DOES, (uint32_t)mj_glCopyTexSubImage3DOES, (uint32_t **) &old_glCopyTexSubImage3DOES);
//    hook((uint32_t) glEGLImageTargetRenderbufferStorageOES, (uint32_t)mj_glEGLImageTargetRenderbufferStorageOES, (uint32_t **) &old_glEGLImageTargetRenderbufferStorageOES);
//    hook((uint32_t) glEGLImageTargetTexture2DOES, (uint32_t)mj_glEGLImageTargetTexture2DOES, (uint32_t **) &old_glEGLImageTargetTexture2DOES);
//    hook((uint32_t) glFramebufferTexture3DOES, (uint32_t)mj_glFramebufferTexture3DOES, (uint32_t **) &old_glFramebufferTexture3DOES);
//    hook((uint32_t) glGetBufferPointervOES, (uint32_t)mj_glGetBufferPointervOES, (uint32_t **) &old_glGetBufferPointervOES);
//    hook((uint32_t) glGetProgramBinaryOES, (uint32_t)mj_glGetProgramBinaryOES, (uint32_t **) &old_glGetProgramBinaryOES);
//    hook((uint32_t) glProgramBinaryOES, (uint32_t)mj_glProgramBinaryOES, (uint32_t **) &old_glProgramBinaryOES);
//    hook((uint32_t) glMapBufferOES, (uint32_t)mj_glMapBufferOES, (uint32_t **) &old_glMapBufferOES);
//    hook((uint32_t) glUnmapBufferOES, (uint32_t)mj_glUnmapBufferOES, (uint32_t **) &old_glUnmapBufferOES);


//    hook((uint32_t) glEGLImageTargetTexture2DOES, (uint32_t)mj_glCopyTexSubImage2D, (uint32_t **) &old_glCopyTexSubImage2D);
    return;
}


