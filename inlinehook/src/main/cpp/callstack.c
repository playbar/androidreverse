#include <stdio.h>
#include <unwind.h>
#include <unistd.h>
#include <android/log.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "callstack.h"
#include "hooklog.h"

#define CALLSTACK_TAG "callstack"
#define BACKTRACE_SIZE  64


// 6f000000-6f01e000 rwxp 00000000 00:0c 16389419   /system/lib/libcomposer.so
// 012345678901234567890123456789012345678901234567890123456789
// 0         1         2         3         4         5

static mapinfo *parse_maps_line(char *line)
{
    mapinfo *mi;
    int len = strlen(line);

    if(len < 1){
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"error len\n");
        return 0;
    }
    line[--len] = 0;

    if(len < 40){
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"error max len %d\n", len);
        return 0;
    }
    if(line[20] != 'x'){
//        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"error executing \n");
        return 0;
    }

    //mi = dlmalloc(sizeof(mapinfo) + (len - 47));
    mi = malloc(sizeof(mapinfo) + (len - 47));

    if(mi == 0){
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"error memory\n");
        return 0;
    }

    mi->start = strtoul(line, 0, 16);
    mi->end = strtoul(line + 9, 0, 16);
    // mi->is_readable = (line[19] == 'r')? 1: 0;
    /* To be filled in parse_elf_info if the mapped section starts with
     * elf_header
     */
    mi->next = 0;
    strcpy(mi->name, line + 49);

    return mi;
}

mapinfo *init_mapinfo(int pid)
{
    mapinfo *head = NULL, *prev, *t;
    char data[1024];
    FILE *fp;
    sprintf(data, "/proc/%d/maps", pid);
    //    strcpy(data, "/proc/28037/maps");
    fp = fopen(data, "r");
    if(fp) {
        while(fgets(data, sizeof(data), fp)) {
            t = parse_maps_line(data);
            if(t){
                head = t;
                break;
            }
        }
        if(!head)
            return NULL;
        prev = head;
        while(fgets(data, sizeof(data), fp)) {
            mapinfo *mi = parse_maps_line(data);
            if(mi) {
                prev->next = mi;
                prev = mi;
            }
        }
        fclose(fp);
    }
    __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"init_mapinfo head=%x\n", head);
    return head;
}

void deinit_mapinfo(mapinfo *mi)
{
    mapinfo *del;
    while(mi) {
        del = mi;
        mi = mi->next;
        //dlfree(del);
        free(del);
    }
}

void print_mapinfo_mi(mapinfo *mi)
{
    while(mi) {
        // __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"name %s, addr 0x%x-0x%x, R%d\n", mi->name, mi->start, mi->end, mi->is_readable);
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"name %s, addr 0x%x-0x%x, R%d\n", mi->name, mi->start, mi->end);
        mi = mi->next;
    }
}
const mapinfo* find_map_info(const mapinfo* milist, uintptr_t addr) {
    const mapinfo* mi = milist;

    while (mi && mi->end <= addr) {
        mi = mi->next;
    }

    return mi;
}

int try_get_word(const mapinfo* map_info_list, uintptr_t ptr, uint32_t* out_value) {
    //ALOGV("try_get_word: reading word at 0x%08x", ptr);
    if (ptr & 3) {
        //ALOGV("try_get_word: invalid pointer 0x%08x", ptr);
        *out_value = 0xffffffffL;
        return 0;
    }
    //if (!is_readable_map(map_info_list, ptr)) {
    //ALOGV("try_get_word: pointer 0x%08x not in a readable map", ptr);
    //   *out_value = 0xffffffffL;
    //   return 0;
    //}
    if (!find_map_info(map_info_list, ptr)) {
        //ALOGV("try_get_word: pointer 0x%08x not in a readable map", ptr);
        *out_value = 0xffffffffL;
        return 0;
    }
    *out_value = *(uint32_t*)ptr;
    return 1;
}

static int try_get_half_word(const mapinfo* map_info_list, uint32_t pc, uint16_t* out_value) {
    uint32_t word;
    if (try_get_word(map_info_list, pc & ~2, &word)) {
        *out_value = pc & 2 ? word >> 16 : word & 0xffff;
        return 1;
    }
    return 0;
}

uintptr_t rewind_pc_arch(const mapinfo* map_info_list, uintptr_t pc) {
    if (pc & 1) {
        /* Thumb mode - need to check whether the bl(x) has long offset or not.
         * Examples:
         *
         * arm blx in the middle of thumb:
         * 187ae:       2300            movs    r3, #0
         * 187b0:       f7fe ee1c       blx     173ec
         * 187b4:       2c00            cmp     r4, #0
         *
         * arm bl in the middle of thumb:
         * 187d8:       1c20            adds    r0, r4, #0
         * 187da:       f136 fd15       bl      14f208
         * 187de:       2800            cmp     r0, #0
         *
         * pure thumb:
         * 18894:       189b            adds    r3, r3, r2
         * 18896:       4798            blx     r3
         * 18898:       b001            add     sp, #4
         */
        uint16_t prev1, prev2;
        if (try_get_half_word(map_info_list, pc - 5, &prev1)
            && ((prev1 & 0xf000) == 0xf000)
            && try_get_half_word(map_info_list, pc - 3, &prev2)
            && ((prev2 & 0xe000) == 0xe000)) {
            pc -= 4; // long offset
        } else {
            pc -= 2;
        }
    } else {
        /* ARM mode, all instructions are 32bit.  Yay! */
        pc -= 4;
    }
    return pc;
}

_Unwind_Reason_Code trace_function_1(__unwind_context *context, void *arg)
{
    stack_crawl_state_t* state = (stack_crawl_state_t*)arg;
    if (state->count) {
        intptr_t ip = (intptr_t)_Unwind_GetIP(context);
        if (ip) {
            state->addrs[0] = ip;
            state->addrs++;
            state->count--;
            return _URC_NO_REASON;
        }
    }
    /*
     * If we run out of space to record the address or 0 has been seen, stop
     * unwinding the stack.
     */
    return _URC_END_OF_STACK;
}

int get_backtrace(intptr_t* addrs, size_t max_entries)
{
    stack_crawl_state_t state;
    state.count = max_entries;
    state.addrs = (intptr_t*)addrs;
    _Unwind_Backtrace(trace_function_1, (void*)&state);
    return max_entries - state.count;
}

void funcC()
{
    int i;
    intptr_t backtrace[BACKTRACE_SIZE];
    size_t numEntries = get_backtrace(backtrace, BACKTRACE_SIZE);

    for(i=0; i< numEntries; i++)
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"0x%x\n", backtrace[i]);
}
void funcB()
{
    funcC();
}
void funcA()
{
    funcB();
}

int callstacktest()
{
    mapinfo* mi;
    mi = init_mapinfo(getpid());
    print_mapinfo_mi(mi);
    __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"Hello, World!!\n");
    funcA();
    deinit_mapinfo(mi);
    return 0;
}

void print_mapinfo()
{
    mapinfo *mi = init_mapinfo(getpid());
    print_mapinfo_mi(mi);
    deinit_mapinfo(mi);
}

void print_callstack()
{
    int i;
    intptr_t backtrace[BACKTRACE_SIZE];
    size_t numEntries = get_backtrace(backtrace, BACKTRACE_SIZE);
    for(i=0; i< numEntries; i++)
        __android_log_print(ANDROID_LOG_INFO, CALLSTACK_TAG,"0x%x\n", backtrace[i]);
    return;
}