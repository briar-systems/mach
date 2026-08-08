#include <stdarg.h>
#include <stddef.h>

extern unsigned long long *__local_stdio_printf_options(void);
extern int __stdio_common_vsprintf(unsigned long long options,
                                   char *buffer,
                                   size_t buffer_count,
                                   const char *format,
                                   void *locale,
                                   va_list args);

int vsnprintf(char *buffer, size_t buffer_count, const char *format, va_list args) {
    return __stdio_common_vsprintf(*__local_stdio_printf_options(),
                                   buffer, buffer_count, format, 0, args);
}
