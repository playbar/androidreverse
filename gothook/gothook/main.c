#include <string.h>
#include "config.h"
#include "elf_utils.h"
#include "injector.h"
#include "ptrace.h"
#include "utils.h"


// ./gothook com.inject /data/local/tmp/libhook.so /data/app/com.inject-1/lib/arm/libtriangle.so
int main(int argc, char const *argv[]) {
    if (argc < 4) {
        return -1;
    }
    const char* process_name = argv[1];
    const char* hook_library_path = argv[2];
    const char* target_library_path = argv[3];
    pid_t pid = GetPid(process_name);
    long so_handle = InjectLibrary(pid, hook_library_path);
    PtraceAttach(pid);
    long hook_new_strcmp_addr = CallDlsym(pid, so_handle, "new_strcmp");
//    long set_strcmp_add = CallDlsym(pid, so_handle, "set_strcmp");
    PtraceDetach(pid);
    long original_function_addr = GetRemoteFuctionAddr(pid, LIBC_PATH, (long)strcmp);
//    long original_function_addr = GetRemoteFuctionAddr(pid, target_library_path, (long)strcmp);
//    CallRemoteFunction( pid, set_strcmp_add, &original_function_addr, 1);
    printf("---------------\n");
    if (DEBUG)
    {
        printf("hook_new_strcmp_addr: %lx, original_function_addr: %lx\n", hook_new_strcmp_addr, original_function_addr);
    }

    PatchRemoteGot(pid, target_library_path, original_function_addr, hook_new_strcmp_addr);
    return 0;
}
