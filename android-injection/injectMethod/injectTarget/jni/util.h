#ifndef _UTIL_
#define _UTIL_
#include <unistd.h>
#define UNIT_SIZE 4
union unit
{
	uint32_t int_val;
	char char_array[UNIT_SIZE];
};

/*
 *Read target process's memory [addr,addr+len) to buf
 *pid: the target process
 *addr: start address
 *buf: buffer to store readed data
 *len: read length
 */
void peek_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len);

/*
 * Write data to target process's memory
 * pid: the target process
 * addr: start address
 * buf: the data in buf will be wrote to target process's memory
 * len: write length
 */
void poke_data(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len);

/*
 * get the module base address through reading /proc/pid/maps
 * pid: the target process. pid<0 means the process self
 * module_name: the module name
 */
uint32_t get_module_base(pid_t pid, const char * module_name);

#endif