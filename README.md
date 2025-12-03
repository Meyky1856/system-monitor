# Linux System Monitor

**Aplikasi System Monitor** ini adalah aplikasi pemantau sistem berbasis terminal (TUI) yang ringan dan ditulis menggunakan bahasa C (C11). Aplikasi ini membaca data langsung dari kernel Linux melalui sistem file `/proc` dan menampilkannya menggunakan library `ncurses` dengan antarmuka yang bersih dan responsif.

## 🚀 Fitur

* **Real-time Monitoring**: Pemantauan CPU dan Memori dengan grafik riwayat (history graph).
* **CPU Core Stats**: Menampilkan penggunaan per-core CPU secara detail.
* **Memory Info**: Menampilkan Total, Used, Free, dan Cache memory.
* **Disk Usage**: Memantau penggunaan disk (Used/Free GB) dengan visual bar progress.
* **Process Manager**:
    * Daftar proses aktif dengan penggunaan CPU% dan MEM%.
    * Sorting berdasarkan CPU atau Memori.
    * Seleksi proses (Arrow Keys) yang stabil (tracking PID).
    * **Kill Process** (SIGKILL) langsung dari aplikasi.
* **UI Estetik**: Menggunakan layout modern, grafik *dot-matrix*, dan skema warna terminal default (transparan).

## 📂 Struktur Proyek

Proyek ini menggunakan struktur folder yang rapi:

```text
system-monitor/
├── Makefile        # Sistem build otomatis
├── README.md       # Dokumentasi ini
├── include/        # Header files (.h)
│   ├── cpu.h
│   ├── disk.h
│   ├── mem.h
│   ├── process.h
│   ├── ui.h
│   └── util.h
└── src/            # Source code (.c)
    ├── main.c
    ├── cpu.c
    ├── disk.c
    ├── mem.c
    ├── process.c
    ├── ui.c
    └── util.c
```
## 🛠️ Prasyarat Prerequisites)

Aplikasi ini hanya berjalan di Linux karena ketergantungan pada /proc. Anda memerlukan compiler GCC, Make, dan library Ncurses.

### Arch Linux / Manjaro
```
sudo pacman -S base-devel ncurses
```
### Ubuntu / Debian / Kali Linux
```
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```
### Fedora / RHEL
```
sudo dnf install ncurses-devel
```
## 📦 Cara Install & Compile

* Extract atau Clone source code ke dalam folder.
  ```
  git clone https://github.com/Meyky1856/system-monitor.git
  ```
* Masuk ke folder dan Compile program menggunakan Make:
  ```
  cd system-monitor
  make
  ```

  Perintah ini akan otomatis membuat folder obj/, mengompilasi file dari src/, dan menghasilkan file executable bernama sysmon.

* Jalankan aplikasi:
  ```
  ./sysmon
  ```
* Membersihkan file build (jika diperlukan):
  ```
  make clean
  ```

## 🎮 Kontrol (Key Bindings)

Berikut adalah tombol navigasi yang tersedia saat aplikasi berjalan:
Tombol	Fungsi
* **↑** / **↓**	Navigasi naik/turun pada daftar proses
* **c**	Urutkan proses berdasarkan CPU Usage
* **m**	Urutkan proses berdasarkan Memory Usage
* **k**	Kill (hentikan paksa/SIGKILL) proses yang dipilih
* **q**	Keluar dari aplikasi
