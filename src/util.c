#include "util.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>

char *format_bytes(size_t bytes, char *buf, size_t buflen) {
    const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    double val = bytes;
    int i = 0;
    while (val >= 1024.0 && i < 4) {
        val /= 1024.0;
        i++;
    }
    // Format: "15.3GB" (Tanpa spasi sebelum unit, 1 desimal)
    snprintf(buf, buflen, "%.1f%s", val, suffixes[i]);
    return buf;
}

long parse_key_value_line(const char *line, const char *key) {
    size_t keylen = strlen(key);
    if (strncmp(line, key, keylen) == 0 && line[keylen] == ':') {
        const char *val_start = line + keylen + 1;
        while (*val_start == ' ') val_start++;
        return atol(val_start);
    }
    return -1;
}

int get_ticks_per_second(void) {
    return sysconf(_SC_CLK_TCK);
}

char *get_username(uid_t uid, char *buf, size_t buflen) {
    struct passwd *pw = getpwuid(uid);
    if (pw) snprintf(buf, buflen, "%s", pw->pw_name);
    else snprintf(buf, buflen, "%u", uid);
    return buf;
}
