/* the C half of the mach-callee case (mach #3416).
 *
 * Compiled by the RUNNER'S OWN C toolchain, so the copy the caller makes before the
 * call is the platform's real calling convention rather than mach's model of it. The
 * mach callee overwrites every field of its by-value parameter; C says the caller
 * cannot observe that, and after #3416 the caller's copy is the only reason it
 * cannot.
 *
 * The observed fields are handed back for mach to print: a bare pass/fail code would
 * say a field changed without saying to what.
 *
 * WHY -O0. Each field read after the call must actually happen for the measurement
 * to exist. */

struct wide { long long a, b, c, d; };
struct pair { long long a, b; };

/* 32 bytes: the address of a caller-allocated copy under AAPCS64, the RISC-V psABI
 * and the Microsoft convention; the outgoing stack area under System V. */
extern long long m_clobber_wide(struct wide w);

/* 16 bytes: two registers under AAPCS64, System V and lp64d, and by reference under
 * the Microsoft convention. */
extern long long m_clobber_pair(struct pair p);

/* fills out[0..12) with each call's return and the caller's own fields after it */
long long c_drives_mach(long long *out) {
    struct wide w;
    w.a = 1;
    w.b = 2;
    w.c = 3;
    w.d = 4;
    out[0] = m_clobber_wide(w);
    out[1] = w.a;
    out[2] = w.b;
    out[3] = w.c;
    out[4] = w.d;

    struct pair q;
    q.a = 7;
    q.b = 8;
    out[5] = m_clobber_pair(q);
    out[6] = q.a;
    out[7] = q.b;

    /* the same object handed over twice: the second call must see the ORIGINAL
     * values, the shape a caller reusing one temporary would fail. */
    out[8] = m_clobber_wide(w);
    out[9] = w.a;
    out[10] = w.b;
    out[11] = w.c;
    out[12] = w.d;
    return 13;
}
