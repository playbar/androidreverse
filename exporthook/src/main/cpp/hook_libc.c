#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <jni.h>
#include <stdlib.h>

#include "exporthook.h"
//#include "replace_path.h"

#include <android/log.h>
#define LOG_TAG "ThomasKing"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

// this file is going to be compiled into a thumb mode binary

//void __attribute__ ((constructor)) my_init(void);

static struct hook_t eph[13];

extern int buf_len;

// arm version of hook
extern int my_rmdir_arm(const char *path);

int my_rmdir(const char *path){
    LOGI("Hook_rmdir %s", path);
    int (*orig_rmdir)(const char *path);
    orig_rmdir = (void*)eph[0].orig;

    hook_precall(&eph[0]);
    char* buf = malloc(buf_len);
    int res = orig_rmdir(strrpl(path, buf, buf_len));
    free(buf);
    hook_postcall(&eph[0]);

    return res;
}


extern int my_access_arm(const char* path, int mode);

int my_access(const char* path, int mode){
    LOGI("Hook_access %s", path);
    int (*orig_access)(const char* path, int mode);
    orig_access = (void*)eph[1].orig;

    hook_precall(&eph[1]);
    char* buf = malloc(buf_len);
    int res = orig_access(strrpl(path, buf, buf_len), mode);
    free(buf);
    hook_postcall(&eph[1]);

    return res;
}


extern int my_chmod_arm(const char* path, mode_t mode);

int my_chmod(const char* path, mode_t mode){
    LOGI("Hook_chmod %s", path);
    int (*orig_chmod)(const char* path, int mode);
    orig_chmod = (void*)eph[2].orig;

    hook_precall(&eph[2]);
    char* buf = malloc(buf_len);
    int res = orig_chmod(strrpl(path, buf, buf_len), mode);
    free(buf);
    hook_postcall(&eph[2]);

    return res;
}


extern int my_chown_arm(const char* path, uid_t uid, gid_t gid);

int my_chown(const char* path, uid_t uid, gid_t gid){
    LOGI("Hook_chown %s %d %d", path, uid, gid);
    int (*orig_own)(const char* path, uid_t uid, gid_t gid);
    orig_own = (void*)eph[3].orig;

    hook_precall(&eph[3]);
    char* buf = malloc(buf_len);
    int res = orig_own(strrpl(path, buf, buf_len), uid, gid);
    free(buf);
    hook_postcall(&eph[3]);

    return res;
}


extern int my_execv_arm(const char *name, char *const *argv);

int my_execv(const char *name, char *const *argv){
    LOGI("Hook_execv %s", name);
    int (*orig_execv)(const char *name, char *const *argv);
    orig_execv = (void*)eph[4].orig;

    hook_precall(&eph[4]);
    char* buf = malloc(buf_len);
    int res = orig_execv(strrpl(name, buf, buf_len), argv);
    free(buf);
    hook_postcall(&eph[4]);

    return res;
}


extern int my_execve_arm(const char* name, char* const* p1, char* const* p2);

int my_execve(const char* name, char* const* p1, char* const* p2){
    LOGI("Hook_execve %s", name);
    int (*orig_execve)(const char* name, char* const* p1, char* const* p2);
    orig_execve = (void*)eph[5].orig;

    hook_precall(&eph[5]);
    char* buf = malloc(buf_len);
    int res = orig_execve(strrpl(name, buf, buf_len), p1, p2);
    free(buf);
    hook_postcall(&eph[5]);

    return res;
}


extern int my_mkdir_arm(const char* path, mode_t mode);

int my_mkdir(const char* path, mode_t mode){
    LOGI("Hook_mkdir %s", path);
    int (*orig_mkdir)(const char* path, int mode);
    orig_mkdir = (void*)eph[6].orig;

    hook_precall(&eph[6]);
    char* buf = malloc(buf_len);
    int res = orig_mkdir(strrpl(path, buf, buf_len), mode);
    free(buf);
    hook_postcall(&eph[6]);

    return res;
}


extern int my_fopen_arm(const char *file, const char *mode);

int my_fopen(const char *file, const char *mode){
    LOGI("Hook_fopen %s", file);
    int (*orig_fopen)(const char* file, int mode);
    orig_fopen = (void*)eph[7].orig;

    hook_precall(&eph[7]);
    char* buf = malloc(buf_len);
    int res = orig_fopen(strrpl(file, buf, buf_len), mode);
    free(buf);
    hook_postcall(&eph[7]);

    return res;
}


