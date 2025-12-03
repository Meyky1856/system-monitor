#ifndef CPU_H
#define CPU_H

#include <stddef.h>

typedef struct {
    long user, nice, system, idle, iowait, irq, softirq, steal;
} CpuStat;

typedef struct {
    double total;
    double *per_core;
    int core_count;
    double load_1min, load_5min, load_15min;
} CpuUsage;

void init_cpu_monitor(void);
CpuUsage get_cpu_usage(void);

#endif
