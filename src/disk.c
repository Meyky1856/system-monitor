#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>

static void read_mounts(DiskList *dl) {
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) return;

    char line[512];
    while (fgets(line, sizeof(line), fp) && dl->count < dl->capacity) {
        char device[256], mountpoint[256], fstype[64];
        if (sscanf(line, "%255s %255s %63s", device, mountpoint, fstype) != 3) continue;

        // 1. Filter berdasarkan Tipe Filesystem
        if (strstr(fstype, "tmpfs") || strstr(fstype, "devtmpfs") ||
            strstr(fstype, "proc") || strstr(fstype, "sysfs") ||
            strstr(fstype, "cgroup") || strstr(fstype, "squashfs") ||
            strstr(fstype, "overlay") || strstr(fstype, "tracefs") ||
            strstr(fstype, "debugfs") || strstr(fstype, "fuse") || 
            strstr(fstype, "efivarfs")) continue;

        // 2. Filter berdasarkan Mountpoint (Untuk membuang /sys/firmware dll)
        if (strncmp(mountpoint, "/sys", 4) == 0 || 
            strncmp(mountpoint, "/proc", 5) == 0 ||
            strncmp(mountpoint, "/run", 4) == 0 ||
            strncmp(mountpoint, "/dev", 4) == 0 ||
            strncmp(mountpoint, "/snap", 5) == 0 ||
            strstr(mountpoint, "/docker") != NULL) continue;

        // 3. Filter Device Loop (kecuali Anda ingin melihat snap/flatpak)
        if (strncmp(device, "/dev/loop", 9) == 0) continue;

        struct statfs fs;
        if (statfs(mountpoint, &fs) != 0) continue;

        if (fs.f_blocks == 0) continue; // Skip partisi 0 size

        size_t total = (fs.f_blocks * fs.f_bsize) / 1024;
        size_t free_sp = (fs.f_bfree * fs.f_bsize) / 1024;
        
        DiskInfo d = {0};
        snprintf(d.device, sizeof(d.device), "%s", device);
        snprintf(d.mountpoint, sizeof(d.mountpoint), "%s", mountpoint);
        d.total_kb = total;
        d.free_kb = free_sp;
        d.used_kb = total - free_sp;

        dl->list[dl->count++] = d;
    }
    fclose(fp);
}

void init_disk_list(DiskList *dl) {
    dl->capacity = 32;
    dl->list = calloc(dl->capacity, sizeof(DiskInfo));
    dl->count = 0;
    read_mounts(dl);
}

void update_disk_list(DiskList *dl) {
    dl->count = 0;
    read_mounts(dl);
}

void free_disk_list(DiskList *dl) {
    free(dl->list);
    dl->list = NULL;
}
