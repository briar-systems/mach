/* C half of the narrow integer argument controls (mach #3927). Compiled at -O0 so
 * every read really happens. */
#ifdef _WIN32
int _fltused = 0;
#endif

long long c_narrow(unsigned char a, signed char b, unsigned short c, short d, long long next) {
    return (long long)a + (long long)b * 1000 + (long long)c * 1000000 + (long long)d * 10000000000LL + next;
}

/* on aarch64-darwin the four narrow arguments arrive extended to 32 bits, so read
 * each whole w register; elsewhere the convention defines only the narrow bits */
#if defined(__APPLE__) && defined(__aarch64__)
typedef unsigned int w_u8;
typedef int w_i8;
typedef unsigned int w_u16;
typedef int w_i16;
#else
typedef unsigned char w_u8;
typedef signed char w_i8;
typedef unsigned short w_u16;
typedef short w_i16;
#endif

static long long w_a, w_b, w_c, w_d;

long long c_widened(w_u8 a, w_i8 b, w_u16 c, w_i16 d) {
    w_a = (long long)a;
    w_b = (long long)b;
    w_c = (long long)c;
    w_d = (long long)d;
    return 0;
}

long long c_widened_a(void) { return w_a; }
long long c_widened_b(void) { return w_b; }
long long c_widened_c(void) { return w_c; }
long long c_widened_d(void) { return w_d; }

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same types */
long long m_narrow(unsigned char a, signed char b, unsigned short c, short d, long long next) MACH_SYM(m_narrow);

long long c_drives_mach(void) {
    if (m_narrow(0x81, (signed char)-126, 0xbc01, (short)-17151, 7) != 129 - 126000 + 48129000000LL - 171510000000000LL + 7) return 1;
    if (m_narrow(1, 2, 3, 4, 5) != 1 + 2000 + 3000000 + 40000000000LL + 5) return 2;
    return 0;
}
