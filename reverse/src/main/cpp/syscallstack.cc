#include <dlfcn.h>
#include "utils/CallStack.h"

#ifdef __cplusplus
extern "C" {
#endif

void sysCallstack();

#ifdef __cplusplus
}
#endif

void *handle = dlopen(NULL, RTLD_NOW);
void* (*callstack_handle)(void*,const char*, int32_t) = (void *(*)(void*,const char *, int32_t)) dlsym(handle, "_ZN7android9CallStackC2EPKci");

void sysCallstack()
{
//    android::CallStack cs("CallStack",1);
    void* p=malloc(sizeof(android::CallStack));
    callstack_handle(p,"CallStack",1);
}
