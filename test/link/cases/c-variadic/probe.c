/* the C half of the c-variadic surface case (mach #2575).
 *
 * Compiled by the RUNNER'S OWN C toolchain, so the va_arg side of every call is the
 * platform's real ABI rather than mach's model of it. That is the whole point: on
 * Apple arm64 a variadic argument is read off the stack, and a caller that placed it
 * in a register produces a wrong VALUE here, not a link error or a crash.
 *
 * `spec` names the type of each variadic argument, one character per argument, so a
 * single entry point covers every shape the case exercises. Each argument is folded
 * into a long long and written to `out`, which the mach caller prints. A double is
 * scaled by 1000 and truncated so the observable is exact integer text on every
 * target; a pointer is dereferenced rather than printed, since its value is not
 * reproducible across runs. */

#include <stdarg.h>
#include <stdint.h>

struct pair { long long a, b; };
struct wide { long long a, b, c, d; };

long long va_probe(const char *spec, long long *out, ...) {
    va_list ap;
    va_start(ap, out);

    long long n = 0;
    for (const char *p = spec; *p; ++p) {
        switch (*p) {
        case 'i': /* int, the width C's default argument promotions produce */
            out[n] = (long long)va_arg(ap, int);
            break;
        case 'l':
            out[n] = va_arg(ap, long long);
            break;
        case 'd':
            out[n] = (long long)(va_arg(ap, double) * 1000.0);
            break;
        case 'p':
            out[n] = *(long long *)va_arg(ap, void *);
            break;
        case 's': { /* a 16-byte aggregate by value */
            struct pair q = va_arg(ap, struct pair);
            out[n] = q.a * 100 + q.b;
            break;
        }
        case 'w': { /* a 32-byte aggregate: by reference under AAPCS64, by value under SysV */
            struct wide q = va_arg(ap, struct wide);
            out[n] = q.a * 1000 + q.b * 100 + q.c * 10 + q.d;
            break;
        }
        default:
            va_end(ap);
            return -1;
        }
        ++n;
    }

    va_end(ap);
    return n;
}
