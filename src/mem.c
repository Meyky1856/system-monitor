#include "mem.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

MemInfo get_memory_info(void) {
    MemInfo mem = {0};
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return mem;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        long val = -1;
        if ((val = parse_key_value_line(line, "MemTotal")) >= 0) mem.total_kb = val;
        else if ((val = parse_key_value_line(line, "MemFree")) >= 0) mem.free_kb = val;
        else if ((val = parse_key_value_line(line, "MemAvailable")) >= 0) mem.available_kb = val;
        else if ((val = parse_key_value_line(line, "Cached")) >= 0) mem.cached_kb = val;
    }
    fclose(fp);

    if (mem.available_kb > 0) {
        mem.used_kb = mem.total_kb - mem.available_kb;
    } else {
        mem.used_kb = mem.total_kb - mem.free_kb - mem.cached_kb;
    }

    return mem;
}