extern int my_remove_arm(const char *file);

int my_remove(const char *file){
    LOGI("Hook_remove %s", file);
    int (*orig_remove)(const char *file);
    orig_remove = (void*)eph[8].orig;

    hook_precall(&eph[8]);
    char* buf = malloc(buf_len);
    int res = orig_remove(strrpl(file, buf, buf_len));
    free(buf);
    hook_postcall(&eph[8]);

    return res;
}


extern int my_rename_arm(const char* old_path, const char* new_path);

int my_rename(const char* old_path, const char* new_path) {
    LOGI("Hook_rename %s to %s", old_path, new_path);
    int (*orig_rename)(const char* old_path, const char* new_path);
    orig_rename = (void*)eph[9].orig;

    hook_precall(&eph[9]);
    char* buf = malloc(buf_len);
    int res = orig_rename(strrpl(old_path, buf, buf_len), strrpl(new_path, buf, buf_len));
    free(buf);
    hook_postcall(&eph[9]);

    return res;
}


extern int my_stat_arm(const char *path, struct stat* sb);

int my_stat(const char *path, struct stat* sb){
    LOGI("Hook_stat %s", path);
    int (*orig_stat)(const char *path, struct stat* sb);
    orig_stat = (void*)eph[10].orig;

    hook_precall(&eph[10]);
    char* buf = malloc(buf_len);
    int res = orig_stat(strrpl(path, buf, buf_len), sb);
    free(buf);
    hook_postcall(&eph[10]);

    return res;
}


extern int my_symlink_arm(const char* old_path, const char* new_path);

int my_symlink(const char* old_path, const char* new_path){
    LOGI("Hook_symlink %s to %s", old_path, new_path);
    int (*orig_symlink)(const char* old_path, const char* new_path);
    orig_symlink = (void*)eph[11].orig;

    hook_precall(&eph[11]);
    char* buf = malloc(buf_len);
    int res = orig_symlink(strrpl(old_path, buf, buf_len), strrpl(new_path, buf, buf_len));
    free(buf);
    hook_postcall(&eph[11]);

    return res;
}

extern int my_statvfs_arm(const char *path, struct statvfs* result);

int my_statvfs(const char *path, struct statvfs* result){
    LOGI("Hook_statvfs %s", path);
    int (*orig_statvfs)(const char *path, struct statvfs* result);
    orig_statvfs = (void*)eph[12].orig;

    hook_precall(&eph[12]);
    char* buf = malloc(buf_len);
    int res = orig_statvfs(strrpl(path, buf, buf_len), result);
    free(buf);
    hook_postcall(&eph[12]);

    return res;
}

void init_hook_libc(void)
{
    LOGI("%s started\n", __FILE__);

    exporthook(&eph[0], getpid(), "libc.", "rmdir", my_rmdir_arm, my_rmdir);
    exporthook(&eph[1], getpid(), "libc.", "access", my_access_arm, my_access);
    exporthook(&eph[2], getpid(), "libc.", "chmod", my_chmod_arm, my_chmod);
    exporthook(&eph[3], getpid(), "libc.", "chown", my_chown_arm, my_chown);
    exporthook(&eph[4], getpid(), "libc.", "execv", my_execv_arm, my_execv);
    exporthook(&eph[5], getpid(), "libc.", "execve", my_execve_arm, my_execve);
    exporthook(&eph[6], getpid(), "libc.", "mkdir", my_mkdir_arm, my_mkdir);
    exporthook(&eph[7], getpid(), "libc.", "fopen", my_fopen_arm, my_fopen);
    exporthook(&eph[8], getpid(), "libc.", "remove", my_remove_arm, my_remove);
    exporthook(&eph[9], getpid(), "libc.", "rename", my_rename_arm, my_rename);
    exporthook(&eph[10], getpid(), "libc.", "stat", my_stat_arm, my_stat);
    exporthook(&eph[11], getpid(), "libc.", "symlink", my_symlink_arm, my_symlink);
    exporthook(&eph[12], getpid(), "libc.", "statvfs", my_statvfs_arm, my_statvfs);
}

