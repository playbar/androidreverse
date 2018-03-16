#include "process_util.h"
#define NAME_SIZE 32
#define BUF_SIZE 256

// find pid with package name
int find_pid_of(const char *process_name) {
  int id;
  pid_t pid = -1;
  DIR *dir;
  FILE *fp;
  char filename[NAME_SIZE];
  char cmdline[BUF_SIZE];

  struct dirent *entry;

  if (process_name == NULL)
    return -1;

  dir = opendir("/proc");
  if (dir == NULL)
    return -1;

  while ((entry = readdir(dir)) != NULL) {
    id = atoi(entry->d_name);
    if (id != 0) {
      sprintf(filename, "/proc/%d/cmdline", id);
      fp = fopen(filename, "r");
      if (fp) {
        fgets(cmdline, sizeof(cmdline), fp);
        fclose(fp);

        if (strcmp(process_name, cmdline) == 0) {
          /* process found */
          pid = id;
          break;
        }
      }
    }
  }

  closedir(dir);
  return pid;
}

/**
 * get lib load address from proc/pid/maps
 * */
void *get_lib_adress(pid_t pid, const char *module_name) {
  FILE *fp;
  long addr = 0;
  char *pch;
  char filename[32];
  char line[1024];

  if (pid < 0) {
    /* self process */
    snprintf(filename, sizeof(filename), "/proc/self/maps");
  } else {
    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
  }

  fp = fopen(filename, "r");

  if (fp != NULL) {
    while (fgets(line, sizeof(line), fp)) {
      // walk all maps entry
      if (strstr(line, module_name)) {
        pch = strtok(line, "-");
        addr = strtoul(pch, NULL, 16);

        if (addr == 0x8000)
          addr = 0;

        break;
      }
    }

    fclose(fp);
  }

  return (void *)addr;
}

/**
 * get remote process function address
 * */
void *get_remote_func_address(pid_t target_pid, const char *module_name,
                              void *local_addr) {
  void *local_handle, *remote_handle;
  local_handle = get_lib_adress(-1, module_name);
  remote_handle = get_lib_adress(target_pid, module_name);

  /* remote_func_addr = remote_lib_addr +（local_func_addr - local_lib_addr）*/
  void *ret_addr = (void *)((uint32_t)remote_handle + (uint32_t)local_addr -
                            (uint32_t)local_handle);
  return ret_addr;
}
