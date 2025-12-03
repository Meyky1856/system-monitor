#ifndef DISK_H
#define DISK_H

#include <stddef.h>

typedef struct {
    char device[256];      // Diubah dari 64 ke 256 agar sesuai dengan buffer pembaca
    char mountpoint[256];
    size_t total_kb;
    size_t used_kb;
    size_t free_kb;
} DiskInfo;

typedef struct {
    DiskInfo *list;
    int count;
    int capacity;
} DiskList;

void init_disk_list(DiskList *dl);
void update_disk_list(DiskList *dl);
void free_disk_list(DiskList *dl);

#endif
