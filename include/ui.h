#ifndef UI_H
#define UI_H

#include "cpu.h"
#include "mem.h"
#include "disk.h"
#include "process.h"

#define SORT_CPU 0
#define SORT_MEM 1

void init_ui(void);
void draw_ui(CpuUsage *cpu, MemInfo *mem, DiskList *disks, ProcessList *pl, int selected);
void cleanup_ui(void);
int handle_input(ProcessList *pl, int selected, int *sort_mode);

#endif
