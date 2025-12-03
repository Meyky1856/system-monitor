#include "ui.h"
#include "util.h"
#include <ncurses.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define HIST_SIZE 256 
static double cpu_hist[HIST_SIZE] = {0};
static double mem_hist[HIST_SIZE] = {0};

// Helper untuk menggambar border (Dipanggil SETELAH isi window digambar)
void draw_border_title(WINDOW *win, const char *title) {
    box(win, 0, 0); // Gambar border
    if (title) {
        wattron(win, A_BOLD);
        mvwprintw(win, 0, 2, " %s ", title);
        wattroff(win, A_BOLD);
    }
    wrefresh(win);
}

// Grafik menggunakan DOT (.) dan aman terhadap border
void draw_graph(WINDOW *win, double *data, int max_val) {
    int h = getmaxy(win) - 2; 
    int w = getmaxx(win) - 2; 
    
    if (h <= 0 || w <= 0) return;

    // Bersihkan area grafik (agar tidak ada sisa karakter sampah)
    for(int i=1; i<=h; i++) {
        mvwhline(win, i, 1, ' ', w);
    }

    for (int x = 0; x < w; x++) {
        // Ambil data history
        int idx = x; 
        if (idx >= HIST_SIZE) break;

        double val = data[idx];
        
        int bar_h = (int)((val / (double)max_val) * h);
        if (bar_h > h) bar_h = h;
        if (val > 1.0 && bar_h == 0) bar_h = 1; 

        for (int y = 0; y < h; y++) {
            int draw_y = h - y; 
            
            if (y < bar_h) {
                // HANYA MENGGUNAKAN TITIK (.) SESUAI REQUEST
                mvwaddch(win, draw_y, w - x, '.'); 
            }
        }
    }
}

// Bar usage [||||    ]
void draw_usage_bar(WINDOW *win, int y, int x, int width, double pct, int color_pair) {
    if (width < 2) return;
    int inner_width = width - 2;
    int filled = (int)((pct / 100.0) * inner_width);
    if (filled > inner_width) filled = inner_width;
    if (filled < 0) filled = 0;

    mvwaddch(win, y, x, '[');
    if (color_pair > 0) wattron(win, COLOR_PAIR(color_pair));
    for (int i = 0; i < inner_width; i++) {
        waddch(win, (i < filled) ? '|' : ' ');
    }
    if (color_pair > 0) wattroff(win, COLOR_PAIR(color_pair));
    mvwaddch(win, y, x + width - 1, ']');
}

void init_ui(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE); 
    keypad(stdscr, TRUE);  
    
    start_color();
    use_default_colors(); 

    init_pair(1, COLOR_CYAN, -1);    
    init_pair(2, COLOR_GREEN, -1);   
    init_pair(3, COLOR_YELLOW, -1);  
    init_pair(4, COLOR_WHITE, COLOR_BLUE); 
    init_pair(5, COLOR_RED, -1);           
}

void cleanup_ui(void) {
    endwin();
}

