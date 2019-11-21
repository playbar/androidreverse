cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-narrowing -fexceptions")

file(GLOB TCMALLOC_BASE_SRC
        tcmalloc/base/*.cc
        tcmalloc/base/*.c)

file(GLOB TCMALLOC_SRC
        tcmalloc/*.cc)

set(TCMALLOC_SRCS
        tcmalloc/central_freelist.cc
        tcmalloc/common.cc
        tcmalloc/emergency_malloc.cc
        tcmalloc/emergency_malloc_for_stacktrace.cc
        tcmalloc/fake_stacktrace_scope.cc
        tcmalloc/heap-checker.cc
        tcmalloc/heap-checker-bcad.cc
        tcmalloc/heap-profile-table.cc
#        tcmalloc/heap-profiler.cc
        tcmalloc/internal_logging.cc
        tcmalloc/malloc_extension.cc
        tcmalloc/malloc_hook.cc
        tcmalloc/maybe_threads.cc
        tcmalloc/memfs_malloc.cc
        tcmalloc/memory_region_map.cc
        tcmalloc/page_heap.cc
        tcmalloc/profile-handler.cc
        tcmalloc/profiledata.cc
#        tcmalloc/profiler.cc
        tcmalloc/raw_printer.cc
        tcmalloc/sampler.cc
        tcmalloc/span.cc
        tcmalloc/stack_trace_table.cc
        tcmalloc/stacktrace.cc
        tcmalloc/static_vars.cc
        tcmalloc/symbolize.cc
        tcmalloc/system-alloc.cc
        tcmalloc/tcmalloc.cc
        tcmalloc/thread_cache.cc
        )

add_library(tcmalloc STATIC
        ${TCMALLOC_BASE_SRC}
        ${TCMALLOC_SRCS})
target_include_directories(tcmalloc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tcmalloc
        )

#############

add_library(tcmalloctest SHARED
        tcmalloctest.cc
        ${TCMALLOC_BASE_SRC}
        ${TCMALLOC_SRCS}
            )

target_link_libraries(tcmalloctest
                      android
                      log)

target_include_directories(tcmalloctest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tcmalloc
        )
