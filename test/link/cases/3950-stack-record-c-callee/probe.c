/* C half of the stack-passed record controls (mach #3950). Compiled at -O0 so
 * every read really happens. */
struct r3 { unsigned char a, b, c; };
struct m { unsigned char a; unsigned int b; };
struct __attribute__((packed)) p { unsigned __int128 v; };
struct h { float a, b, c; };

#define WORDS long long a, long long b, long long c, long long d, long long e, long long f, long long g, long long h
#define WSUM ((a + b + c + d + e + f + g + h) * 10000 + x * 100000000 + next * 1000000)

long long c_r3(WORDS, signed char x, struct r3 v, long long next) { return WSUM + v.a + v.b * 10 + v.c * 100; }
long long c_m(WORDS, signed char x, struct m v, long long next) { return WSUM + v.a + v.b * 10; }
long long c_p(WORDS, signed char x, struct p v, long long next) { return WSUM + (long long)(v.v >> 64) * 100 + (long long)v.v; }
long long c_h(double a, double b, double c, double d, double e, double f, double g, double h,
    float x, struct h v, float next) {
    return (long long)((a + b + c + d + e + f + g + h) * 10000 + x * 100000000.0 + next * 1000000.0
        + v.a + v.b * 10 + v.c * 100);
}

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same shapes */
long long m_r3(WORDS, signed char x, struct r3 v, long long next) MACH_SYM(m_r3);
long long m_m(WORDS, signed char x, struct m v, long long next) MACH_SYM(m_m);
long long m_p(WORDS, signed char x, struct p v, long long next) MACH_SYM(m_p);
double m_h(double a, double b, double c, double d, double e, double f, double g, double h,
    float x, struct h v, float next) MACH_SYM(m_h);

long long c_drives_mach(void) {
    struct r3 r = {1, 2, 3};
    struct m mm = {7, 3};
    struct p pp = {((unsigned __int128)3 << 64) | 7};
    struct h hh = {1.0f, 2.0f, 3.0f};
    if (m_r3(1, 1, 1, 1, 1, 1, 1, 1, 2, r, 5) != 205080321) return 1;
    if (m_m(1, 1, 1, 1, 1, 1, 1, 1, 2, mm, 5) != 205080037) return 2;
    if (m_p(1, 1, 1, 1, 1, 1, 1, 1, 2, pp, 5) != 205080307) return 3;
    if (m_h(1, 1, 1, 1, 1, 1, 1, 1, 2.0f, hh, 5.0f) != 205080321.0) return 4;
    return 0;
}
