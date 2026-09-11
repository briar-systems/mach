/* C half of the record transport controls (mach #3263). Each struct is the C spelling
 * of a mach record of matching layout. Compiled at -O0 so every field read really
 * happens. */
/* the MSVC-target compiler marks floating-point use with this symbol; nothing links a C runtime here */
#ifdef _WIN32
int _fltused = 0;
#endif

struct p1 { unsigned char d; };
struct t3 { unsigned char d; unsigned char p[2]; };
struct s8 { unsigned char d; unsigned int p; };
struct t9 { unsigned char d; unsigned char p[8]; };
struct t16 { unsigned char d; long long p; };
struct o24 { unsigned char d; long long p[2]; };
struct nest { unsigned char d; struct s8 p; };
struct al16 { unsigned char d; unsigned char p; } __attribute__((aligned(16)));
struct tf { unsigned char d; double p; };
struct tf8 { unsigned char d; float p; };
struct tfm { double p; long long q; };

long long c_p1(struct p1 v, long long next) { return v.d * 100 + next; }
long long c_t3(struct t3 v, long long next) { return v.d * 100 + v.p[0] + v.p[1] * 10 + next * 1000; }
long long c_s8(struct s8 v, long long next) { return v.d * 100 + (long long)v.p + next * 1000000; }
long long c_t9(struct t9 v, long long next) { return v.d * 100 + v.p[0] + v.p[7] * 10 + next * 1000; }
long long c_t16(struct t16 v, long long next) { return v.d * 100 + v.p + next * 1000000; }
long long c_o24(struct o24 v, long long next) { return v.d * 100 + v.p[0] + v.p[1] * 10 + next * 1000; }
long long c_nest(struct nest v, long long next) { return v.d * 100 + v.p.d * 10 + (long long)v.p.p + next * 1000000; }
long long c_al16(struct al16 v, long long next) { return v.d * 100 + v.p + next * 1000; }
long long c_al16_after(long long first, struct al16 v, long long next) { return first + v.d * 100 + v.p + next * 1000; }
long long c_tf(struct tf v, long long next) { return v.d * 100 + (long long)v.p + next * 1000; }
long long c_tf8(struct tf8 v, long long next) { return v.d * 100 + (long long)v.p + next * 1000; }
long long c_tfm(struct tfm v, long long next) { return (long long)v.p * 100 + v.q + next * 1000; }

struct p1 c_mk_p1(unsigned char d) { struct p1 r; r.d = d; return r; }
struct t3 c_mk_t3(unsigned char x) { struct t3 r; r.d = 1; r.p[0] = x; r.p[1] = x + 1; return r; }
struct s8 c_mk_s8(unsigned int x) { struct s8 r; r.d = 1; r.p = x; return r; }
struct t9 c_mk_t9(unsigned char x) { struct t9 r; r.d = 1; r.p[0] = x; r.p[7] = x + 1; return r; }
struct t16 c_mk_t16(long long x) { struct t16 r; r.d = 1; r.p = x; return r; }
struct o24 c_mk_o24(long long x) { struct o24 r; r.d = 1; r.p[0] = x; r.p[1] = x + 1; return r; }
struct nest c_mk_nest(unsigned int x) { struct nest r; r.d = 1; r.p.d = 1; r.p.p = x; return r; }
struct al16 c_mk_al16(unsigned char x) { struct al16 r; r.d = 1; r.p = x; return r; }
struct tf c_mk_tf(double x) { struct tf r; r.d = 1; r.p = x; return r; }
struct tf8 c_mk_tf8(float x) { struct tf8 r; r.d = 1; r.p = x; return r; }
struct tfm c_mk_tfm(long long x) { struct tfm r; r.p = 2.5; r.q = x; return r; }

/* the reverse direction: C calls mach with the same shapes */
long long m_t3(struct t3 v, long long next);
long long m_t9(struct t9 v, long long next);
long long m_al16(struct al16 v, long long next);
long long m_al16_after(long long first, struct al16 v, long long next);
long long m_tf(struct tf v, long long next);
long long m_tfm(struct tfm v, long long next);
long long m_o24(struct o24 v, long long next);
struct t9 m_mk_t9(unsigned char x);
struct al16 m_mk_al16(unsigned char x);
struct tf m_mk_tf(double x);
struct tfm m_mk_tfm(long long x);
struct t3 m_mk_t3(unsigned char x);

long long c_drives_mach(void) {
    struct t3 a = c_mk_t3(4);
    struct t9 b = c_mk_t9(5);
    struct al16 c = c_mk_al16(6);
    struct tf d = c_mk_tf(7.5);
    struct o24 e = c_mk_o24(8);
    struct tfm f = c_mk_tfm(-9);
    if (m_t3(a, 1) != 100 + 4 + 50 + 1000) return 1;
    if (m_t9(b, 2) != 100 + 5 + 60 + 2000) return 2;
    if (m_al16(c, 3) != 100 + 6 + 3000) return 3;
    if (m_al16_after(9, c, 3) != 9 + 100 + 6 + 3000) return 4;
    if (m_tf(d, 4) != 100 + 7 + 4000) return 5;
    if (m_o24(e, 5) != 100 + 8 + 90 + 5000) return 6;
    if (m_tfm(f, 6) != 200 - 9 + 6000) return 7;
    struct t9 rb = m_mk_t9(11);
    if (rb.d != 1 || rb.p[0] != 11 || rb.p[7] != 12) return 8;
    struct al16 rc = m_mk_al16(13);
    if (rc.d != 1 || rc.p != 13) return 9;
    struct tf rd = m_mk_tf(14.5);
    if (rd.d != 1 || rd.p != 14.5) return 10;
    struct tfm rf = m_mk_tfm(-15);
    if (rf.p != 2.5 || rf.q != -15) return 11;
    struct t3 ra = m_mk_t3(15);
    if (ra.d != 1 || ra.p[0] != 15 || ra.p[1] != 16) return 12;
    return 0;
}
