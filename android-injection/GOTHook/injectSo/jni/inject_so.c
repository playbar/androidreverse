#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <android/log.h>

#include "ptrace_help.h"

/***debug setting***/
#define LOGD(format, args...) __android_log_print(ANDROID_LOG_DEBUG, "inject",format,##args)
//#define LOGD(format, args...) printf(format,##args)
#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
#define DEBUG_MSG(format,args...) \
			LOGD(format, ##args)
#else
#define DEBUG_MSG(format,args...)
#endif
/***debug setting end***/

#define  MAX_PATH 0x100

#define REMOTE_ADDR( addr, local_base, remote_base ) ( (uint32_t)(addr) + (uint32_t)(remote_base) - (uint32_t)(local_base) )

const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";

//call mmap() in remote process
int call_mmap_remote(pid_t remote_pid, uint32_t mmap_remote_addr, struct pt_regs* regs)
{ // void *mmap(void *addr, size_t length, int prot, int flags,int fd, off_t offset);
	long parameters[6]; //init parameters of mmap()
	parameters[0] = 0; // addr
	parameters[1] = 0x4000; // size
	parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC; // prot
	parameters[3] = MAP_ANONYMOUS | MAP_PRIVATE; // flags
	parameters[4] = 0; //fd
	parameters[5] = 0; //offset
	if (ptrace_call(remote_pid, mmap_remote_addr, parameters, 6, regs) == -1) //call mmap
	{
		return -1;
	}
	return 0;
}
/*
 * inject library to remote process and call remote function to init hook
 */
int inject_remote_process_bk(pid_t remote_pid, const char *library_path, const char *remote_func_name, void *param, size_t param_size)
{
	DEBUG_MSG( "[+] Injecting process: %d\n", remote_pid);

	//step1.attach remote process
	if (ptrace_attach(remote_pid) == -1)
	{
		fprintf(stderr, "attach failed:%s\n", strerror(errno));
		return -1;
	}

	struct pt_regs regs, original_regs;
	if (ptrace_getregs(remote_pid, &regs) == -1)
	{
		fprintf(stderr, "getregs failed:%s\n", strerror(errno));
		return -1;
	}
	// save original registers
	memcpy(&original_regs, &regs, sizeof(regs));

	//step2. call mmap() in remote process
	uint32_t mmap_remote_addr = get_remote_addr(remote_pid, libc_path, (uint32_t) ((void *) mmap));
	if (0 == mmap_remote_addr)
	{
		fprintf(stderr, "get mmap address failed:%s\n", strerror(errno));
		return -1;
	}
	DEBUG_MSG( "[+] Remote mmap address: %X\n", mmap_remote_addr);

	//*** call mmap in remote process***
	DEBUG_MSG( "[+] Calling mmap in remote process.\n");
	if (call_mmap_remote(remote_pid, mmap_remote_addr, &regs) < 0)
	{
		fprintf(stderr, "call mmap in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	DEBUG_MSG( "[+] Remote process returned from mmap, return value=%x\n", (uint32_t)regs.ARM_r0);
	//***call mmap in remote process end***

	uint32_t map_base = (uint32_t) regs.ARM_r0;
	long parameters[10];

	//step3. call dlopen() in remote process to load libhook.so
	//***call dlopen***//
	uint32_t dlopen_remote_addr = get_remote_addr(remote_pid, linker_path, (uint32_t) ((void *) dlopen));
	uint32_t dlopen_param1_remote_addr = map_base;
	ptrace_pokedata(remote_pid, dlopen_param1_remote_addr, (uint8_t *) library_path, strlen(library_path) + 1);
	parameters[0] = dlopen_param1_remote_addr;
	parameters[1] = RTLD_GLOBAL;
	if (ptrace_call(remote_pid, dlopen_remote_addr, parameters, 2, &regs) == -1)
	{ //void *dlopen(const char *filename, int flag);
		fprintf(stderr, "call dlopen in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	//***call dlopen end***//

	//step4. call dlsym() in remote process to find remote function in libhook.so
	//***call dlsym***//
	uint32_t dlsym_remote_addr = get_remote_addr(remote_pid, linker_path, (uint32_t) ((void *) dlsym));
	uint32_t dlsym_param1_remote_addr = regs.ARM_r0;
	uint32_t dlsym_param2_remote_addr = dlopen_param1_remote_addr + MAX_PATH;
	ptrace_pokedata(remote_pid, dlsym_param2_remote_addr, (uint8_t *) remote_func_name, strlen(remote_func_name) + 1);
	parameters[0] = dlsym_param1_remote_addr;
	parameters[1] = dlsym_param2_remote_addr;
	if (ptrace_call(remote_pid, dlsym_remote_addr, parameters, 2, &regs) == -1)
	{ //void *dlsym(void *handle, const char *symbol);
		fprintf(stderr, "call dlopen in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	//***call dlsym end***//

	//step5. call remote function
	//***call remote_func_name***//
	uint32_t function_remote_addr = regs.ARM_r0;
	uint32_t function_param1_remote_addr = dlsym_param2_remote_addr + MAX_PATH;
	ptrace_pokedata(remote_pid, function_param1_remote_addr, (uint8_t *) param, param_size);
	uint32_t function_param2_remote_addr = function_param1_remote_addr + MAX_PATH;
	uint32_t function_param3_remote_addr = function_param2_remote_addr + 4;
	parameters[0] = function_param1_remote_addr;
	parameters[1] = function_param2_remote_addr;
	parameters[2] = function_param3_remote_addr;
	if (ptrace_call(remote_pid, function_remote_addr, parameters, 3, &regs) == -1)
	{
		fprintf(stderr, "call dlopen in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	//***call remote_func_name end***//

	//step6. hook global offset table
	union unit rel_addr;
	ptrace_peekdata(remote_pid, function_param2_remote_addr, (uint8_t*) (rel_addr.char_array), 4); //get relocation address from remote process
	union unit hook_method_addr;
	ptrace_peekdata(remote_pid, function_param3_remote_addr, (uint8_t*) (hook_method_addr.char_array), 4); //get hook method address
	if (0 == rel_addr.int_val || 0 == hook_method_addr.int_val)
	{
		DEBUG_MSG("[+] can not find hook method\n");
	} else
	{
		DEBUG_MSG("[+] rel_addr=%X,hook_method_addr=%X\n", rel_addr.int_val, hook_method_addr.int_val);
		ptrace_pokedata(remote_pid, rel_addr.int_val, (uint8_t *) (hook_method_addr.char_array), 4); //modify relocation address
	}
	//end hook global offset table

	//step7. continue remote process
	if (ptrace_setregs(remote_pid, &original_regs) == -1) //restore context
	{
		fprintf(stderr, "setregs failed:%s\n", strerror(errno));
		return -1;
	}
	ptrace_detach(remote_pid);

	return 0;
}
/*
 * inject library to remote process and call remote function to init hook
 */
int inject_remote_process(pid_t remote_pid, const char *library_path, const char *remote_hookinit_func, const char **hooking_remote_funcs_name, size_t funcs_count)
{
	DEBUG_MSG( "[+] Injecting process: %d\n", remote_pid);

	//step1.attach remote process
	if (ptrace_attach(remote_pid) == -1)
	{
		fprintf(stderr, "attach failed:%s\n", strerror(errno));
		return -1;
	}

	struct pt_regs regs, original_regs;
	if (ptrace_getregs(remote_pid, &regs) == -1)
	{
		fprintf(stderr, "getregs failed:%s\n", strerror(errno));
		return -1;
	}
	// save original registers
	memcpy(&original_regs, &regs, sizeof(regs));

	//step2. call mmap() in remote process
	uint32_t mmap_remote_addr = get_remote_addr(remote_pid, libc_path, (uint32_t) ((void *) mmap));
	if (0 == mmap_remote_addr)
	{
		fprintf(stderr, "get mmap address failed:%s\n", strerror(errno));
		return -1;
	}
	DEBUG_MSG( "[+] Remote mmap address: %X\n", mmap_remote_addr);

	//*** call mmap in remote process***
	DEBUG_MSG( "[+] Calling mmap in remote process.\n");
	if (call_mmap_remote(remote_pid, mmap_remote_addr, &regs) < 0)
	{
		fprintf(stderr, "call mmap in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	DEBUG_MSG( "[+] Remote process returned from mmap, return value=%x\n", (uint32_t)regs.ARM_r0);
	//***call mmap in remote process end***

	uint32_t map_base = (uint32_t) regs.ARM_r0;
	long parameters[10];

	//step3. call dlopen() in remote process to load libhook.so
	//***call dlopen***//
	DEBUG_MSG( "[+] Calling dlopen in remote process.\n");
	uint32_t dlopen_remote_addr = get_remote_addr(remote_pid, linker_path, (uint32_t) ((void *) dlopen));
	uint32_t dlopen_param1_remote_addr = map_base;
	ptrace_pokedata(remote_pid, dlopen_param1_remote_addr, (uint8_t *) library_path, strlen(library_path) + 1);
	parameters[0] = dlopen_param1_remote_addr;
	parameters[1] = RTLD_GLOBAL;
	if (ptrace_call(remote_pid, dlopen_remote_addr, parameters, 2, &regs) == -1)
	{ //void *dlopen(const char *filename, int flag);
		fprintf(stderr, "call dlopen in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	//***call dlopen end***//

	//step4. call dlsym() in remote process to find remote hook init function in libhook.so
	//***call dlsym***//
	DEBUG_MSG( "[+] Calling dlsym in remote process.\n");
	uint32_t dlsym_remote_addr = get_remote_addr(remote_pid, linker_path, (uint32_t) ((void *) dlsym));
	uint32_t dlsym_param1_remote_addr = regs.ARM_r0;
	uint32_t dlsym_param2_remote_addr = map_base; //dlopen_param1_remote_addr + MAX_PATH;
	ptrace_pokedata(remote_pid, dlsym_param2_remote_addr, (uint8_t *) remote_hookinit_func, strlen(remote_hookinit_func) + 1);
	parameters[0] = dlsym_param1_remote_addr;
	parameters[1] = dlsym_param2_remote_addr;
	if (ptrace_call(remote_pid, dlsym_remote_addr, parameters, 2, &regs) == -1)
	{ //void *dlsym(void *handle, const char *symbol);
		fprintf(stderr, "call dlopen in remote process failed:%s\n", strerror(errno));
		return -1;
	}
	//***call dlsym end***//


	//modify relocation address
	uint32_t hookinit_func_remote_addr = regs.ARM_r0;
	int i = 0;
	for (i = 0; i < funcs_count; i++)
	{
		//step5. call remote hook init function
		DEBUG_MSG( "[+] Calling hook init function in remote process.\n");
		uint32_t hookinit_param1_remote_addr = map_base; //dlsym_param2_remote_addr + MAX_PATH;
		ptrace_pokedata(remote_pid, hookinit_param1_remote_addr, (uint8_t *) hooking_remote_funcs_name[i], strlen(hooking_remote_funcs_name[i])+1);
		uint32_t hookinit_param2_remote_addr = hookinit_param1_remote_addr + MAX_PATH;
		uint32_t hookinit_param3_remote_addr = hookinit_param2_remote_addr + 4;
		parameters[0] = hookinit_param1_remote_addr;
		parameters[1] = hookinit_param2_remote_addr;
		parameters[2] = hookinit_param3_remote_addr;
		if (ptrace_call(remote_pid, hookinit_func_remote_addr, parameters, 3, &regs) == -1)
		{ //hook_init(char *plt_rel_name, uint32_t *rel_addr_ptr, uint32_t *hook_method_addr_ptr)
			fprintf(stderr, "call hook init function in remote process failed:%s\n", strerror(errno));
			return -1;
		}
		//call remote hook init function end

		//step6. hook global offset table
		DEBUG_MSG( "[+] Modify relocation address of %s.\n",hooking_remote_funcs_name[i]);
		union unit rel_addr;
		ptrace_peekdata(remote_pid, hookinit_param2_remote_addr, (uint8_t*) (rel_addr.char_array), 4); //get relocation address from remote process
		union unit hook_method_addr;
		ptrace_peekdata(remote_pid, hookinit_param3_remote_addr, (uint8_t*) (hook_method_addr.char_array), 4); //get hook method address
		if (0 == rel_addr.int_val || 0 == hook_method_addr.int_val)
		{
			DEBUG_MSG("[+] can not find hook method\n");
		} else
		{
			DEBUG_MSG("[+] method name=%s,rel_addr=%X,hook_method_addr=%X\n", hooking_remote_funcs_name[i],rel_addr.int_val, hook_method_addr.int_val);
			ptrace_pokedata(remote_pid, rel_addr.int_val, (uint8_t *) (hook_method_addr.char_array), 4); //modify relocation address
		}
		//end hook global offset table
	}

	//step7. continue remote process
	if (ptrace_setregs(remote_pid, &original_regs) == -1) //restore context
	{
		fprintf(stderr, "setregs failed:%s\n", strerror(errno));
		return -1;
	}
	ptrace_detach(remote_pid);

	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Usage:%s <pid to be traced>\n", argv[0]);
		return 1;
	}
	pid_t remote_pid = atoi(argv[1]);
	if (0 == remote_pid)
	{
		fprintf(stderr, "pid error!\n");
		return -1;
	}

#define HOOKING_FUNNC_COUNT 2
	const char *hooking_methods_name[HOOKING_FUNNC_COUNT] ={ "strlen","strcmp"};
//	char func_parm[]="strlen";
//	inject_remote_process_bk(remote_pid, "/data/local/tmp/libhook.so", "hook_init", func_parm, strlen(func_parm));
	inject_remote_process(remote_pid, "/data/local/tmp/libhook.so", "hook_init", hooking_methods_name, HOOKING_FUNNC_COUNT);
//	void hook_init(char *plt_rel_name, uint32_t *rel_addr_ptr, uint32_t *hook_method_addr_ptr)

}
