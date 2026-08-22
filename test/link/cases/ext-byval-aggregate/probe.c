/* the C half of the by-value aggregate case (mach #3063).
 *
 * Compiled by the RUNNER'S OWN C toolchain, so the callee's treatment of its
 * parameter is the platform's real calling convention rather than mach's model of
 * it. Each function OVERWRITES every field of a by-value parameter, which C says
 * the caller cannot observe - and which mach's caller could observe, because it
 * handed over the address of its own object instead of the address of a copy.
 *
 * WHY -O0. The writes are dead in the C sense: nothing reads the parameter after
 * them that an optimizer cannot fold. At -O1 clang deletes the stores outright and
 * the probe measures nothing at all, so the case would pass against a compiler with
 * the defect. The instrument has to perform the writes for the measurement to
 * exist, and this is the one flag that guarantees it. */

struct wide { long long a, b, c, d; };
struct pair { long long a, b; };

/* 32 bytes: by reference under AAPCS64, the RISC-V psABI and the Microsoft
 * convention; by value on the stack under System V, where the copy already exists
 * and there is nothing to get wrong. */
long long clobber_wide(struct wide w) {
    w.a = -1;
    w.b = -2;
    w.c = -3;
    w.d = -4;
    return w.a + w.b + w.c + w.d;
}

/* 16 bytes: two registers under AAPCS64, System V and lp64d, and BY REFERENCE
 * under the Microsoft convention, which passes anything that is not 1, 2, 4 or 8
 * bytes that way. It is here so the windows leg exercises the by-reference path at
 * a size where the arm64 and System V legs do not - the rule this case is about
 * follows the convention, not the byte count. */
long long clobber_pair(struct pair p) {
    p.a = -5;
    p.b = -6;
    return p.a + p.b;
}
