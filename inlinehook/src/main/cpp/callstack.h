#ifndef __CALLSTACH_H__
#define __CALLSTACH_H__

#include <stdio.h>
#include <unwind.h>
#include <unistd.h>
#include <android/log.h>

typedef struct {
    struct _mapinfo* next;
    uintptr_t start;
    uintptr_t end;
//    char is_readable;  don't need, for excutable -> readable
//    bool is_executable;
//    void* data; // arbitrary data associated with the map by the user, initially NULL
    char name[];
} mapinfo;

typedef struct
{
    size_t count;
    intptr_t* addrs;
    const mapinfo* map_info_list;
} stack_crawl_state_t;

typedef struct _Unwind_Context __unwind_context;

static mapinfo *parse_maps_line(char *line);

mapinfo *init_mapinfo(int pid);

void deinit_mapinfo(mapinfo *mi);

void print_mapinfo_mi(mapinfo *mi);

const mapinfo* find_map_info(const mapinfo* milist, uintptr_t addr);

int try_get_word(const mapinfo* map_info_list, uintptr_t ptr, uint32_t* out_value);

static int try_get_half_word(const mapinfo* map_info_list, uint32_t pc, uint16_t* out_value);

uintptr_t rewind_pc_arch(const mapinfo* map_info_list, uintptr_t pc);

_Unwind_Reason_Code trace_function(__unwind_context *context, void *arg);

int get_backtrace(intptr_t* addrs, size_t max_entries);

int callstacktest();

void print_mapinfo();

void print_callstack();
#endif