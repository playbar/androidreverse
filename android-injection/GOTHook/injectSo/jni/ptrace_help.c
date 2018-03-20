#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>

#include "ptrace_help.h"


int ptrace_attach(pid_t pid)
{
	if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0)
	{
		return -1;
	}

	waitpid(pid, NULL, WUNTRACED); //wait process pid to suspend

	return 0;
}

int ptrace_detach(pid_t pid)
{
	if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0)
	{
		return -1;
	}

	return 0;
}

void ptrace_peekdata(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;
	uint32_t j = len / UNIT_SIZE;
	uint8_t *tmp_buf_ptr = buf;
	union unit peek_data;

	for (i = 0; i < j; i++)
	{
		peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL);
		memcpy(tmp_buf_ptr, peek_data.char_array, UNIT_SIZE);
		tmp_buf_ptr += UNIT_SIZE;
	}

	j = len % UNIT_SIZE;
	if (j != 0)
	{
		peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL);
		memcpy(tmp_buf_ptr, peek_data.char_array, j);
	}
}

void ptrace_pokedata(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;
	uint32_t j = len / UNIT_SIZE;
	uint8_t *tmp_buf_ptr = buf;
	union unit poke_data;

	for (i = 0; i < j; i++)
	{
		memcpy(poke_data.char_array, tmp_buf_ptr, UNIT_SIZE);
		ptrace(PTRACE_POKEDATA, pid, addr + i * UNIT_SIZE, poke_data.int_val);
		tmp_buf_ptr += UNIT_SIZE;
	}

	j = len % UNIT_SIZE;
	if (j != 0)
	{
		union unit tmp_peek_data;
		tmp_peek_data.int_val = ptrace(PTRACE_PEEKDATA, pid, addr + i * UNIT_SIZE, NULL);
		memcpy(poke_data.char_array, tmp_buf_ptr, j);
		memcpy(poke_data.char_array + j, tmp_peek_data.char_array + j, UNIT_SIZE - j);
		ptrace(PTRACE_POKEDATA, pid, addr + i * UNIT_SIZE, poke_data.int_val);
	}
}

int ptrace_getregs(pid_t pid, struct pt_regs* regs)
{
	if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0)
	{
		return -1;
	}
	return 0;
}

int ptrace_setregs(pid_t pid, struct pt_regs* regs)
{
	if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0)
	{
		return -1;
	}
	return 0;
}

int ptrace_continue(pid_t pid)
{
	if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0)
	{
		return -1;
	}
	return 0;
}

int ptrace_call(pid_t remote_pid, uint32_t remote_function_addr, long *params, uint32_t num_params, struct pt_regs* regs)
{
	uint32_t i;
	for (i = 0; i < num_params && i < 4; i++) // push first four parameters onto register[0->3]
	{
		regs->uregs[i] = params[i];
	}

	// push remained parameters onto stack
	if (i < num_params)
	{
		regs->ARM_sp -= (num_params - i) * sizeof(long); //full stack
		ptrace_pokedata(remote_pid, (uint32_t) regs->ARM_sp, (uint8_t *) (params + i), (num_params - i) * sizeof(long)); //wirte params to stack
	}

	regs->ARM_pc = remote_function_addr; //PC be assigned to remote function entry address

	//switch status
	if (regs->ARM_pc & 1)
	{ //thumb
		regs->ARM_pc &= (~1u); //align
		regs->ARM_cpsr |= CPSR_T_MASK; // T_mask=1
	} else
	{ //arm
		regs->ARM_cpsr &= ~CPSR_T_MASK; // T_mask=0
	}
	//0 is assigned to remote function's return address so that NPE(Null Pointer Exception) will be raised when remote function returns.
	//InjectSo process will capture this Exception and then could restore context of remote process after ptrace_call returns.
	regs->ARM_lr = 0;

	//run remote function
	if (ptrace_setregs(remote_pid, regs) == -1 || ptrace_continue(remote_pid) == -1)
	{
		return -1;
	}

	waitpid(remote_pid, NULL, WUNTRACED); //wait remote function return

	if (ptrace_getregs(remote_pid, regs) == -1) //get remote function return value->regs.ARM_r0
	{
		return -1;
	}

	return 0;
}


uint32_t get_module_base(pid_t pid, const char * module_name)
{
	if (NULL == module_name)
		return 0;

	char memory_maps_location[48];
	memset(memory_maps_location, 0, sizeof(memory_maps_location));
	if (pid < 0)
	{ //self process
		snprintf(memory_maps_location, sizeof(memory_maps_location), "/proc/self/maps");
	} else
	{
		snprintf(memory_maps_location, sizeof(memory_maps_location), "/proc/%d/maps", pid);
	}

	FILE *fp = fopen(memory_maps_location, "r");
	if (NULL == fp)
	{
		return 0;
	}

	uint32_t addr = 0;
	uint32_t line_len = 100 + strlen(module_name);
	char *line = (char *) malloc(line_len);
	while (1)
	{
		memset(line, 0, line_len);
		if (NULL == fgets(line, line_len, fp))
			break;

		if (strstr(line, module_name))
		{
			char *addr_end_position = strchr(line, '-');
			if (NULL == addr_end_position)
				continue;
			addr = strtoul(line, &addr_end_position, 16);
			if (0 == addr)
				continue;
			break;
		}
	}
	free(line);
	fclose(fp);

	return addr;
}

uint32_t get_remote_addr(pid_t remote_pid, const char* module_name, uint32_t local_addr)
{
	if (remote_pid < 0 || NULL == module_name)
	{
		return 0; //failed
	}

	uint32_t local_module_base_addr = get_module_base(-1, module_name); //get module base address in current process
	uint32_t remote_module_base_addr = get_module_base(remote_pid, module_name); //get module base address in remote process
	if (0 == local_module_base_addr || 0 == remote_module_base_addr)
	{
		return 0; //failed
	}

	uint32_t offset = local_addr - local_module_base_addr;
	if (offset < 0)
	{
		return 0; //failed
	}

	return offset + remote_module_base_addr;
}

int find_pid_by_pname(const char * process_name)
{
	DIR* dir = NULL;
	if (NULL == process_name || NULL == (dir = opendir("/proc")))
	{
		return -1;
	}

	pid_t pid = -1;
	struct dirent * entry;
	while ((entry = readdir(dir)) != NULL)
	{
		int tmp_pid = atoi(entry->d_name);
		if (0 == tmp_pid)
		{
			continue;
		}

		//init filename
		char filename[32];
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "/proc/%d/cmdline", tmp_pid);

		//read content of /proc/pid/cmdline
		FILE *fp = fopen(filename, "r");
		if (NULL == fp)
		{
			continue;
		}
		char cmdline[256];
		memset(cmdline, 0, sizeof(cmdline));
		fgets(cmdline, sizeof(cmdline), fp);
		fclose(fp);

		if (strcmp(process_name, cmdline) == 0)
		{ //process found
			pid = tmp_pid;
			break;
		}
	}

	closedir(dir);

	return pid;
}

