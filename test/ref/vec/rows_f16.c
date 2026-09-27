#include <math.h>
#include "corpus.h"

/* f16 lanes are held as the doubles they are exactly; + - * / computed in
 * binary64 and rounded once to binary16 is the correctly rounded f16 result,
 * binary64 carrying more than twice binary16's precision plus two bits. the
 * rounding is to nearest even, overflowing to infinity, and no lane here
 * makes a NaN. */
typedef struct { double l[8]; } vec;
typedef struct { uint16_t l[8]; } bits;

static uint16_t narrow(double d) {
    uint64_t x;
    memcpy(&x, &d, sizeof x);
    const uint16_t sign = (uint16_t)((x >> 48) & UINT64_C(0x8000));
    const uint64_t exp = (x >> 52) & UINT64_C(0x7FF);
    const uint64_t mant = x & UINT64_C(0xFFFFFFFFFFFFF);
    if (exp == UINT64_C(0x7FF)) { return (uint16_t)(sign | 0x7C00u); }
    if (exp == 0) { return sign; }
    const uint64_t sig = mant | (UINT64_C(1) << 52);
    int64_t e = (int64_t)exp - 1023 + 15;
    int64_t s = 42;
    if (e <= 0) { s += 1 - e; }
    if (s > 53) { return sign; }
    uint64_t q = sig >> s;
    const uint64_t rem = sig & ((UINT64_C(1) << s) - 1u);
    const uint64_t half = UINT64_C(1) << (s - 1);
    if (rem > half || (rem == half && (q & 1u) != 0)) { q += 1u; }
    if (e <= 0) { return (uint16_t)(sign | (uint16_t)q); }
    if (q == (UINT64_C(1) << 11)) {
        q >>= 1;
        e += 1;
    }
    if (e >= 31) { return (uint16_t)(sign | 0x7C00u); }
    return (uint16_t)(sign | (uint16_t)((uint64_t)e << 10) | (uint16_t)(q & 0x3FFu));
}

/* a half-precision value back to binary64, exactly */
static double widen(uint16_t b) {
    const double m = (double)(b & 0x3FFu);
    const int e = (b >> 10) & 0x1F;
    double v = e == 0 ? ldexp(m, -24) : ldexp(m + 1024.0, e - 25);
    if (e == 31) { v = INFINITY; }
    return (b & 0x8000u) ? -v : v;
}

static bits vadd(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = narrow(x.l[i] + y.l[i]); } return r; }
static bits vsub(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = narrow(x.l[i] - y.l[i]); } return r; }
static bits vmul(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = narrow(x.l[i] * y.l[i]); } return r; }
static bits vdiv(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = narrow(x.l[i] / y.l[i]); } return r; }
static bits vcmp_lt(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = (x.l[i] < y.l[i]) ? (uint16_t)0xFFFFu : (uint16_t)0; } return r; }
static bits vcmp_eq(vec x, vec y) { bits r; for (unsigned i = 0; i < 8u; i++) { r.l[i] = (x.l[i] == y.l[i]) ? (uint16_t)0xFFFFu : (uint16_t)0; } return r; }

static uint64_t fold_bits(uint64_t h, bits v) { for (unsigned i = 0; i < 8u; i++) { h = mix_u16(h, v.l[i]); } return h; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = narrow((double)seed);
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vadd(a, b));
    }
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vsub(a, b));
    }
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vmul(a, b));
    }
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vdiv(a, b));
    }
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vcmp_lt(a, b));
    }
    {
        vec a = {{ 1.5, -2.25, 1024.0, 0.125, 65504.0, 0.00006103515625, -0.000000059604644775390625, 3.0 }};
        vec b = {{ 0.5, 4.0, -2.0, 8.0, 65504.0, 0.5, 0.5, 7.0 }};
        a.l[0] = widen(narrow(a.l[0] + widen(s)));
        h = fold_bits(h, vcmp_eq(a, b));
    }
    return h;
}
