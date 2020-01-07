#include "hookutils.h"
#include "callstack.h"
#include "hooklog.h"

int hook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr)
{
    if (registerInlineHook(target_addr, new_addr, proto_addr) != ELE7EN_OK) {
        return -1;
    }
    if (inlineHook((uint32_t) target_addr) != ELE7EN_OK) {
        return -1;
    }

    return 0;
}

int unHook(uint32_t target_addr)
{
    if (inlineUnHook(target_addr) != ELE7EN_OK) {
        return -1;
    }

    return 0;
}


JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_printMapInfo(JNIEnv* env, jobject obj)
{
    print_mapinfo();
}

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_initHook(JNIEnv* env, jobject obj)
{
    LOGI("initHook begin");
    hookMemFun();
//    hookGLESFun();
//    hookThreadFun();
//    hookVulkanFun();
//    hookBadAlloc();
    LOGI("initHook after");
}

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_hookTest(JNIEnv* env, jobject obj)
{

}

JNIEXPORT void JNICALL Java_com_inline_hook_GLESHook_unInitHook(JNIEnv* env, jobject obj)
{
    unhookAllFun();
}
