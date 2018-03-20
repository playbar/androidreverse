#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dlfcn.h>
#include <android/log.h>

#include "linker.h"

#define LOGD(format, args...) __android_log_print(ANDROID_LOG_DEBUG, "inject",format,##args)

void before_main() __attribute__((constructor)); //in .init_array section
void before_main()
{
	LOGD("I'm in\n");
	return;
}

//configure the lib you want to hook
const char lib_path[] = "/data/local/tmp/libtarget.so";

int hook_strlen(char * str)
{
	int ret = strlen(str);
//	LOGD("original result:%d\n", ret);
	return 1; //modify return value
}
int hook_strcmp(const char *str1, const char *str2)
{
	int ret = strcmp(str1, str2);
	return 0; //modify return value
}
struct HookItem
{
	char original_method[128];
	uint32_t hook_method_addr;
} hook_items[] = //configure the hook methods
{
	{ "strlen", (uint32_t) hook_strlen },
	{ "strcmp", (uint32_t) hook_strcmp }
};

/*
 *get hook method address
 *original_method: original method name
 *return: hook method address if success else 0
 */
uint32_t get_hook_method_addr(const char *original_method)
{
	if (NULL == original_method)
		return 0;

	int i = 0;
	for (i = 0; i < sizeof(hook_items) / sizeof(struct HookItem); i++)
	{
		if (strcmp(original_method, hook_items[i].original_method) == 0)
		{
			return hook_items[i].hook_method_addr;
		}
	}
	return 0;
}

/*
 * traverse the .plt_rel section and find the relocation address of plt_rel_name.
 */
uint32_t get_relocation_addr(const char *plt_rel_name, const char *library_path)
{
	void* soHandle = dlopen(library_path, RTLD_GLOBAL);
	if (NULL == soHandle)
	{
		LOGD("dlopen() failed! library = %s,error msg = %s\n", library_path, strerror(errno));
		return 0;
	}

	Elf32_Addr reloc = 0;
	struct soinfo* si = (struct soinfo*) soHandle;
	Elf32_Sym* symtab = si->symtab; //symbol table
	const char* strtab = si->strtab; //string table
	Elf32_Rel* rel = si->plt_rel; //relocation table
	unsigned count = si->plt_rel_count;

	size_t idx;
	for (idx = 0; idx < count; idx++, rel++)
	{
		unsigned type = ELF32_R_TYPE(rel->r_info); //relocation type
		unsigned sym_index = ELF32_R_SYM(rel->r_info); //symbol table index
		char *sym_name = (char *) (strtab + symtab[sym_index].st_name); //get symbol name
		if (strcmp(sym_name, plt_rel_name) == 0)
		{
			reloc = (Elf32_Addr) (rel->r_offset + si->load_bias);
			break;
		}
	}
	dlclose(soHandle);

	if (0 == reloc)
	{
		LOGD("%s not found in %s\n", plt_rel_name, library_path);
	}

	return reloc;
}

/*
 * plt_rel_name: the export function name
 * rel_addr_ptr: the relocation iteam address of this function in global offset table
 * hook_method_addr_ptr:*hook_method_addr_ptr is the hook method address. *hook_method_addr_ptr is going to be stored to rel_addr
 */
void hook_init(char *plt_rel_name, uint32_t *rel_addr_ptr, uint32_t *hook_method_addr_ptr)
{
	LOGD("original method name=%s\n", plt_rel_name);

	*rel_addr_ptr = 0;
	*hook_method_addr_ptr = 0;

	uint32_t hook_method_addr = get_hook_method_addr(plt_rel_name);
	if (0 == hook_method_addr)
	{
		LOGD("The hook method is not defined\n");
		return;
	}

	uint32_t reloc_addr = get_relocation_addr(plt_rel_name, lib_path);
	if (0 == reloc_addr)
	{
		LOGD("get relocation address failed!\n");
		return;
	}

	*hook_method_addr_ptr = hook_method_addr;
	*rel_addr_ptr = reloc_addr;
	return;
}

