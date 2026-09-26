/* C half of the empty record controls (mach #3930). Compiled at -O0 so every read
 * really happens. An empty struct is a GNU C extension of size 0, which is what
 * mach's `rec E { }` is. */
#ifdef _WIN32
int _fltused = 0;
#endif

struct e {};

long long c_e(long long first, struct e v, long long next) { return first * 100 + next; }
long long c_ee(struct e v, long long first, struct e w, double x, long long next) {
    return first * 100 + (long long)(x * 2.0) * 10000 + next;
}
long long c_e7(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct e v, long long next) {
    return (a + b + c + d + e + f + g) * 100 + next;
}

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

/* the reverse direction: C calls mach with the same shapes */
long long m_e(long long first, struct e v, long long next) MACH_SYM(m_e);
long long m_ee(struct e v, long long first, struct e w, double x, long long next) MACH_SYM(m_ee);
long long m_e7(long long a, long long b, long long c, long long d, long long e, long long f, long long g,
    struct e v, long long next) MACH_SYM(m_e7);

long long c_drives_mach(void) {
    struct e v;
    if (m_e(3, v, 5) != 305) return 1;
    if (m_ee(v, 3, v, 1.5, 5) != 30305) return 2;
    if (m_e7(1, 1, 1, 1, 1, 1, 1, v, 5) != 705) return 3;
    return 0;
}
