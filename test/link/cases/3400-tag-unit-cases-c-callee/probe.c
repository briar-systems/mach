/* C half of the unit-case tag controls (mach #3400). Unit cases have no payload, so the
 * union holds only the payload cases and each struct is the same spelling a tag without
 * the unit cases would have. Compiled at -O0 so every field read really happens. */
/* the MSVC-target compiler marks floating-point use with this symbol; nothing links a C runtime here */
#ifdef _WIN32
int _fltused = 0;
#endif

struct u16 { unsigned char d; union { long long p; } u; };
struct u16f { unsigned char d; union { double p; } u; };
struct u13 { unsigned char d; union { unsigned int p; } u; };
struct w13 { unsigned char d; union { struct u13 p; } u; };

long long c_u16(struct u16 v, long long next) { return v.d * 100 + v.u.p + next * 1000000; }
long long c_u16f(struct u16f v, long long next) { return v.d * 100 + (long long)v.u.p + next * 1000; }
long long c_w13(struct w13 v, long long next) { return v.d * 1000 + v.u.p.d * 100 + (long long)v.u.p.u.p + next * 1000000; }

struct u16 c_mk_u16(long long x) { struct u16 r; r.d = 16; r.u.p = x; return r; }
struct u16f c_mk_u16f(double x) { struct u16f r; r.d = 16; r.u.p = x; return r; }
struct w13 c_mk_w13(unsigned int x) { struct w13 r; r.d = 1; r.u.p.d = 13; r.u.p.u.p = x; return r; }

/* the reverse direction: C calls mach with the same shapes */
long long m_u16(struct u16 v, long long next);
long long m_u16f(struct u16f v, long long next);
long long m_w13(struct w13 v, long long next);
struct u16 m_mk_u16(long long x);
struct u16f m_mk_u16f(double x);
struct w13 m_mk_w13(unsigned int x);

long long c_drives_mach(void) {
    if (m_u16(c_mk_u16(-5000000000LL), 3) != 1600 - 5000000000LL + 3000000) return 1;
    if (m_u16f(c_mk_u16f(7.5), 4) != 1600 + 7 + 4000) return 2;
    if (m_w13(c_mk_w13(77), 5) != 1000 + 1300 + 77 + 5000000) return 3;
    struct u16 a = m_mk_u16(-6000000000LL);
    if (a.d != 16 || a.u.p != -6000000000LL) return 4;
    struct u16f b = m_mk_u16f(14.5);
    if (b.d != 16 || b.u.p != 14.5) return 5;
    struct w13 c = m_mk_w13(88);
    if (c.d != 1 || c.u.p.d != 13 || c.u.p.u.p != 88) return 6;
    return 0;
}
