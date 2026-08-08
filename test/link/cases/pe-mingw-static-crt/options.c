static unsigned long long options;

unsigned long long *__local_stdio_printf_options(void) {
    return &options;
}
