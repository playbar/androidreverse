#include <string.h>
#include <SLES/OpenSLES.h>
#include <EGL/egl.h>
#include "config.h"
#include "elf_utils.h"
#include "injector.h"
#include "ptrace.h"
#include "utils.h"


// ./gothook com.gothook /data/local/tmp/libhook.so /data/app/com.gothook-1/lib/arm/libtriangle.so

// ./gothook com.inject /data/local/tmp/libhook.so /data/app/com.inject-1/lib/arm/libtriangle.so
int main(int argc, char const *argv[]) {
//    if (argc < 4) {
//        return -1;
//    }
//    const char* process_name = argv[1];
//    const char* hook_library_path = argv[2];
//    const char* target_library_path = argv[3];
    const char* process_name = "com.inject";
    const char* hook_library_path = "/data/local/tmp/libhook.so";
//    const char* target_library_path = "/data/user/0/com.inject/lib/libtriangle.so";
    const char* target_library_path = "/data/app/com.inject-1/lib/arm/libtriangle.so";
    pid_t pid = GetPid(process_name);
    printf("process: %s, pid=%d\n", process_name, pid);
    long so_handle = InjectLibrary(pid, hook_library_path);
    PtraceAttach(pid);
//    long hook_new_fun_addr = CallDlsym(pid, so_handle, "new_eglSwapBuffers");
//    long original_function_addr = GetRemoteFuctionAddr(pid, LIBEGL_PATH, (long)eglSwapBuffers);

    long hook_new_fun_addr = CallDlsym(pid, so_handle, "new_strcmp");
    long original_function_addr = GetRemoteFuctionAddr(pid, LIBC_PATH, (long)strcmp);
//
    long set_strcmp_add = CallDlsym(pid, so_handle, "set_strcmp");
    CallRemoteFunction( pid, set_strcmp_add, &original_function_addr, 1);

    PtraceDetach(pid);
    printf("---------------\n");
    if (DEBUG)
    {
        printf("hook_new_fun_addr: %lx, original_function_addr: %lx\n", hook_new_fun_addr, original_function_addr);
    }

//    PatchRemoteGot(pid, target_library_path, original_function_addr, hook_new_fun_addr);
    return 0;
}


extern int BufferAppend(int buffer, char *data, int len);


// for gaodeditu
int main_gao(int argc, char const *argv[]) {
//    if (argc < 4) {
//        return -1;
//    }
//    const char* process_name = argv[1];
//    const char* hook_library_path = argv[2];
//    const char* target_library_path = argv[3];
    const char* process_name = "com.autonavi.minimap";
    const char* hook_library_path = "/data/local/tmp/libhook.so";
    const char* target_library_path = "/data/app/com.autonavi.minimap-1/lib/arm/libdice.so";
    pid_t pid = GetPid(process_name);
    printf("process: %s, pid=%d\n", process_name, pid);
    long so_handle = InjectLibrary(pid, hook_library_path);
    PtraceAttach(pid);
//    long hook_new_fun_addr = CallDlsym(pid, so_handle, "new_eglSwapBuffers");
//    long original_function_addr = GetRemoteFuctionAddr(pid, LIBEGL_PATH, (long)eglSwapBuffers);

//    long hook_new_fun_addr = CallDlsym(pid, so_handle, "new_slCreateEngine");
//    long original_function_addr = GetRemoteFuctionAddr(pid, LIBC_PATH, (long)slCreateEngine);

    long hook_new_fun_addr = CallDlsym(pid, so_handle, "new_strcmp");
    long original_function_addr = GetRemoteFuctionAddr(pid, LIBC_PATH, (long)strcmp);

    long set_strcmp_add = CallDlsym(pid, so_handle, "set_strcmp");
    CallRemoteFunction( pid, set_strcmp_add, &original_function_addr, 1);

    PtraceDetach(pid);
    printf("---------------\n");
    if (DEBUG)
    {
        printf("hook_new_fun_addr: %lx, original_function_addr: %lx\n", hook_new_fun_addr, original_function_addr);
    }

    PatchRemoteGot(pid, target_library_path, original_function_addr, hook_new_fun_addr);
    return 0;
}