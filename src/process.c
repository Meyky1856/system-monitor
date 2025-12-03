#include "process.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>

static long ticks_per_sec = 100;

void init_process_list(ProcessList *pl) {
    pl->capacity = 1024;
    pl->list = calloc(pl->capacity, sizeof(Process));
    pl->count = 0;
    ticks_per_sec = get_ticks_per_second();
}

static int read_process_stat(pid_t pid, Process *p) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", (int)pid);

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[2048]; // Buffer besar
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // Parse nama process
    char *start = strchr(line, '(');
    char *end = strrchr(line, ')');
    if (!start || !end) return -1;

    *end = '\0';
    snprintf(p->name, sizeof(p->name), "%s", start + 1);

    char *rest = end + 2;
    char state;
    long dummy_long;
    unsigned long dummy_ulong;
    long utime, stime;
    // Variabel num_threads dan rss dihapus karena tidak dipakai di sini

    // Skip fields 3-13
    sscanf(rest, "%c %ld %ld %ld %ld %ld %lu %lu %lu %lu %lu %ld %ld",
           &state, &dummy_long, &dummy_long, &dummy_long, &dummy_long, &dummy_long,
           &dummy_ulong, &dummy_ulong, &dummy_ulong, &dummy_ulong, &dummy_ulong,
           &utime, &stime);
    
    p->pid = pid;
    p->state = state;
    p->utime = utime;
    p->stime = stime;
    
    // Baca status untuk RSS dan UID
    snprintf(path, sizeof(path), "/proc/%d/status", (int)pid);
    fp = fopen(path, "r");
    if (fp) {
        char buf[256];
        while(fgets(buf, sizeof(buf), fp)) {
            if (strncmp(buf, "Uid:", 4) == 0) {
                unsigned int uid;
                sscanf(buf, "Uid:\t%u", &uid);
                get_username(uid, p->user, sizeof(p->user));
            } else if (strncmp(buf, "VmRSS:", 6) == 0) {
                long rss_kb;
                sscanf(buf, "VmRSS:\t%ld", &rss_kb);
                p->mem_rss_kb = rss_kb;
            }
        }
        fclose(fp);
    }

    // Baca Cmdline
    snprintf(path, sizeof(path), "/proc/%d/cmdline", (int)pid);
    fp = fopen(path, "r");
    if (fp) {
        size_t len = fread(line, 1, sizeof(line)-1, fp);
        if (len > 0) {
            line[len] = 0;
            // Ganti null separator dengan spasi
            for(size_t i=0; i<len; i++) if(line[i]=='\0') line[i]=' ';
            
            // PERBAIKAN: Gunakan %.255s untuk membatasi copy agar tidak warning truncate
            snprintf(p->cmd, sizeof(p->cmd), "%.255s", line);
        } else {
            snprintf(p->cmd, sizeof(p->cmd), "[%s]", p->name);
        }
        fclose(fp);
    }
    
    return 0;
}

void update_process_list(ProcessList *pl) {
    DIR *dir = opendir("/proc");
    if (!dir) return;

    // Simpan data lama untuk hitung CPU delta
    Process *old_list = pl->list;
    int old_count = pl->count;
    
    Process *new_list = calloc(pl->capacity, sizeof(Process));
    int new_count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && new_count < pl->capacity) {
        if (entry->d_type != DT_DIR || entry->d_name[0] < '0' || entry->d_name[0] > '9') continue;
        
        pid_t pid = atoi(entry->d_name);
        Process p = {0};
        
        if (read_process_stat(pid, &p) != 0) continue;

        // Cari process lama untuk hitung CPU
        for (int i=0; i<old_count; i++) {
            if (old_list[i].pid == pid) {
                long delta_time = (p.utime + p.stime) - (old_list[i].last_utime + old_list[i].last_stime);
                p.cpu_percent = (double)delta_time / ticks_per_sec / 0.5 * 100.0; // 0.5 refresh rate
                if (p.cpu_percent > 100.0) p.cpu_percent = 100.0;
                break;
            }
        }
        p.last_utime = p.utime;
        p.last_stime = p.stime;
        
        new_list[new_count++] = p;
    }
    closedir(dir);

    free(old_list);
    pl->list = new_list;
    pl->count = new_count;
}

int compare_process_by_cpu(const void *a, const void *b) {
    double diff = ((Process*)b)->cpu_percent - ((Process*)a)->cpu_percent;
    return (diff > 0) - (diff < 0);
}

int compare_process_by_mem(const void *a, const void *b) {
    return ((Process*)b)->mem_rss_kb - ((Process*)a)->mem_rss_kb;
}

void free_process_list(ProcessList *pl) {
    free(pl->list);
    pl->list = NULL;
}
