#ifndef __HOOKUTILS_H__
#define __HOOKUTILS_H__

#include <stdint.h>
#include <jni.h>
#include "inlineHook.h"

#ifdef __cplusplus
extern "C" {
#endif

int hook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);
int unHook(uint32_t target_addr);
void hookESFun();
void hookGLESFun();
void unhookAllFun();

#ifdef __cplusplus
}
#endif

#endif

