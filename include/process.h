#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include <stddef.h>

typedef struct {
    pid_t pid;
    char name[64];
    char cmd[256];
    char user[32];
    int threads;
    size_t mem_rss_kb;
    double cpu_percent;
    char state;
    long utime, stime;
    long last_utime, last_stime;
} Process;

typedef struct {
    Process *list;
    int count;
    int capacity;
} ProcessList;

void init_process_list(ProcessList *pl);
void update_process_list(ProcessList *pl);
void free_process_list(ProcessList *pl);
int compare_process_by_cpu(const void *a, const void *b);
int compare_process_by_mem(const void *a, const void *b);

#endif
