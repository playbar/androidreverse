#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hook.h"



#define LIBSF_PATH  "/data/app/com.inject-1/lib/arm/libtriangle.so"

int (*old_strcmp)(const char* c1, const char* c2) = -1;

int new_strcmp(const char* c1, const char* c2)
{
    LOGD("[+]new_strcmp called [+]\n");
    LOGD("[+] s1 = %s [+]\n", c1);
    LOGD("[+] s2 = %s [+]\n", c2);
    if (old_strcmp == -1)
        LOGD("[+] error:old_strcmp = -1 [+]\n");
    return 0;
}


int main(int argc, char** argv)
{
    LOGD("Hook success\n");
    LOGD("Start hooking\n");
    old_strcmp = strcmp;
    Hook(new_strcmp,LIBSF_PATH,strcmp);
    return 0;
}