/* C half of the naturally aligned record controls (mach #3929). Compiled at -O0 so
 * every read really happens. */
#ifdef _WIN32
int _fltused = 0;
#endif

struct w { unsigned __int128 v; };
struct al { long long a, b; } __attribute__((aligned(16)));
struct na { struct al v; };

static long long halves(unsigned __int128 v) { return (long long)(v >> 64) * 100 + (long long)v; }

long long c_w(long long first, struct w v, long long next) { return first * 10000 + halves(v.v) + next * 1000000; }
long long c_n(long long first, struct na v, long long next) { return first * 10000 + v.v.b * 100 + v.v.a + next * 1000000; }
long long c_a(long long first, struct al v, long long next) { return first * 10000 + v.b * 100 + v.a + next * 1000000; }
long long c_as(long long a, long long b, long long c, long long d, long long e, long long f, long long g, long long h,
    signed char x, struct al v, long long next) {
    return (a + b + c + d + e + f + g + h) * 10000 + x * 100000000 + v.b * 100 + v.a + next * 1000000;
}

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same shapes */
long long m_w(long long first, struct w v, long long next) MACH_SYM(m_w);
long long m_n(long long first, struct na v, long long next) MACH_SYM(m_n);
long long m_a(long long first, struct al v, long long next) MACH_SYM(m_a);
long long m_as(long long a, long long b, long long c, long long d, long long e, long long f, long long g, long long h,
    signed char x, struct al v, long long next) MACH_SYM(m_as);

long long c_drives_mach(void) {
    struct w w = {((unsigned __int128)3 << 64) | 7};
    struct al a = {7, 3};
    struct na n = {{7, 3}};
    if (m_w(9, w, 5) != 5090307) return 1;
    if (m_n(9, n, 5) != 5090307) return 2;
    if (m_a(9, a, 5) != 5090307) return 3;
    if (m_as(1, 1, 1, 1, 1, 1, 1, 1, 2, a, 5) != 205080307) return 4;
    return 0;
}
