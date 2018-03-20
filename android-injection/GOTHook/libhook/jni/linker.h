#ifndef _LINKER_H_
#define _LINKER_H_

//http://androidxref.com/5.1.1_r6/xref/bionic/linker/linker.h
//http://androidxref.com/5.1.1_r6/xref/bionic/linker/linker.cpp
//http://androidxref.com/5.1.1_r6/xref/bionic/libc/include/link.h
#include <elf.h>

//struct Elf32_Dyn
//{
//	Elf32_Sword d_tag; // Type of dynamic table entry.
//	union
//	{
//		Elf32_Word d_val; // Integer value of entry.
//		Elf32_Addr d_ptr; // Pointer value of entry.
//	} d_un;
//};

//#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE-1))
#define PAGE_START(x)  ((x) & PAGE_MASK)
#define PAGE_OFFSET(x) ((x) & ~PAGE_MASK)
#define PAGE_END(x)    PAGE_START((x) + (PAGE_SIZE-1))

#define MAYBE_MAP_FLAG(x, from, to) (((x) & (from)) ? (to) : 0)
#define PFLAGS_TO_PROT(x)                 \
  (MAYBE_MAP_FLAG((x), PF_X, PROT_EXEC) | \
   MAYBE_MAP_FLAG((x), PF_R, PROT_READ) | \
   MAYBE_MAP_FLAG((x), PF_W, PROT_WRITE))

struct link_map
{
	Elf32_Addr l_addr;
	char * l_name;
	Elf32_Dyn * l_ld;
	struct link_map * l_next;
	struct link_map * l_prev;
};

typedef void (*linker_function_t)();

#define SOINFO_NAME_LEN 128
struct soinfo
{
	char name[SOINFO_NAME_LEN];
	const Elf32_Phdr* phdr;
	size_t phnum;
	Elf32_Addr entry;
	Elf32_Addr base;
	size_t size;

	uint32_t unused; // DO NOT USE, maintained for compatibility.

	Elf32_Dyn* dynamic;

	unsigned unused2; // DO NOT USE, maintained for compatibility
	unsigned unused3; // DO NOT USE, maintained for compatibility

	struct soinfo* next;
	unsigned flags;

	const char* strtab;
	Elf32_Sym* symtab;

	size_t nbucket;
	size_t nchain;
	unsigned* bucket;
	unsigned* chain;

	Elf32_Addr** plt_got;

	Elf32_Rel* plt_rel;
	size_t plt_rel_count;

	Elf32_Rel* rel;
	size_t rel_count;

	linker_function_t* preinit_array;
	size_t preinit_array_count;

	linker_function_t* init_array;
	size_t init_array_count;
	linker_function_t* fini_array;
	size_t fini_array_count;

	linker_function_t init_func;
	;
	linker_function_t fini_func;

	// ARM EABI section used for stack unwinding.
	unsigned* ARM_exidx;
	unsigned ARM_exidx_count;

	size_t ref_count;
	struct link_map link_map_head;

	int constructors_called;

	// When you read a virtual address from the ELF file, add this
	// value to get the corresponding address in the process' address space.
	Elf32_Addr load_bias;

	int has_text_relocations;
	int has_DT_SYMBOLIC;
};


//int phdr_table_unprotect_segments(const Elf32_Phdr*phdr_table, int phdr_count, Elf32_Addr load_bias)
//{
//	return phdr_table_set_load_prot(phdr_table, phdr_count, load_bias, PROT_WRITE);
//}
//
//phdr_table_set_load_prot(const Elf32_Phdr*phdr_table, int phdr_count, Elf32_Addr load_bias, int extra_prot_flags)
//{
//	const Elf32_Phdr *phdr = phdr_table;
//	const Elf32_Phdr* phdr_limit = phdr + phdr_count;
//	for (; phdr < phdr_limit; phdr++)
//	{
//		if (phdr->p_type != PT_LOAD || (phdr->p_flags & PF_W) != 0)
//			continue;
//		Elf32_Addr seg_page_start = PAGE_START(phdr->p_vaddr) + load_bias;
//		Elf32_Addr seg_page_end = PAGE_END(phdr->p_vaddr + phdr->p_memsz) + load_bias;
//		int ret = mprotect((void*) seg_page_start, seg_page_end - seg_page_start, PFLAGS_TO_PROT(phdr->p_flags) | extra_prot_flags);
//		if (ret < 0)
//		{
//			return -1;
//		}
//	}
//	return 0;
//}

#endif
