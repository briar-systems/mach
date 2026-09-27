/* C half of the ymm vector transport controls (mach #3751). Compiled at -O0 and
 * -mavx2, where System V passes and returns a 32-byte vector in a ymm register. */
typedef int v8 __attribute__((vector_size(32)));
typedef int v16 __attribute__((vector_size(64)));

static long long w8(v8 x) {
    long long s = 0;
    for (int i = 0; i < 8; i++) s += (long long)x[i] * (i + 1);
    return s;
}

static long long w16(v16 x) {
    long long s = 0;
    for (int i = 0; i < 16; i++) s += (long long)x[i] * (i + 1);
    return s;
}

long long c_v8(long long a, v8 x, long long b) { return a * 100000 + w8(x) * 10 + b; }
long long c_two(v8 x, v8 y, long long next) { return w8(x) * 100000 + w8(y) * 10 + next; }
long long c_fmix(double x, v8 v, double y, long long next) {
    return (long long)(x * 2.0) * 100000 + w8(v) * 10 + next + (long long)(y * 2.0) * 10000000;
}
long long c_full(double a, double b, double c, double d, double e, double f, double g, double h, v8 x, long long next) {
    return (long long)(a + b + c + d + e + f + g + h) * 100000 + w8(x) * 10 + next;
}
long long c_v16(long long a, v16 x, long long b) { return a * 1000000 + w16(x) * 10 + b; }
v8 c_mk(int seed) {
    v8 r;
    for (int i = 0; i < 8; i++) r[i] = seed * (i + 1) - i;
    return r;
}

/* the reverse direction: C calls mach with the same shapes */
long long m_v8(long long a, v8 x, long long b);
long long m_two(v8 x, v8 y, long long next);
long long m_fmix(double x, v8 v, double y, long long next);
long long m_full(double a, double b, double c, double d, double e, double f, double g, double h, v8 x, long long next);
long long m_v16(long long a, v16 x, long long b);
v8 m_mk(int seed);

long long c_drives_mach(void) {
    v8 x, y;
    v16 z;
    for (int i = 0; i < 8; i++) x[i] = 3 * i + 1;
    for (int i = 0; i < 8; i++) y[i] = 40 - 5 * i;
    for (int i = 0; i < 16; i++) z[i] = 16 - i;
    if (m_v8(9, x, 3) != c_v8(9, x, 3)) return 1;
    if (m_two(x, y, 4) != c_two(x, y, 4)) return 2;
    if (m_fmix(1.5, x, 2.5, 6) != c_fmix(1.5, x, 2.5, 6)) return 3;
    if (m_full(1, 2, 3, 4, 5, 6, 7, 8, y, 7) != c_full(1, 2, 3, 4, 5, 6, 7, 8, y, 7)) return 4;
    if (m_v16(7, z, 8) != c_v16(7, z, 8)) return 5;
    const v8 r = m_mk(11);
    for (int i = 0; i < 8; i++) {
        if (r[i] != 11 * (i + 1) - i) return 6;
    }
    return 0;
}
