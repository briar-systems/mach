/* C half of the f16 transport controls (mach #3800). Compiled at -O0 so every
 * read really happens. No _Float16 arithmetic happens here, since that would
 * call a runtime helper mach does not link: every value is read and built as
 * its bits. */
#ifdef _WIN32
int _fltused = 0;
#endif

typedef unsigned short u16;

/* `h` is the record member, C's `_Float16` everywhere. `hs` is the scalar
 * argument and result. mach's windows target is the Microsoft x64 convention,
 * whose only compiler with a half type is clang: `_Float16` in the xmm register
 * of its positional slot, returned in xmm0. mingw-w64 GCC passes it in the
 * integer register instead (doc/language/ext-fun.md), so under GCC on windows a
 * `float` whose low 16 bits are the f16 stands for it, which GCC places as
 * clang places the `_Float16` */
typedef _Float16 h;
#if defined(_WIN32) && defined(__GNUC__) && !defined(__clang__)
#define HALF_BY_FLOAT 1
#endif
#ifdef HALF_BY_FLOAT
typedef float hs;
#else
typedef _Float16 hs;
#endif

typedef struct { h a, b; } H2;
typedef struct { h a; int b; } HI;
typedef struct { h a; float b; } HF;
typedef struct { h v[4]; } H4;
typedef struct { h v[5]; } H5;

static long long hbits(h x) {
    union { h f; u16 u; } c;
    c.f = x;
    return c.u;
}

static h mkh(u16 u) {
    union { h f; u16 u; } c;
    c.u = u;
    return c.f;
}

#ifdef HALF_BY_FLOAT
static long long bits(hs x) {
    union { float f; unsigned u; } c;
    c.f = x;
    return c.u & 0xFFFF;
}

static hs mk(u16 u) {
    union { float f; unsigned u; } c;
    c.u = u;
    return c.f;
}

static h tomem(hs x) { return mkh((u16)bits(x)); }
#else
#define bits hbits
#define mk mkh
#define tomem(x) (x)
#endif

/* each result weighs every argument by its position, so a value read from the
 * wrong register or slot changes it */
long long c_scalars(int a, hs x, double d, hs y, float f, long long b) {
    return a + 2 * bits(x) + 3 * (long long)(d * 4.0) + 4 * bits(y) + 5 * (long long)(f * 4.0f) + 6 * b;
}

long long c_nine(hs x0, hs x1, hs x2, hs x3, hs x4, hs x5, hs x6, hs x7, hs x8, int tail) {
    return bits(x0) + 2 * bits(x1) + 3 * bits(x2) + 4 * bits(x3) + 5 * bits(x4) + 6 * bits(x5)
        + 7 * bits(x6) + 8 * bits(x7) + 9 * bits(x8) + 10 * tail;
}

hs c_flip(hs x, int k) {
    if (k == 0) return x;
    return mk((u16)(bits(x) ^ 0x8000));
}

long long c_recs(H2 p, HI q, HF r, H4 s, H5 t) {
    long long sum = hbits(p.a) + 2 * hbits(p.b) + 3 * hbits(q.a) + 4 * q.b + 5 * hbits(r.a) + 6 * (long long)(r.b * 4.0f);
    for (int i = 0; i < 4; i++) sum += (7 + i) * hbits(s.v[i]);
    for (int i = 0; i < 5; i++) sum += (11 + i) * hbits(t.v[i]);
    return sum;
}

H2 c_r2(hs a, hs b) {
    H2 r;
    r.a = tomem(b);
    r.b = tomem(a);
    return r;
}

HI c_ri(hs a, int b) {
    HI r;
    r.a = tomem(a);
    r.b = b * 3;
    return r;
}

HF c_rf(hs a, float b) {
    HF r;
    r.a = tomem(a);
    r.b = b * 2.0f;
    return r;
}

H4 c_r4(hs a) {
    H4 r;
    for (int i = 0; i < 4; i++) r.v[i] = mkh((u16)(bits(a) + i));
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
long long m_scalars(int a, hs x, double d, hs y, float f, long long b) MACH_SYM(m_scalars);
long long m_nine(hs x0, hs x1, hs x2, hs x3, hs x4, hs x5, hs x6, hs x7, hs x8, int tail) MACH_SYM(m_nine);
hs m_flip(hs x, int k) MACH_SYM(m_flip);
long long m_recs(H2 p, HI q, HF r, H4 s, H5 t) MACH_SYM(m_recs);
H2 m_r2(hs a, hs b) MACH_SYM(m_r2);
HI m_ri(hs a, int b) MACH_SYM(m_ri);
HF m_rf(hs a, float b) MACH_SYM(m_rf);
H4 m_r4(hs a) MACH_SYM(m_r4);

long long c_drives_mach(void) {
    const hs x = mk(0x3E00), y = mk(0xC080);
    if (m_scalars(7, x, 2.5, y, 1.25f, 9) != c_scalars(7, x, 2.5, y, 1.25f, 9)) return 1;
    if (m_nine(mk(1), mk(2), mk(3), mk(4), mk(5), mk(6), mk(7), mk(8), mk(9), 10)
        != c_nine(mk(1), mk(2), mk(3), mk(4), mk(5), mk(6), mk(7), mk(8), mk(9), 10)) return 2;
    if (bits(m_flip(x, 0)) != 0x3E00 || bits(m_flip(x, 1)) != 0xBE00) return 3;
    H2 p = { mkh(0x3C00), mkh(0x4000) };
    HI q = { mkh(0x4200), 5 };
    HF r = { mkh(0x4400), 2.5f };
    H4 s = { { mkh(0x4500), mkh(0x4600), mkh(0x4700), mkh(0x4800) } };
    H5 t = { { mkh(0x4880), mkh(0x4900), mkh(0x4980), mkh(0x4A00), mkh(0x4A80) } };
    if (m_recs(p, q, r, s, t) != c_recs(p, q, r, s, t)) return 4;
    const H2 r2 = m_r2(x, y);
    if (hbits(r2.a) != 0xC080 || hbits(r2.b) != 0x3E00) return 5;
    const HI ri = m_ri(y, 11);
    if (hbits(ri.a) != 0xC080 || ri.b != 33) return 6;
    const HF rf = m_rf(x, 1.5f);
    if (hbits(rf.a) != 0x3E00 || rf.b != 3.0f) return 7;
    const H4 r4 = m_r4(y);
    for (int i = 0; i < 4; i++) {
        if (hbits(r4.v[i]) != 0xC080 + i) return 8;
    }
    return 0;
}
