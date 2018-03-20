#ifndef _PTRACE_HELP_
#define _PTRACE_HELP_

#include <unistd.h>

#define UNIT_SIZE 4
#define CPSR_T_MASK		( 1u << 5 )
union unit
{
	uint32_t int_val;
	char char_array[UNIT_SIZE];
};

/*
 * Attach to process pid, if attaching succeeds, process pid will be suspended.
 * return: 0 if success else -1
 */
int ptrace_attach(pid_t pid);

/*
 * Detach process pid
 * return: 0 if success else -1
 */
int ptrace_detach(pid_t pid);

/*
 *Read target process's memory [addr,addr+len) to buf
 *pid: the target process
 *addr: start address
 *buf: buffer to store readed data
 *len: read length
 */
void ptrace_peekdata(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len);

/*
 * Write data to target process's memory
 * pid: the target process
 * addr: start address
 * buf: the data in buf will be wrote to target process's memory
 * len: write length
 */
void ptrace_pokedata(pid_t pid, uint32_t addr, uint8_t *buf, uint32_t len);

/*
 * get registers' value
 * pid: process id
 * regs: the registers' value will be saved to regs
 * return: 0 if success else -1
 */
int ptrace_getregs(pid_t pid, struct pt_regs* regs);

/*
 * set registers' value
 * pid: process id
 * regs: set registers' value to regs
 * return: 0 if success else -1
 */
int ptrace_setregs(pid_t pid, struct pt_regs* regs);

/*
 * continue to run process pid
 * return: 0 if success else -1
 */
int ptrace_continue(pid_t pid);

/*
 *call function in remote process
 *remote_pid:
 *remote_function_addr: function address in remote process
 *params: the parameters of function
 *num_params: the number of parameters
 *regs: registers
 */
int ptrace_call(pid_t remote_pid, uint32_t remote_function_addr, long *params, uint32_t num_params, struct pt_regs* regs);

/*
 * get the module base address through reading /proc/pid/maps
 * pid: the target process. pid<0 means the process self
 * module_name: the module name
 * return: module base address. return 0 if failed
 */
uint32_t get_module_base(pid_t pid, const char * module_name);

/*
 * get remote address of local_addr in remote process
 * remote_pid: remote process id
 * module_name: the name of module where local_addr is located in. Current process and remote process both have loaded this module.
 * local_addr: local address in current process
 * return: corresponding remote address. return 0 if failed
 */
uint32_t get_remote_addr(pid_t remote_pid, const char* module_name, uint32_t local_addr);

/*
 * find pid according to process name.
 * /proc/pid/cmdline -> process name and process parameters.
 * return: pid. return -1 if failed
 */
int find_pid_by_pname(const char * process_name);

#endif
