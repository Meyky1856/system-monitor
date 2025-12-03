# Nama Output Program
TARGET = sysmon

# Compiler dan Flags
CC = gcc
# -Iinclude memberitahu gcc untuk mencari file header (.h) di folder 'include/'
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -Iinclude
LDFLAGS = -lncurses -lm

# Direktori
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Mencari semua file .c di dalam folder src/
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Mengubah daftar file .c menjadi .o di dalam folder obj/
# Contoh: src/main.c -> obj/main.o
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- Rules ---

# Rule Utama (Default)
all: create_dir $(TARGET)

# Link semua object file menjadi executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build berhasil! Jalankan dengan: ./$(TARGET)"

# Compile setiap file .c menjadi .o
# $< adalah input file (.c), $@ adalah output file (.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Buat folder obj jika belum ada
create_dir:
	@mkdir -p $(OBJ_DIR)

# Bersihkan file hasil compile
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
	@rmdir $(OBJ_DIR) 2>/dev/null || true

.PHONY: all clean create_dir
