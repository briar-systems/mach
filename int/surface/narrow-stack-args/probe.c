/* the C half of the narrow-stack-args surface case (mach #2598).
 *
 * Compiled by the RUNNER'S OWN C toolchain, so each callee reads its stack arguments
 * at the offsets the platform's real ABI puts them at rather than mach's model of it.
 * That is the whole point: Apple's arm64 ABI gives a stack-passed FIXED argument its
 * natural size and alignment, while the AAPCS64 base standard rounds every one up to
 * an eight-byte slot, so a caller using the wrong rule writes each argument into a
 * live offset belonging to a DIFFERENT argument. The result is a wrong VALUE here, not
 * a link error or a crash.
 *
 * Both entry points take `out` first so it consumes the first GP argument register:
 * the fillers that follow exhaust the register file exactly, and every argument after
 * them is stack-passed. Each argument is written to `out` for the mach caller to
 * print, so a misplaced one is visible individually rather than folded into a sum. */

/* eight GP registers consumed (out + seven fillers), then a narrow tail.
 *
 * Apple arm64 places the tail at [sp+0] (int), [sp+4] (short), [sp+6] (signed char),
 * [sp+8] (int, realigned to 4), [sp+16] (long long, realigned to 8) - 24 bytes total.
 * AAPCS64 places it at 0, 8, 16, 24, 32 - 40 bytes. Every offset past the first
 * differs, so a caller on the wrong rule gets four wrong values out of five. */
long long narrow_tail(long long *out, long long a, long long b, long long c,
                      long long d, long long e, long long f, long long g,
                      int i, short j, signed char k, int l, long long m) {
    out[0] = a; out[1] = b; out[2] = c; out[3] = d;
    out[4] = e; out[5] = f; out[6] = g;
    out[7]  = (long long)i;
    out[8]  = (long long)j;
    out[9]  = (long long)k;
    out[10] = (long long)l;
    out[11] = m;
    return 12;
}

/* the same divergence on the FP side: eight doubles exhaust V0-V7, then a narrow tail.
 *
 * Apple arm64 places it at [sp+0] (float), [sp+4] (float), [sp+8] (double); AAPCS64 at
 * 0, 8, 16. Each value is scaled by 10 and truncated so the observable is exact integer
 * text on every target. */
long long float_tail(long long *out, double a, double b, double c, double d,
                     double e, double f, double g, double h,
                     float i, float j, double k) {
    out[0] = (long long)(a * 10.0); out[1] = (long long)(b * 10.0);
    out[2] = (long long)(c * 10.0); out[3] = (long long)(d * 10.0);
    out[4] = (long long)(e * 10.0); out[5] = (long long)(f * 10.0);
    out[6] = (long long)(g * 10.0); out[7] = (long long)(h * 10.0);
    out[8]  = (long long)((double)i * 10.0);
    out[9]  = (long long)((double)j * 10.0);
    out[10] = (long long)(k * 10.0);
    return 11;
}
