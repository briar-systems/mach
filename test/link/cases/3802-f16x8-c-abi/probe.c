/* C half of the f16 lane transport controls (mach #3802). Compiled at -O0,
 * where AAPCS64 passes and returns a short vector in a v register. */
#include <arm_neon.h>
#include <string.h>

static long long w8(float16x8_t x) {
    unsigned short b[8];
    memcpy(b, &x, sizeof b);
    long long s = 0;
    for (int i = 0; i < 8; i++) s += (long long)b[i] * (i + 1);
    return s;
}

static long long w4(float16x4_t x) {
    unsigned short b[4];
    memcpy(b, &x, sizeof b);
    long long s = 0;
    for (int i = 0; i < 4; i++) s += (long long)b[i] * (i + 1);
    return s;
}

static long long hbits(_Float16 h) {
    unsigned short b;
    memcpy(&b, &h, sizeof b);
    return b;
}

long long c_v8(long long a, float16x8_t x, long long b) { return a * 100000000 + w8(x) * 100 + b; }
long long c_v4(long long a, float16x4_t x, long long b) { return a * 1000000000 + w4(x) * 1000 + b; }
long long c_mix(float f, float16x8_t x, double d, _Float16 h, long long next) {
    return hbits(h) * 10000000000LL + (long long)(f * 2.0f) * 1000000000 + w8(x) * 10 + next + (long long)(d * 2.0) * 100;
}
long long c_full(double a, double b, double c, double d, double e, double f, double g, double h, float16x8_t x, long long next) {
    return (long long)(a + b + c + d + e + f + g + h) * 100000000 + w8(x) * 1000 + next;
}
float16x8_t c_mk(int seed) {
    unsigned short b[8];
    for (int i = 0; i < 8; i++) b[i] = (unsigned short)(seed * (i + 1) - i);
    float16x8_t r;
    memcpy(&r, b, sizeof r);
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
long long m_v8(long long a, float16x8_t x, long long b) MACH_SYM(m_v8);
long long m_v4(long long a, float16x4_t x, long long b) MACH_SYM(m_v4);
long long m_mix(float f, float16x8_t x, double d, _Float16 h, long long next) MACH_SYM(m_mix);
long long m_full(double a, double b, double c, double d, double e, double f, double g, double h, float16x8_t x, long long next) MACH_SYM(m_full);
float16x8_t m_mk(int seed) MACH_SYM(m_mk);

long long c_drives_mach(void) {
    unsigned short bx[8], by[8], bq[4];
    for (int i = 0; i < 8; i++) bx[i] = (unsigned short)(0x3C00 + 3 * i);
    for (int i = 0; i < 8; i++) by[i] = (unsigned short)(0xC000 - 5 * i);
    for (int i = 0; i < 4; i++) bq[i] = (unsigned short)(0x7BFF - i);
    float16x8_t x, y;
    float16x4_t q;
    memcpy(&x, bx, sizeof x);
    memcpy(&y, by, sizeof y);
    memcpy(&q, bq, sizeof q);
    unsigned short hb = 0x3555;
    _Float16 h;
    memcpy(&h, &hb, sizeof h);
    if (m_v8(9, x, 3) != c_v8(9, x, 3)) return 1;
    if (m_v4(9, q, 4) != c_v4(9, q, 4)) return 2;
    if (m_mix(1.5f, y, 2.5, h, 6) != c_mix(1.5f, y, 2.5, h, 6)) return 3;
    if (m_full(1, 2, 3, 4, 5, 6, 7, 8, y, 7) != c_full(1, 2, 3, 4, 5, 6, 7, 8, y, 7)) return 4;
    const float16x8_t r = m_mk(11);
    unsigned short br[8];
    memcpy(br, &r, sizeof br);
    for (int i = 0; i < 8; i++) {
        if (br[i] != (unsigned short)(11 * (i + 1) - i)) return 5;
    }
    return 0;
}
