#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <errno.h>

void peek_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len)
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

void poke_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len)
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

uint32_t get_module_base(pid_t pid, const char * module_name)
{
	char memory_maps_location[48];
	//init memory_maps_location
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

