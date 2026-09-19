/* C half of the 128-bit integer transport controls (mach #3511). Compiled at -O0 so
 * every read really happens. */
#ifdef _WIN32
int _fltused = 0;
#endif

typedef unsigned __int128 u128;
typedef __int128 i128;

static long long halves(u128 v, long long next) {
    return (long long)(v >> 64) * 100 + (long long)v + next * 1000;
}

long long c_u128(u128 v, long long next) { return halves(v, next); }
long long c_i128(i128 v, long long next) { return (long long)(v >> 64) * 100 + (long long)v + next * 1000; }
long long c_odd(long long first, u128 v, long long next) { return first * 10000 + halves(v, next); }
long long c_six(long long a, long long b, long long c, long long d, long long e, long long f, u128 v, long long next) {
    return (a + b + c + d + e + f) * 10000 + halves(v, next);
}
long long c_seven(long long a, long long b, long long c, long long d, long long e, long long f, long long g, u128 v, long long next) {
    return (a + b + c + d + e + f + g) * 10000 + halves(v, next);
}
long long c_fmix(double x, u128 v, double y, long long next) {
    return (long long)(x * 2.0) * 10000 + halves(v, next) + (long long)(y * 2.0) * 100000;
}
u128 c_mk_u128(long long lo, long long hi) { return ((u128)(unsigned long long)hi << 64) | (u128)(unsigned long long)lo; }
i128 c_mk_i128(long long x) { return (i128)x * 3; }

/* the reverse direction: C calls mach with the same shapes */
long long m_u128(u128 v, long long next);
long long m_i128(i128 v, long long next);
long long m_odd(long long first, u128 v, long long next);
long long m_six(long long a, long long b, long long c, long long d, long long e, long long f, u128 v, long long next);
long long m_seven(long long a, long long b, long long c, long long d, long long e, long long f, long long g, u128 v, long long next);
long long m_fmix(double x, u128 v, double y, long long next);
u128 m_mk_u128(long long lo, long long hi);
i128 m_mk_i128(long long x);

long long c_drives_mach(void) {
    const u128 v = ((u128)3 << 64) | 7;
    if (m_u128(v, 1) != 1307) return 1;
    if (m_i128((i128)-5, 2) != 1895) return 2;
    if (m_odd(9, v, 3) != 93307) return 3;
    if (m_six(1, 2, 3, 4, 5, 6, v, 4) != 214307) return 4;
    if (m_seven(1, 2, 3, 4, 5, 6, 7, v, 5) != 285307) return 5;
    if (m_fmix(1.5, v, 2.5, 6) != 536307) return 6;
    const u128 ru = m_mk_u128(11, 13);
    if ((long long)(ru >> 64) != 13 || (long long)ru != 11) return 7;
    const i128 ri = m_mk_i128(-17);
    if (ri != (i128)-51) return 8;
    return 0;
}
