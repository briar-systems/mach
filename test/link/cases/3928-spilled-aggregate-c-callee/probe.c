/* C half of the spilled aggregate controls (mach #3928). Compiled at -O0 so every
 * read really happens. */
#ifdef _WIN32
int _fltused = 0;
#endif

struct s12 { long long a; int b; };
struct h3 { double a, b, c; };
struct f2 { float a, b; };

long long c_s(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct s12 s, long long n) {
    return a + b + c + d + e + f + g + s.a * 100 + s.b * 10000 + n * 1000000;
}
long long c_sd(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct s12 s, double x, long long n) {
    return a + b + c + d + e + f + g + s.a * 100 + s.b * 10000 + (long long)(x * 2.0) * 100000 + n * 1000000;
}
long long c_h(double a, double b, double c, double d, double e, double f, struct h3 h, double n) {
    return (long long)(a + b + c + d + e + f + h.a * 100 + h.b * 1000 + h.c * 10000 + n * 100000);
}
long long c_hg(double a, double b, double c, double d, double e, double f, struct h3 h, struct f2 g, double n) {
    return (long long)(a + b + c + d + e + f + h.a * 100 + h.b * 1000 + h.c * 10000 + g.a * 100000 + g.b * 1000000 + n * 10000000);
}

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same shapes */
long long m_s(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct s12 s, long long n) MACH_SYM(m_s);
long long m_sd(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct s12 s, double x, long long n) MACH_SYM(m_sd);
long long m_h(double a, double b, double c, double d, double e, double f, struct h3 h, double n) MACH_SYM(m_h);
long long m_hg(double a, double b, double c, double d, double e, double f, struct h3 h, struct f2 g, double n) MACH_SYM(m_hg);

long long c_drives_mach(void) {
    struct s12 s = {3, 5};
    struct h3 h = {2.0, 3.0, 4.0};
    struct f2 g = {5.0f, 6.0f};
    if (m_s(1, 1, 1, 1, 1, 1, 1, s, 7) != 7050307) return 1;
    if (m_sd(1, 1, 1, 1, 1, 1, 1, s, 4.5, 7) != 7950307) return 2;
    if (m_h(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, h, 7.0) != 743206) return 3;
    if (m_hg(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, h, g, 7.0) != 76543206) return 4;
    return 0;
}
