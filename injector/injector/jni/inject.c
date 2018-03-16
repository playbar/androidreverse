#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "process_util.h"
#include "ptrace_util.h"

#define FUNCTION_NAME_ADDR_OFFSET 0x100
#define FUNCTION_PARAM_ADDR_OFFSET 0x200

const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";

/**
 * inject target process, remote execute inject_log_message function
 * target_pid: target process pid
 * library_path：payload.so path
 * function_name: remote execute function in payload.so
 * param：custom string param to print in logcat
 * param_size：custom string parm size
 *
 * */
int inject_remote_process(pid_t target_pid, const char *library_path,
                          const char *function_name, const char *param,
                          size_t param_size) {
    // function_name always inject_log_message
    LOGD("start injecting process< %d > \n", target_pid);

    // 1.attach remote process
    if (ptrace_attach(target_pid) < 0) {
        LOGD("attach error");
        return -1;
    }

    // 2.store remote process registers, reload after inject complete
    struct pt_regs regs, original_regs;
    if (ptrace_getregs(target_pid, &regs) < 0) {
        LOGD("getregs error");
        return -1;
    }
    memcpy(&original_regs, &regs, sizeof(regs));

    // 3.get remote process mmap function address
    void *target_mmap_addr =
            get_remote_func_address(target_pid, libc_path, (void *)mmap);
    LOGD("target mmap address: %x\n", target_mmap_addr);

    // 4.call remote process mmap function to get memory
    long parameters[6];
    parameters[0] = 0;                                  // addr
    parameters[1] = 0x400;                              // size
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC; // prot
    parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE;        // flags
    parameters[4] = 0;                                  // fd
    parameters[5] = 0;                                  // offset

    if (ptrace_call_wrapper(target_pid, "mmap", target_mmap_addr, parameters, 6,
                            &regs) < 0) {
        LOGD("call target mmap error");
        return -1;
    }
    // get addr of mmap allocate memory
    uint8_t *target_mmap_base = ptrace_retval(&regs);
    LOGD("target_mmap_base: %x\n", target_mmap_base);

    // 5.call dlopen in remote process to load my payload lib

    // dlopen define : void *dlopen(const char *filename, int flag);

    // get remote dlopen addr
    void *target_dlopen_addr =
            get_remote_func_address(target_pid, linker_path, (void *)dlopen);
    LOGD("target dlopen address: %x\n", target_dlopen_addr);

    // write payload path in remote process
    ptrace_writedata(target_pid, target_mmap_base, library_path,
                     strlen(library_path) + 1);

    // parameters[0] is payload path in remote process memory
    parameters[0] = target_mmap_base;
    parameters[1] = RTLD_NOW | RTLD_GLOBAL;
    // remote call dlopen function
    if (ptrace_call_wrapper(target_pid, "dlopen", target_dlopen_addr, parameters,
                            2, &regs) < 0) {
        LOGD("call target dlopen error");
        return -1;
    }

    void *target_so_handle = ptrace_retval(&regs);

    // 6.remote call dlsym function to get remote funcion addr inject_log_message
    // define in payload.so

    // dlsym define ：void *dlsym(void *handle, const char *symbol);

    // get remote dlsym function address
    void *target_dlsym_addr =
            get_remote_func_address(target_pid, linker_path, (void *)dlsym);
    LOGD("target dlsym address: %x\n", target_dlsym_addr);
    // write inject_log_message name in remote memory
    ptrace_writedata(target_pid, target_mmap_base + FUNCTION_NAME_ADDR_OFFSET,
                     function_name, strlen(function_name) + 1);

    parameters[0] = target_so_handle;
    parameters[1] = target_mmap_base + FUNCTION_NAME_ADDR_OFFSET;

    if (ptrace_call_wrapper(target_pid, "dlsym", target_dlsym_addr, parameters, 2,
                            &regs) < 0) {
        LOGD("call target dlsym error");
        return -1;
    }

    void *hook_func_addr = ptrace_retval(&regs);
    LOGD("target %s address: %x\n", function_name, target_dlsym_addr);
    // 7.remote call function inject_log_message
    ptrace_writedata(target_pid, target_mmap_base + FUNCTION_PARAM_ADDR_OFFSET,
                     param, strlen(param) + 1);
    parameters[0] = target_mmap_base + FUNCTION_PARAM_ADDR_OFFSET;

    if (ptrace_call_wrapper(target_pid, function_name, hook_func_addr, parameters,
                            1, &regs) < 0) {
        LOGD("call target %s error", function_name);
        return -1;
    }

    // 8.remote call dlclose
    // dlclose define :int dlclose(void *handle);
    void *target_dlclose_addr =
            get_remote_func_address(target_pid, linker_path, (void *)dlclose);
    parameters[0] = target_so_handle;

    if (ptrace_call_wrapper(target_pid, "dlclose", target_dlclose_addr,
                            parameters, 1, &regs) < -1) {
        LOGD("call target dlclose error");
        return -1;
    }
    // 9.reload backup registers
    ptrace_setregs(target_pid, &original_regs);
    // 10.detach
    ptrace_detach(target_pid);

    return 0;
}


// adb push inject /data/local/tmp
// adb shell chmod 777 /data/local/tmp/inject
// adb push libpayload.so /data/local/tmp
// adb shell /data/local/tmp/inject com.inject /data/local/tmp/libpayload.so string_you_want_print

int main(int argc, char **argv) {

    pid_t target_pid;
    printf("Fun:%s, Line:%d\n", __FUNCTION__, __LINE__ );
    char *remote_func_name = "inject_log_message";
    target_pid = find_pid_of(argv[1]);
//    if (argc < 4) {
//        printf("Usage inject processName payloadPath.\n");
//        return 0;
//    }
    if (-1 == target_pid) {
        LOGD("Can't find the process\n");
        return -1;
    }
    printf("Fun:%s, Line:%d\n", __FUNCTION__, __LINE__ );
    inject_remote_process(target_pid, "/data/local/tmp/libpayload.so", remote_func_name, "test",
                          strlen("test"));

    return 0;
}

//int main(int argc, char **argv) {
//
//    pid_t target_pid;
//    printf("Fun:%s, Line:%d\n", __FUNCTION__, __LINE__ );
//    char *remote_func_name = "inject_log_message";
//    target_pid = find_pid_of(argv[1]);
//    if (argc < 4) {
//        printf("Usage inject processName payloadPath.\n");
//        return 0;
//    }
//    if (-1 == target_pid) {
//        LOGD("Can't find the process\n");
//        return -1;
//    }
//    printf("Fun:%s, Line:%d\n", __FUNCTION__, __LINE__ );
//    inject_remote_process(target_pid, argv[2], remote_func_name, argv[3],
//                          strlen(argv[3]));
//
//    return 0;
//}
