#include <sys/types.h>
#include <stdio.h>
#include <dlfcn.h>

extern int my_rmdir_arm(const char *path);
int my_rmdir_arm(const char *path)
{
    return my_rmdir(path);
}

extern int my_access_arm(const char* path, int mode);
int my_access_arm(const char* path, int mode)
{
    return my_access(path, mode);
}

extern int my_chmod_arm(const char* path, mode_t mode);
int my_chmod_arm(const char* path, mode_t mode)
{
    return my_chmod(path, mode);
}

extern int my_chown_arm(const char* path, uid_t uid, gid_t gid);
int my_chown_arm(const char* path, uid_t uid, gid_t gid)
{
    return my_chown(path, uid, gid);
}

extern int my_execv_arm(const char *name, char *const *argv);
int my_execv_arm(const char *name, char *const *argv)
{
    return my_execv(name, argv);
}

extern int my_execve_arm(const char* name, char* const* p1, char* const* p2);
int my_execve_arm(const char* name, char* const* p1, char* const* p2)
{
    return my_execve(name, p1, p2);
}

extern int my_mkdir_arm(const char* path, mode_t mode);
int my_mkdir_arm(const char* path, mode_t mode)
{
    return my_mkdir(path, mode);
}

extern int my_fopen_arm(const char *file, const char *mode);
int my_fopen_arm(const char *file, const char *mode)
{
    return my_fopen(file, mode);
}

extern int my_remove_arm(const char *file);
int my_remove_arm(const char *file)
{
    return my_remove(file);
}

extern int my_rename_arm(const char* old_path, const char* new_path);
int my_rename_arm(const char* old_path, const char* new_path)
{
    return my_rename(old_path, new_path);
}

int my_stat_arm(const char *path, struct stat* sb)
{
    return my_stat(path, sb);
}

extern int my_symlink_arm(const char* old_path, const char* new_path);
int my_symlink_arm(const char* old_path, const char* new_path)
{
    return my_symlink(old_path, new_path);
}

int my_statvfs_arm(const char *path, struct statvfs* result)
{
    return my_statvfs(path, result);
}