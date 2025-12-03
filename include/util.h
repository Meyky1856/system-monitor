#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <sys/types.h>

char *format_bytes(size_t bytes, char *buf, size_t buflen);
long parse_key_value_line(const char *line, const char *key);
int get_ticks_per_second(void);
char *get_username(uid_t uid, char *buf, size_t buflen);
#endif