void draw_ui(CpuUsage *cpu, MemInfo *mem, DiskList *disks, ProcessList *pl, int selected) {
    int mx, my;
    getmaxyx(stdscr, my, mx);

    // Geser history grafik
    for (int i = HIST_SIZE - 1; i > 0; i--) {
        cpu_hist[i] = cpu_hist[i-1];
        mem_hist[i] = mem_hist[i-1];
    }
    cpu_hist[0] = cpu->total;
    mem_hist[0] = (double)mem->used_kb / mem->total_kb * 100.0;

    // Layout
    int left_w = mx / 2;
    int right_w = mx - left_w;
    int panel_h = my / 3; 

    WINDOW *win_g_cpu = newwin(panel_h, left_w, 0, 0);
    WINDOW *win_g_mem = newwin(panel_h, left_w, panel_h, 0);
    WINDOW *win_disk  = newwin(my - (panel_h * 2), left_w, panel_h * 2, 0);

    int right_top_h = panel_h; 
    int rt_w1 = right_w / 2; 
    int rt_w2 = right_w - rt_w1;
    
    WINDOW *win_t_mem = newwin(right_top_h, rt_w1, 0, left_w);
    WINDOW *win_t_cpu = newwin(right_top_h, rt_w2, 0, left_w + rt_w1);
    WINDOW *win_proc  = newwin(my - right_top_h, right_w, right_top_h, left_w);

    // 1. CPU GRAPH
    wattron(win_g_cpu, COLOR_PAIR(1));
    draw_graph(win_g_cpu, cpu_hist, 100);
    wattroff(win_g_cpu, COLOR_PAIR(1));
    mvwprintw(win_g_cpu, 1, 2, "Total: %.1f%%", cpu->total);
    draw_border_title(win_g_cpu, "CPU Usage"); // Gambar border terakhir!

    // 2. RAM GRAPH
    wattron(win_g_mem, COLOR_PAIR(2));
    draw_graph(win_g_mem, mem_hist, 100);
    wattroff(win_g_mem, COLOR_PAIR(2));
    draw_border_title(win_g_mem, "Memory Usage");

    // 3. DISK SPACE (Free di Kanan)
    int dy = 1;
    int max_d = (getmaxy(win_disk)-2)/3;
    int show_d = 0;
    int dw = getmaxx(win_disk);
    
    for(int i=0; i<disks->count && show_d < max_d; i++) {
        double total_gb = (double)disks->list[i].total_kb / (1024*1024);
        double used_gb = (double)disks->list[i].used_kb / (1024*1024);
        double free_gb = (double)disks->list[i].free_kb / (1024*1024);
        double pct = (used_gb / total_gb) * 100.0;
        
        // Baris 1: Mountpoint
        wattron(win_disk, A_BOLD);
        mvwprintw(win_disk, dy, 2, "%s", disks->list[i].mountpoint);
        wattroff(win_disk, A_BOLD);
        
        // Baris 2: Bar
        draw_usage_bar(win_disk, dy+1, 2, dw-4, pct, 3);
        
        // Baris 3: Used (Kiri) & Free (Kanan Mentok)
        mvwprintw(win_disk, dy+2, 2, "Used: %.1fG", used_gb);
        
        char free_str[32];
        snprintf(free_str, 32, "Free: %.1fG", free_gb);
        // Hitung posisi X agar pas di kanan (width - strlen - 2 padding)
        mvwprintw(win_disk, dy+2, dw - strlen(free_str) - 2, "%s", free_str);
        
        dy+=4;
        show_d++;
    }
    draw_border_title(win_disk, "Disk Space");

    // 4. MEMORY INFO
    char b_tot[32], b_use[32], b_free[32], b_cache[32];
    format_bytes(mem->total_kb*1024, b_tot, 32);
    format_bytes(mem->used_kb*1024, b_use, 32);
    format_bytes(mem->free_kb*1024, b_free, 32);
    format_bytes(mem->cached_kb*1024, b_cache, 32);

    wattron(win_t_mem, A_BOLD);
    mvwprintw(win_t_mem, 1, 2, "Total: %s", b_tot);
    mvwprintw(win_t_mem, 2, 2, "Used : %s", b_use);
    mvwprintw(win_t_mem, 3, 2, "Free : %s", b_free);
    mvwprintw(win_t_mem, 4, 2, "Cache: %s", b_cache);
    wattroff(win_t_mem, A_BOLD);
    draw_border_title(win_t_mem, "Memory Info");

    // 5. CPU CORES
    wattron(win_t_cpu, A_BOLD);
    int max_rows = getmaxy(win_t_cpu) - 2;
    int c_y = 1, c_x = 2;
    for (int i = 0; i < cpu->core_count; i++) {
        mvwprintw(win_t_cpu, c_y, c_x, "CPU%d: %3.0f%%", i, cpu->per_core[i]);
        c_y++;
        if (c_y > max_rows) { c_y = 1; c_x += 12; }
        if (c_x > getmaxx(win_t_cpu) - 8) break;
    }
    wattroff(win_t_cpu, A_BOLD);
    draw_border_title(win_t_cpu, "CPU Cores");

    // 6. PROCESS LIST
    wattron(win_proc, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win_proc, 1, 1, "%-6s %-10s %-6s %-6s %s", "PID", "USER", "CPU%", "MEM%", "CMD");
    wattroff(win_proc, COLOR_PAIR(4) | A_BOLD);
    
    int py = 2;
    int ph = getmaxy(win_proc) - 1;
    static int scroll_offset = 0;
    
    if (selected < scroll_offset) scroll_offset = selected;
    else if (selected >= scroll_offset + (ph - 2)) scroll_offset = selected - (ph - 3);
    if (scroll_offset > pl->count) scroll_offset = 0;

    for (int i = scroll_offset; i < pl->count && py < ph; i++) {
        Process *p = &pl->list[i];
        if (i == selected) wattron(win_proc, A_REVERSE | A_BOLD);
        
        double mem_pct = (double)p->mem_rss_kb / mem->total_kb * 100.0;
        int cmd_w = getmaxx(win_proc) - 34;
        if (cmd_w < 5) cmd_w = 5;

        char fmt[128];
        snprintf(fmt, sizeof(fmt), "%%-6d %%-10.10s %%-6.1f %%-6.1f %%-%d.%ds", cmd_w, cmd_w);
        mvwprintw(win_proc, py++, 1, fmt, p->pid, p->user, p->cpu_percent, mem_pct, p->name);
            
        if (i == selected) wattroff(win_proc, A_REVERSE | A_BOLD);
    }
    draw_border_title(win_proc, "Processes (Select & 'k' to Kill)");

    // Cleanup
    delwin(win_g_cpu); delwin(win_g_mem); delwin(win_disk);
    delwin(win_t_mem); delwin(win_t_cpu); delwin(win_proc);
}

int handle_input(ProcessList *pl, int selected, int *sort_mode) {
    int ch = getch();
    if (ch == ERR) return selected;
    switch(ch) {
        case 'q': return -1;
        case KEY_UP: if (selected > 0) selected--; break;
        case KEY_DOWN: if (selected < pl->count - 1) selected++; break;
        case 'c': 
            *sort_mode = SORT_CPU; 
            qsort(pl->list, pl->count, sizeof(Process), compare_process_by_cpu);
            break;
        case 'm': 
            *sort_mode = SORT_MEM; 
            qsort(pl->list, pl->count, sizeof(Process), compare_process_by_mem);
            break;
        case 'k':
            if (selected >= 0 && selected < pl->count) {
                kill(pl->list[selected].pid, SIGKILL);
                beep();
            }
            break;
    }
    if (selected >= pl->count && pl->count > 0) selected = pl->count - 1;
    return selected;
}
