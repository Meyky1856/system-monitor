#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static CpuStat read_cpu_stat_line(const char *line);
static CpuStat *read_all_cpu_stats(int *count);

static CpuStat *prev_stats = NULL;
static int core_count = 0;

static CpuStat read_cpu_stat_line(const char *line) {
    CpuStat s = {0};
    // Parsing yang lebih aman
    if (sscanf(line, "%*s %ld %ld %ld %ld %ld %ld %ld %ld",
               &s.user, &s.nice, &s.system, &s.idle,
               &s.iowait, &s.irq, &s.softirq, &s.steal) < 4) {
        // Minimal harus ada user, nice, system, idle
        memset(&s, 0, sizeof(CpuStat));
    }
    return s;
}

static CpuStat *read_all_cpu_stats(int *count) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return NULL;

    char line[512];
    int max_cores = 256; // Tingkatkan limit core
    CpuStat *stats = calloc(max_cores + 1, sizeof(CpuStat));
    int n = 0;

    while (fgets(line, sizeof(line), fp) && n < max_cores) {
        if (strncmp(line, "cpu ", 4) == 0) {
            stats[n++] = read_cpu_stat_line(line);
        } else if (strncmp(line, "cpu", 3) == 0 && line[3] >= '0' && line[3] <= '9') {
            stats[n++] = read_cpu_stat_line(line);
        }
    }
    fclose(fp);

    *count = n;
    return stats;
}

void init_cpu_monitor(void) {
    if (prev_stats == NULL) {
        CpuStat *stats = read_all_cpu_stats(&core_count);
        prev_stats = stats ? stats : calloc(1, sizeof(CpuStat));
    }
}

CpuUsage get_cpu_usage(void) {
    CpuStat *curr = read_all_cpu_stats(&core_count);
    if (!curr) {
        CpuUsage u = {0};
        u.per_core = calloc(1, sizeof(double));
        return u;
    }

    CpuUsage usage = {0};
    // Core 0 biasanya adalah total (cpu), sisanya cpu0, cpu1, dst
    usage.core_count = (core_count > 1) ? core_count - 1 : 1;
    usage.per_core = calloc(usage.core_count, sizeof(double));

    for (int i = 0; i < core_count; i++) {
        CpuStat p = prev_stats[i];
        CpuStat c = curr[i];
        
        long prev_idle = p.idle + p.iowait;
        long idle = c.idle + c.iowait;
        long prev_non_idle = p.user + p.nice + p.system + p.irq + p.softirq + p.steal;
        long non_idle = c.user + c.nice + c.system + c.irq + c.softirq + c.steal;
        long prev_total = prev_idle + prev_non_idle;
        long total = idle + non_idle;

        long total_diff = total - prev_total;
        long idle_diff = idle - prev_idle;

        double cpu_percent = 0.0;
        if (total_diff > 0) {
            cpu_percent = (100.0 * (double)(total_diff - idle_diff)) / (double)total_diff;
        }

        if (i == 0) {
            usage.total = cpu_percent;
        } else if (i - 1 < usage.core_count) {
            usage.per_core[i - 1] = cpu_percent;
        }
    }

    // Ambil Load Average
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp) {
        fscanf(fp, "%lf %lf %lf", &usage.load_1min, &usage.load_5min, &usage.load_15min);
        fclose(fp);
    }

    free(prev_stats);
    prev_stats = curr;
    return usage;
}
