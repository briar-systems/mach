/* C half of the wide vector transport controls (mach #3589). Compiled at -O0 so
 * every read really happens. */
#ifdef _WIN32
int _fltused = 0;
#endif

/* the carriers. Win64 declares a vector of any extent but 2, 4, 8 and 16 bytes
 * as a byte-array record (doc/language/ext-fun.md, "Windows vector carriers"),
 * which the convention passes as a pointer to a caller copy and returns through
 * caller-provided storage; an int array of the same extent is placed the same
 * way. Every other convention takes C's own vector type. */
#ifdef _WIN32
typedef struct { int v[8]; } v8;
typedef struct { int v[16]; } v16;
#define LANE(x, i) ((x).v[i])
#else
typedef int v8 __attribute__((vector_size(32)));
typedef int v16 __attribute__((vector_size(64)));
#define LANE(x, i) ((x)[i])
#endif

/* the result carrier. System V makes a vector wider than the vector registers
 * MEMORY class, returned through the hidden result pointer, as gcc does; clang
 * returns it in xmm0:xmm1 instead, so under clang on System V the record of the
 * same extent, which clang returns as MEMORY, stands for it */
#if defined(__x86_64__) && defined(__clang__) && !defined(_WIN32)
typedef struct { int v[8]; } r8;
#define RLANE(x, i) ((x).v[i])
#else
typedef v8 r8;
#define RLANE(x, i) LANE(x, i)
#endif

static long long w8(v8 x) {
    long long s = 0;
    for (int i = 0; i < 8; i++) s += (long long)LANE(x, i) * (i + 1);
    return s;
}

static long long w16(v16 x) {
    long long s = 0;
    for (int i = 0; i < 16; i++) s += (long long)LANE(x, i) * (i + 1);
    return s;
}

long long c_v8(long long a, v8 x, long long b) { return a * 100000 + w8(x) * 10 + b; }
long long c_six(long long a, long long b, long long c, long long d, long long e, long long f, v8 x, long long next) {
    return (a + b + c + d + e + f) * 100000 + w8(x) * 10 + next;
}
long long c_seven(long long a, long long b, long long c, long long d, long long e, long long f, long long g, v8 x, long long next) {
    return (a + b + c + d + e + f + g) * 100000 + w8(x) * 10 + next;
}
long long c_fmix(double x, v8 v, double y, long long next) {
    return (long long)(x * 2.0) * 100000 + w8(v) * 10 + next + (long long)(y * 2.0) * 10000000;
}
long long c_v16(long long a, v16 x, long long b) { return a * 1000000 + w16(x) * 10 + b; }
r8 c_mk(int seed) {
    r8 r;
    for (int i = 0; i < 8; i++) RLANE(r, i) = seed * (i + 1) - i;
    return r;
}

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same shapes */
long long m_v8(long long a, v8 x, long long b) MACH_SYM(m_v8);
long long m_six(long long a, long long b, long long c, long long d, long long e, long long f, v8 x, long long next) MACH_SYM(m_six);
long long m_seven(long long a, long long b, long long c, long long d, long long e, long long f, long long g, v8 x, long long next) MACH_SYM(m_seven);
long long m_fmix(double x, v8 v, double y, long long next) MACH_SYM(m_fmix);
long long m_v16(long long a, v16 x, long long b) MACH_SYM(m_v16);
r8 m_mk(int seed) MACH_SYM(m_mk);

long long c_drives_mach(void) {
    v8 x;
    v16 y;
    for (int i = 0; i < 8; i++) LANE(x, i) = 3 * i + 1;
    for (int i = 0; i < 16; i++) LANE(y, i) = 16 - i;
    if (m_v8(9, x, 3) != c_v8(9, x, 3)) return 1;
    if (m_six(1, 2, 3, 4, 5, 6, x, 4) != c_six(1, 2, 3, 4, 5, 6, x, 4)) return 2;
    if (m_seven(1, 2, 3, 4, 5, 6, 7, x, 5) != c_seven(1, 2, 3, 4, 5, 6, 7, x, 5)) return 3;
    if (m_fmix(1.5, x, 2.5, 6) != c_fmix(1.5, x, 2.5, 6)) return 4;
    if (m_v16(7, y, 8) != c_v16(7, y, 8)) return 5;
    const r8 r = m_mk(11);
    for (int i = 0; i < 8; i++) {
        if (RLANE(r, i) != 11 * (i + 1) - i) return 6;
    }
    return 0;
}
