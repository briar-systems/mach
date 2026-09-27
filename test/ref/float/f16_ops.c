/* every f16 operator, comparison and conversion over the binary16 values a
 * lowering is most likely to lose. every f16 result is the correctly rounded
 * one: an operation on two binary16 values computed exactly in binary64 and
 * rounded once. that rounding is the compiler's own _Float16 conversion where
 * the host compiler has the type, and otherwise the bit-exact software
 * narrowing below. one noinline part per family, as in the case. */
#include "corpus.h"

#if defined(__FLT16_MANT_DIG__)
static uint16_t narrow_f16_ops(double d) {
    _Float16 f = (_Float16)d;
    uint16_t b;
    memcpy(&b, &f, sizeof b);
    return b;
}

static double widen_f16_ops(uint16_t b) {
    _Float16 f;
    memcpy(&f, &b, sizeof f);
    return (double)f;
}
#else
/* round to nearest even into binary16: overflow to infinity, subnormals, and a
 * NaN keeping its sign and the top of its payload with the quiet bit set */
static uint16_t narrow_f16_ops(double d) {
    uint64_t x;
    memcpy(&x, &d, sizeof x);
    const uint16_t sign = (uint16_t)((x >> 48) & UINT64_C(0x8000));
    const uint64_t exp = (x >> 52) & UINT64_C(0x7FF);
    const uint64_t mant = x & UINT64_C(0xFFFFFFFFFFFFF);
    if (exp == UINT64_C(0x7FF)) {
        if (mant == 0) { return (uint16_t)(sign | 0x7C00u); }
        return (uint16_t)(sign | 0x7E00u | (uint16_t)(mant >> 42));
    }
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

static double widen_f16_ops(uint16_t b) {
    const uint64_t sign = (uint64_t)(b & 0x8000u) << 48;
    const uint64_t e = (uint64_t)((b >> 10) & 0x1Fu);
    const uint64_t m = (uint64_t)(b & 0x3FFu);
    uint64_t r;
    if (e == 31u) {
        r = sign | UINT64_C(0x7FF0000000000000) | (m << 42);
    } else if (e == 0u) {
        const double v = (double)m * (1.0 / 16777216.0);
        memcpy(&r, &v, sizeof r);
        r |= sign;
    } else {
        r = sign | ((e + 1008u) << 52) | (m << 42);
    }
    double d;
    memcpy(&d, &r, sizeof d);
    return d;
}
#endif

static uint16_t opaque_f16_ops(uint16_t bits, uint16_t s) { return (uint16_t)(bits + s); }

static uint64_t fold_f16_ops(uint64_t h, uint16_t v) {
    if ((v & 0x7FFFu) > 0x7C00u) { return mix_u16(h, 0xFFFFu); }
    return mix_u16(h, v);
}

static uint64_t fold64_f16_ops(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

static uint64_t fold32_f16_ops(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

static uint16_t add_f16_ops(uint16_t a, uint16_t b) { return narrow_f16_ops(widen_f16_ops(a) + widen_f16_ops(b)); }
static uint16_t sub_f16_ops(uint16_t a, uint16_t b) { return narrow_f16_ops(widen_f16_ops(a) - widen_f16_ops(b)); }
static uint16_t mul_f16_ops(uint16_t a, uint16_t b) { return narrow_f16_ops(widen_f16_ops(a) * widen_f16_ops(b)); }
static uint16_t div_f16_ops(uint16_t a, uint16_t b) { return narrow_f16_ops(widen_f16_ops(a) / widen_f16_ops(b)); }

/* mach's float `%` is the truncated remainder a - trunc(a / b) * b, exact and
 * so always representable, and the NaN that formula makes for an infinite or
 * NaN divisor or a NaN dividend. every binary16 value is an integer
 * multiple of 2^-24 below 2^40 of them, so the remainder is taken on those
 * integers; a zero result keeps the dividend's sign */
static uint16_t rem_f16_ops(uint16_t a, uint16_t b) {
    const double x = widen_f16_ops(a);
    const double y = widen_f16_ops(b);
    if (x - x != 0.0 || y - y != 0.0 || y == 0.0) { return 0x7E00u; }
    const int64_t xi = (int64_t)(x * 16777216.0);
    const int64_t yi = (int64_t)(y * 16777216.0);
    const int64_t r = xi % yi;
    if (r == 0) { return (uint16_t)(a & 0x8000u); }
    return narrow_f16_ops((double)r / 16777216.0);
}

static uint16_t table_f16_ops(uint32_t i) {
    uint16_t t[22];
    t[0] = 0x0000u;
    t[1] = 0x8000u;
    t[2] = 0x0001u;
    t[3] = 0x8001u;
    t[4] = 0x03FFu;
    t[5] = 0x83FFu;
    t[6] = 0x0400u;
    t[7] = 0x7BFFu;
    t[8] = 0xFBFFu;
    t[9] = 0x7C00u;
    t[10] = 0xFC00u;
    t[11] = 0x7E00u;
    t[12] = 0x3C00u;
    t[13] = 0xBC00u;
    t[14] = 0x3C01u;
    t[15] = 0x3E00u;
    t[16] = 0x4200u;
    t[17] = 0x2E66u;
    t[18] = 0x6800u;
    t[19] = 0x3555u;
    t[20] = 0x0200u;
    t[21] = 0xC900u;
    return t[i];
}

#define N_F16_OPS 22u

static uint64_t arith_f16_ops(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;
    for (uint32_t i = 0; i < N_F16_OPS; i++) {
        const uint16_t a = opaque_f16_ops(table_f16_ops(i), s);
        h = fold_f16_ops(h, (uint16_t)(a ^ 0x8000u));
        for (uint32_t j = 0; j < N_F16_OPS; j++) {
            const uint16_t b = opaque_f16_ops(table_f16_ops(j), s);
            h = fold_f16_ops(h, add_f16_ops(a, b));
            h = fold_f16_ops(h, sub_f16_ops(a, b));
            h = fold_f16_ops(h, mul_f16_ops(a, b));
            h = fold_f16_ops(h, div_f16_ops(a, b));
            if ((table_f16_ops(i) & 0x7FFFu) != 0x7C00u && (table_f16_ops(j) & 0x7FFFu) != 0) {
                h = fold_f16_ops(h, rem_f16_ops(a, b));
            }
        }
    }

    const uint16_t big = opaque_f16_ops(0x6800u, s);
    const uint16_t one = opaque_f16_ops(0x3C00u, s);
    h = fold_f16_ops(h, add_f16_ops(big, one));
    h = fold_f16_ops(h, add_f16_ops(add_f16_ops(add_f16_ops(big, one), one), one));
    h = fold_f16_ops(h, add_f16_ops(big, add_f16_ops(add_f16_ops(one, one), one)));
    h = fold_f16_ops(h, add_f16_ops(opaque_f16_ops(0x7BFFu, s), opaque_f16_ops(0x4C00u, s)));
    h = fold_f16_ops(h, add_f16_ops(opaque_f16_ops(0x7BFFu, s), opaque_f16_ops(0x4BFFu, s)));
    h = fold_f16_ops(h, mul_f16_ops(opaque_f16_ops(0x0001u, s), opaque_f16_ops(0x3800u, s)));
    h = fold_f16_ops(h, mul_f16_ops(opaque_f16_ops(0x0003u, s), opaque_f16_ops(0x3800u, s)));
    h = fold_f16_ops(h, add_f16_ops(opaque_f16_ops(0x03FFu, s), opaque_f16_ops(0x0001u, s)));
    return h;
}

static uint64_t cmp_f16_ops(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;
    for (uint32_t i = 0; i < N_F16_OPS; i++) {
        const double a = widen_f16_ops(opaque_f16_ops(table_f16_ops(i), s));
        for (uint32_t j = 0; j < N_F16_OPS; j++) {
            const double b = widen_f16_ops(opaque_f16_ops(table_f16_ops(j), s));
            uint8_t c = 0;
            if (a < b) { c = (uint8_t)(c + 1u); }
            if (a <= b) { c = (uint8_t)(c + 2u); }
            if (a > b) { c = (uint8_t)(c + 4u); }
            if (a >= b) { c = (uint8_t)(c + 8u); }
            if (a == b) { c = (uint8_t)(c + 16u); }
            if (a != b) { c = (uint8_t)(c + 32u); }
            h = mix_u8(h, c);
        }
    }
    return h;
}

static uint64_t widen_part_f16_ops(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;
    for (uint32_t i = 0; i < N_F16_OPS; i++) {
        const uint16_t a = opaque_f16_ops(table_f16_ops(i), s);
        const double w = widen_f16_ops(a);
        h = fold32_f16_ops(h, (float)w);
        h = mix_f64(h, w);
        if ((table_f16_ops(i) & 0x7C00u) != 0x7C00u) {
            h = mix_i32(h, (int32_t)w);
            h = mix_i64(h, (int64_t)w);
            h = mix_i16(h, (int16_t)widen_f16_ops(div_f16_ops(a, opaque_f16_ops(0x5C00u, s))));
            if ((table_f16_ops(i) & 0x8000u) == 0) {
                h = mix_u16(h, (uint16_t)w);
                h = mix_u64(h, (uint64_t)w);
            }
        }
    }
    h = fold64_f16_ops(h, widen_f16_ops(opaque_f16_ops(0x7F55u, s)));
    return h;
}

static double f64_of_f16_ops(uint64_t bits) {
    double d;
    memcpy(&d, &bits, sizeof d);
    return d;
}

static float f32_of_f16_ops(uint32_t bits) {
    float f;
    memcpy(&f, &bits, sizeof f);
    return f;
}

static uint64_t narrow_part_f16_ops(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;
    uint64_t d[24];
    d[0] = UINT64_C(0x40A0020000000000);
    d[1] = UINT64_C(0x40A0060000000000);
    d[2] = UINT64_C(0x40EFFDFFFFFFFFFF);
    d[3] = UINT64_C(0x40EFFE0000000000);
    d[4] = UINT64_C(0xC0EFFE0000000000);
    d[5] = UINT64_C(0x4202A05F20000000);
    d[6] = UINT64_C(0x3E60000000000000);
    d[7] = UINT64_C(0x3E60000000000001);
    d[8] = UINT64_C(0x3E78000000000000);
    d[9] = UINT64_C(0x3E84000000000000);
    d[10] = UINT64_C(0x3F0FFC0000000000);
    d[11] = UINT64_C(0x3E50000000000000);
    d[12] = UINT64_C(0x0000000000000001);
    d[13] = UINT64_C(0x8000000000000000);
    d[14] = UINT64_C(0x7FF0000000000000);
    d[15] = UINT64_C(0xFFF0000000000000);
    d[16] = UINT64_C(0x7FF4000000000001);
    d[17] = UINT64_C(0xFFF8040000000000);
    d[18] = UINT64_C(0x3FB999999999999A);
    d[19] = UINT64_C(0x3FF0020000000000);
    d[20] = UINT64_C(0x3FF0060000000000);
    d[21] = UINT64_C(0x3FF0020000000001);
    d[22] = UINT64_C(0x40EFFC0000000000);
    d[23] = UINT64_C(0x3F10000000000000);

    for (uint32_t i = 0; i < 24u; i++) {
        h = fold_f16_ops(h, narrow_f16_ops(f64_of_f16_ops(d[i] + s)));
    }

    const uint32_t s32 = (uint32_t)seed;
    h = mix_u16(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x3DCCCCCD) + s32)));
    h = mix_u16(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x477FF000) + s32)));
    h = mix_u16(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x33800000) + s32)));
    h = mix_u16(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x33000000) + s32)));
    h = mix_u16(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x45001000) + s32)));
    h = fold_f16_ops(h, narrow_f16_ops((double)f32_of_f16_ops(UINT32_C(0x7FC00001) + s32)));

    const int32_t si = (int32_t)(uint32_t)seed;
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(2049 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(2051 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(65519 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(65520 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(-65520 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(-2051 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(int32_t)(0 + si)));
    h = mix_u16(h, narrow_f16_ops((double)(uint32_t)(UINT32_C(0xFFFFFFFF) + (uint32_t)seed)));
    h = mix_u16(h, narrow_f16_ops((double)(int64_t)(INT64_MAX + (int64_t)seed)));
    h = mix_u16(h, narrow_f16_ops((double)(int64_t)(-INT64_MAX + (int64_t)seed)));
    h = mix_u16(h, narrow_f16_ops((double)(uint64_t)(UINT64_C(0xFFFFFFFFFFFFFFFF) + seed)));
    h = mix_u16(h, narrow_f16_ops((double)(uint8_t)(255u + (uint8_t)seed)));
    h = mix_u16(h, narrow_f16_ops((double)(int16_t)(-32767 + (int16_t)seed)));
    return h;
}

static uint64_t folded_f16_ops(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;
    h = mix_u16(h, (uint16_t)(add_f16_ops(0x6800u, 0x3C00u) + s));
    h = mix_u16(h, (uint16_t)(add_f16_ops(0x6800u, 0x4200u) + s));
    h = mix_u16(h, (uint16_t)(add_f16_ops(0x7BFFu, 0x4C00u) + s));
    h = mix_u16(h, (uint16_t)(mul_f16_ops(0x2E66u, 0x2E66u) + s));
    h = mix_u16(h, (uint16_t)(mul_f16_ops(0x0001u, 0x3E00u) + s));
    h = mix_u16(h, (uint16_t)(div_f16_ops(0x3C00u, 0x4200u) + s));
    h = mix_u16(h, (uint16_t)(rem_f16_ops(0x4580u, 0x4200u) + s));
    h = mix_u16(h, (uint16_t)(0xBE00u + s));
    h = mix_u16(h, (uint16_t)(narrow_f16_ops(65519.0) + s));
    h = mix_u16(h, (uint16_t)(narrow_f16_ops(2049.0) + s));
    h = mix_f64(h, widen_f16_ops(0x0001u));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, arith_f16_ops(seed));
    h = mix_u64(h, cmp_f16_ops(seed));
    h = mix_u64(h, widen_part_f16_ops(seed));
    h = mix_u64(h, narrow_part_f16_ops(seed));
    h = mix_u64(h, folded_f16_ops(seed));
    return h;
}
