#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>

#include "util.h"

void inject_process(pid_t pid)
{
	//char inject_code[] = "\x02\x20\x08\xBD"; //mov r0,#2; pop {R3,PC}
	char inject_code[] = "\x02\x20\xF7\x46"; //mov r0,#2; mov pc,lr.
	uint32_t base_addr = get_module_base(pid, "/data/local/tmp/libtarget.so");
	if (0==base_addr)
	{
		fprintf(stderr, "Can't get base address!\n");
		return;
	}

	printf("base address is %X\n",base_addr);

	//inject
	poke_data(pid, base_addr+0xcf8,inject_code, strlen(inject_code));
	printf("inject code success!");
}

int main(int argc, char*argv[])
{
	if (argc != 2)
	{
		printf("Usage:%s <pid to be traced>\n", argv[0]);
		return -1;
	}

	pid_t target_pid = atoi(argv[1]);
	if (0 == target_pid)
	{
		fprintf(stderr, "pid error!\n");
		return -1;
	}

	//attach process
	if (ptrace(PTRACE_ATTACH, target_pid, NULL, NULL) < 0)
	{
		fprintf(stderr, "attach failed! %s\n", strerror(errno));
		return -1;
	}

	inject_process(target_pid);

	//detach process
	if (ptrace(PTRACE_DETACH, target_pid, NULL, NULL) < 0)
	{
		fprintf(stderr, "detach failed! %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
