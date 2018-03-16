#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "log_util.h"

/**
 * get pid with process pakcage name
 * */
int find_pid_of(const char *process_name) ;

/**
 * get lib load address
 * */
void* get_lib_adress(pid_t pid, const char* module_name);

/**
 * get remote func address
 * */
void* get_remote_func_address(pid_t target_pid, const char* module_name,void* local_addr);
