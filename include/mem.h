#ifndef MEM_H
#define MEM_H
#include <stddef.h>

typedef struct {
    size_t total_kb;
    size_t free_kb;
    size_t available_kb;
    size_t used_kb;
    size_t cached_kb;
} MemInfo;

MemInfo get_memory_info(void);
#endif
