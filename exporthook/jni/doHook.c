#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>

#define LOG_TAG "ThomasKing"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

typedef int (*TK_InlineHookFunction)(void *, void *, void** );
TK_InlineHookFunction MYHookFunction = NULL;

char *hook_fun[] = {
    "rmdir",
    "access",
    "chmod",
    "chown",
    "execv",
    "execve",
    "mkdir",
    "fopen",
    "remove",
    "rename",
    "stat",
    "symlink",
    "statvfs",
};

char *hook_so[] = {
    "libc.so",
};

int *old_fun[13];

int buf_len = 256;
int added_fun0(const char *path){
    LOGI("Hook_rmdir %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *))old_fun[0])(strrpl(path, buf, buf_len));
    free(buf);
    return ret;
}

int added_fun1(const char* path, int mode){
    LOGI("Hook_access %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, int))old_fun[1])(strrpl(path, buf, buf_len), mode);
    free(buf);
    return ret;
}

int added_fun2(const char* path, mode_t mode){
    LOGI("Hook_chmod %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, int))old_fun[2])(strrpl(path, buf, buf_len), mode);
    free(buf);
    return ret;
}

int added_fun3(const char* path, uid_t uid, gid_t gid){
    LOGI("Hook_chown %s %d %d", path, uid, gid);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, uid_t, gid_t))old_fun[3])(strrpl(path, buf, buf_len), uid, gid);
    free(buf);
    return ret;
}

int added_fun4(const char *name, char *const *argv){
    LOGI("Hook_execv %s", name);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, char * const *))old_fun[4])(strrpl(name, buf, buf_len), argv);
    free(buf);
    return ret;
}

int added_fun5(const char* name, char* const* p1, char* const* p2){
    LOGI("Hook_execve %s", name);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, char* const*, char* const*))old_fun[5])(strrpl(name, buf, buf_len), p1, p2);
    free(buf);
    return ret;
}

int added_fun6(const char* path, mode_t mode){
    LOGI("Hook_mkdir %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, mode_t))old_fun[6])(strrpl(path, buf, buf_len), mode);
    free(buf);
    return ret;
}

int added_fun7(const char *file, const char *mode){
    LOGI("Hook_fopen %s", file);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, const char *))old_fun[7])(strrpl(file, buf, buf_len), mode);
    free(buf);
    return ret;
}

int added_fun8(const char *file){
    LOGI("Hook_remove %s", file);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *))old_fun[8])(strrpl(file, buf, buf_len));
    free(buf);
    return ret;
}

int added_fun9(const char* old_path, const char* new_path) {
    LOGI("Hook_rename %s to %s", old_path, new_path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, const char *))old_fun[9])(strrpl(old_path, buf, buf_len), strrpl(new_path, buf, buf_len));
    free(buf);
    return ret;
}

int added_fun10(const char *path, struct stat* sb){
    LOGI("Hook_stat %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, struct stat *))old_fun[10])(strrpl(path, buf, buf_len), sb);
    free(buf);
    return ret;
}

int added_fun11(const char* old_path, const char* new_path) {
    LOGI("Hook_symlink %s to %s", old_path, new_path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, const char *))old_fun[11])(strrpl(old_path, buf, buf_len), strrpl(old_path, buf, buf_len));
    free(buf);
    return ret;
}

int added_fun12(const char *path, struct statvfs* result){
    LOGI("Hook_statvfs %s", path);
    char* buf = malloc(buf_len);
    int ret = ((int(*)(const char *, struct statvfs *))old_fun[12])(strrpl(path, buf, buf_len), result);
    free(buf);
    return ret;
}

void HookAll(){
    int i;
    void *handle;
    int *fun;
    int *new_fun[13];

    new_fun[0] = (int*)added_fun0;
    new_fun[1] = (int*)added_fun1;
    new_fun[2] = (int*)added_fun2;
    new_fun[3] = (int*)added_fun3;
    new_fun[4] = (int*)added_fun4;
    new_fun[5] = (int*)added_fun5;
    new_fun[6] = (int*)added_fun6;
    new_fun[7] = (int*)added_fun7;
    new_fun[8] = (int*)added_fun8;
    new_fun[9] = (int*)added_fun9;
    new_fun[10] = (int*)added_fun10;
    new_fun[11] = (int*)added_fun11;
    new_fun[12] = (int*)added_fun12;

    for(i=0; i<13; i++){
        handle = dlopen(hook_so[0], RTLD_NOW);
        if(handle == NULL){
            LOGI("dlopen so[%s] failed", hook_so[0]);
            break;
        }
        fun = dlsym(handle, hook_fun[i]);
        if(fun == NULL){
            LOGI("dlsym fun[%s] failed", hook_fun[i]);
            break;
        }
        int ret = MYHookFunction(fun, new_fun[i], old_fun + i);
        LOGI("hook %d %s", i, ret == 0 ? "success" : "fail");
    }
}

JNIEXPORT void JNICALL Java_demo_hook_hookdemo_MainActivity_doHook(JNIEnv *env, jobject obj){
    // as I know, there is three native hook method: import table hook, export table hook, and inline hook
    // this project demo two hook methods: export table hook and inline hook
    // export table hook from http://bbs.pediy.com/showthread.php?t=196228, but do not have source code of libTKHooklib.so.
    // inline hook is from adbi project on github, LGPL.
    int inline_hook = 1;

    LOGI("Start %d", getpid());
    if (inline_hook == 1) {
        init_hook_libc();
    } else {// export table hook
        void *handle;
        handle = dlopen("libTKHooklib.so", RTLD_NOW);
        if(handle == NULL){
            LOGI("dlopen [%s]", dlerror());
            return ;
        }
        MYHookFunction = dlsym(handle, "TK_InlineHookFunction");
        if(MYHookFunction == NULL){
            LOGI("dlopen [%s]", dlerror());
            return ;
        }
        HookAll();// hook by export table
    }
    LOGI("End");
}
