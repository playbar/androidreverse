#include <dlfcn.h>
#include "stdint.h"
#include "utils/CallStack.h"
#include "syscallstack.h"

void *handle = dlopen(NULL, RTLD_NOW);
void* (*callstack_handle)(void*,const char*, int32_t) = (void *(*)(void*,const char *, int32_t)) dlsym(handle, "_ZN7android9CallStackC2EPKci");

void sys_call_stack()
{
//    android::CallStack cs("CallStack",1);
    void* p=malloc(sizeof(android::CallStack));
    callstack_handle(p,"CallStack",1);
}
