/* the C half of the `va_list` parameter case (mach #3004).
 *
 * Compiled by the RUNNER'S OWN C toolchain, so both the list this originates and
 * the `va_arg` that reads it are the platform's real ABI rather than mach's model
 * of it. mach never constructs a `va_list` and never reads one: it receives the
 * token here, hands it to `va_take`, and the value that comes back is the
 * observable.
 *
 * THE LIST IS HANDED OVER THROUGH A FUNCTION POINTER mach supplies, which is the
 * shape the motivating report used (`SetTraceLogCallback`) and also the shape that
 * needs no agreement about how a mach symbol is spelled in a C object.
 *
 * WHY -O0. `va_take` takes its `va_list` BY VALUE, and whether the caller owns that
 * copy is precisely what this case measures. Inlining it removes the parameter pass
 * being measured, so the C control would stop describing the platform's calling
 * convention and start describing the optimizer's. */

#include <stdarg.h>

typedef long long (*relay_fn)(long long *out, va_list ap);

/* consume ONE argument from `ap`. by value, exactly as `vsnprintf` takes one. */
long long va_take(va_list ap) { return va_arg(ap, long long); }

static long long call_relay(relay_fn relay, long long *out, ...) {
    va_list ap;
    va_start(ap, out);
    long long r = relay(out, ap);
    va_end(ap);
    return r;
}

/* C originates a list and hands it to MACH, which forwards it to `va_take` twice. */
long long c_probe(relay_fn relay, long long *out) {
    return call_relay(relay, out, 111LL, 222LL, 333LL);
}

static long long relay_in_c(long long *out, va_list ap) {
    out[0] = va_take(ap);
    out[1] = va_take(ap);
    return 2;
}

/* THE PLATFORM'S OWN ANSWER to the same experiment, written entirely in C.
 *
 * Two forwards of one `va_list` do not mean the same thing everywhere, and that is
 * a fact about C rather than about mach: under AAPCS64 the type is a 32-byte
 * composite the caller copies, so each callee walks its own and both read argument
 * one; under SysV x86-64 it is an array that decays to a pointer, so the first
 * callee advances the caller's list and the second reads argument two. The golden
 * therefore asserts that MACH AGREES WITH THIS, not that any particular pair of
 * numbers comes back - which is the only claim that holds on every leg and is also
 * the exact claim that fails on AAPCS64-linux when mach forwards the received
 * pointer instead of a fresh copy. */
long long c_control(long long *out) {
    return call_relay(relay_in_c, out, 111LL, 222LL, 333LL);
}
