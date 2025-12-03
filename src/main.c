#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "cpu.h"
#include "mem.h"
#include "disk.h"
#include "process.h"
#include "ui.h"

int main(void) {
    init_cpu_monitor();
    ProcessList pl;
    init_process_list(&pl);
    DiskList dl;
    init_disk_list(&dl);
    init_ui();

    int selected = 0;
    int sort_mode = SORT_CPU;
    int running = 1;

    // Untuk melacak PID yang sedang dipilih
    pid_t selected_pid = -1;

    while (running) {
        // 1. Ambil data baru
        CpuUsage cpu = get_cpu_usage();
        MemInfo mem = get_memory_info();
        update_disk_list(&dl);

        // 2. Simpan PID yang sedang dipilih sebelum update list
        if (pl.count > 0 && selected >= 0 && selected < pl.count) {
            selected_pid = pl.list[selected].pid;
        } else {
            selected_pid = -1;
        }

        // 3. Update dan Sort Process List
        update_process_list(&pl);
        if (sort_mode == SORT_CPU) {
            qsort(pl.list, pl.count, sizeof(Process), compare_process_by_cpu);
        } else {
            qsort(pl.list, pl.count, sizeof(Process), compare_process_by_mem);
        }

        // 4. Cari kembali posisi PID yang tadi dipilih
        // Ini mencegah kursor "lari" saat urutan proses berubah
        if (selected_pid != -1) {
            int found = 0;
            for (int i = 0; i < pl.count; i++) {
                if (pl.list[i].pid == selected_pid) {
                    selected = i;
                    found = 1;
                    break;
                }
            }
            // Jika PID mati/hilang, pertahankan index (dengan batas aman)
            if (!found) {
                if (selected >= pl.count) selected = pl.count - 1;
                if (selected < 0) selected = 0;
            }
        } else {
            if (selected >= pl.count) selected = pl.count - 1;
        }

        // 5. Gambar UI
        draw_ui(&cpu, &mem, &dl, &pl, selected);

        // 6. Handle Input
        int new_selected = handle_input(&pl, selected, &sort_mode);
        if (new_selected == -1) {
            running = 0;
        } else {
            selected = new_selected;
        }

        struct timespec ts = { .tv_sec = 0, .tv_nsec = 500000000L };
        nanosleep(&ts, NULL);

        if (cpu.per_core) free(cpu.per_core);
    }

    free_process_list(&pl);
    free_disk_list(&dl);
    cleanup_ui();
    return 0;
}
