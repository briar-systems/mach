#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

static int format(char *buffer, size_t len, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    int result = vsnprintf(buffer, len, pattern, args);
    va_end(args);
    return result;
}

int call_format(void) {
    char buffer[32];
    return format(buffer, sizeof(buffer), "%s:%d", "crt", 32);
}
