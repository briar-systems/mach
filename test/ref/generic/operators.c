#include "corpus.h"

/* the concrete control for test/cases/generic/operators.mach: every row the
 * generic writes once against `T` is written out here at each width. signed
 * mach arithmetic is the two's-complement identity on the matching unsigned
 * type, so every step is defined by the C standard and a disagreement always
 * indicts mach, never this file. */
static inline int64_t bitcast_i64(uint64_t v) {
    int64_t r;
    memcpy(&r, &v, sizeof r);
    return r;
}

static uint8_t int_cmp_u8(uint8_t a, uint8_t b) {
    uint8_t m = 0;
    if (a == b) { m = (uint8_t)(m | 1); }
    if (a != b) { m = (uint8_t)(m | 2); }
    if (a < b)  { m = (uint8_t)(m | 4); }
    if (a > b)  { m = (uint8_t)(m | 8); }
    if (a <= b) { m = (uint8_t)(m | 16); }
    if (a >= b) { m = (uint8_t)(m | 32); }
    if (a == 0) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t int_cmp_u32(uint32_t a, uint32_t b) {
    uint8_t m = 0;
    if (a == b) { m = (uint8_t)(m | 1); }
    if (a != b) { m = (uint8_t)(m | 2); }
    if (a < b)  { m = (uint8_t)(m | 4); }
    if (a > b)  { m = (uint8_t)(m | 8); }
    if (a <= b) { m = (uint8_t)(m | 16); }
    if (a >= b) { m = (uint8_t)(m | 32); }
    if (a == 0) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t int_cmp_u64(uint64_t a, uint64_t b) {
    uint8_t m = 0;
    if (a == b) { m = (uint8_t)(m | 1); }
    if (a != b) { m = (uint8_t)(m | 2); }
    if (a < b)  { m = (uint8_t)(m | 4); }
    if (a > b)  { m = (uint8_t)(m | 8); }
    if (a <= b) { m = (uint8_t)(m | 16); }
    if (a >= b) { m = (uint8_t)(m | 32); }
    if (a == 0) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t int_cmp_i64(int64_t a, int64_t b) {
    uint8_t m = 0;
    if (a == b) { m = (uint8_t)(m | 1); }
    if (a != b) { m = (uint8_t)(m | 2); }
    if (a < b)  { m = (uint8_t)(m | 4); }
    if (a > b)  { m = (uint8_t)(m | 8); }
    if (a <= b) { m = (uint8_t)(m | 16); }
    if (a >= b) { m = (uint8_t)(m | 32); }
    if (a == 0) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t flt_cmp_f64(double a, double b) {
    uint8_t m = 0;
    if (a == b)   { m = (uint8_t)(m | 1); }
    if (a != b)   { m = (uint8_t)(m | 2); }
    if (a < b)    { m = (uint8_t)(m | 4); }
    if (a > b)    { m = (uint8_t)(m | 8); }
    if (a <= b)   { m = (uint8_t)(m | 16); }
    if (a >= b)   { m = (uint8_t)(m | 32); }
    if (a == 0.0) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t flt_cmp_f32(float a, float b) {
    uint8_t m = 0;
    if (a == b)    { m = (uint8_t)(m | 1); }
    if (a != b)    { m = (uint8_t)(m | 2); }
    if (a < b)     { m = (uint8_t)(m | 4); }
    if (a > b)     { m = (uint8_t)(m | 8); }
    if (a <= b)    { m = (uint8_t)(m | 16); }
    if (a >= b)    { m = (uint8_t)(m | 32); }
    if (a == 0.0f) { m = (uint8_t)(m | 64); }
    return m;
}

static uint8_t int_reduce_u8(uint8_t a, uint8_t b) {
    uint8_t acc = 0;
    acc = (uint8_t)(acc + (uint8_t)(a + b));
    acc = (uint8_t)(acc + (uint8_t)(b - a));
    acc = (uint8_t)(acc + (uint8_t)(a * b));
    acc = (uint8_t)(acc + (uint8_t)(b / a));
    acc = (uint8_t)(acc + (uint8_t)(b % a));
    acc = (uint8_t)(acc + (uint8_t)(a & b));
    acc = (uint8_t)(acc + (uint8_t)(a | b));
    acc = (uint8_t)(acc + (uint8_t)(a ^ b));
    acc = (uint8_t)(acc + (uint8_t)((unsigned)a << 2));
    acc = (uint8_t)(acc + (uint8_t)(b >> 1));
    acc = (uint8_t)(acc + (uint8_t)~a);
    acc = (uint8_t)(acc + (uint8_t)(0 - a));
    acc = (uint8_t)(acc + (uint8_t)1);
    return acc;
}

static uint32_t int_reduce_u32(uint32_t a, uint32_t b) {
    uint32_t acc = 0;
    acc = (uint32_t)(acc + (uint32_t)(a + b));
    acc = (uint32_t)(acc + (uint32_t)(b - a));
    acc = (uint32_t)(acc + (uint32_t)(a * b));
    acc = (uint32_t)(acc + (uint32_t)(b / a));
    acc = (uint32_t)(acc + (uint32_t)(b % a));
    acc = (uint32_t)(acc + (uint32_t)(a & b));
    acc = (uint32_t)(acc + (uint32_t)(a | b));
    acc = (uint32_t)(acc + (uint32_t)(a ^ b));
    acc = (uint32_t)(acc + (uint32_t)(a << 2));
    acc = (uint32_t)(acc + (uint32_t)(b >> 1));
    acc = (uint32_t)(acc + (uint32_t)~a);
    acc = (uint32_t)(acc + (uint32_t)(0 - a));
    acc = (uint32_t)(acc + (uint32_t)1);
    return acc;
}

static uint64_t int_reduce_u64(uint64_t a, uint64_t b) {
    uint64_t acc = 0;
    acc = (uint64_t)(acc + (uint64_t)(a + b));
    acc = (uint64_t)(acc + (uint64_t)(b - a));
    acc = (uint64_t)(acc + (uint64_t)(a * b));
    acc = (uint64_t)(acc + (uint64_t)(b / a));
    acc = (uint64_t)(acc + (uint64_t)(b % a));
    acc = (uint64_t)(acc + (uint64_t)(a & b));
    acc = (uint64_t)(acc + (uint64_t)(a | b));
    acc = (uint64_t)(acc + (uint64_t)(a ^ b));
    acc = (uint64_t)(acc + (uint64_t)(a << 2));
    acc = (uint64_t)(acc + (uint64_t)(b >> 1));
    acc = (uint64_t)(acc + (uint64_t)~a);
    acc = (uint64_t)(acc + (uint64_t)(0 - a));
    acc = (uint64_t)(acc + UINT64_C(1));
    return acc;
}

static int64_t int_reduce_i64(int64_t a, int64_t b) {
    uint64_t acc = 0;
    const uint64_t ua = (uint64_t)a;
    const uint64_t ub = (uint64_t)b;
    acc = (uint64_t)(acc + (uint64_t)(ua + ub));
    acc = (uint64_t)(acc + (uint64_t)(ub - ua));
    acc = (uint64_t)(acc + (uint64_t)(ua * ub));
    acc = (uint64_t)(acc + (uint64_t)(b / a));
    acc = (uint64_t)(acc + (uint64_t)(b % a));
    acc = (uint64_t)(acc + (uint64_t)(ua & ub));
    acc = (uint64_t)(acc + (uint64_t)(ua | ub));
    acc = (uint64_t)(acc + (uint64_t)(ua ^ ub));
    acc = (uint64_t)(acc + (uint64_t)(ua << 2));
    acc = (uint64_t)(acc + (uint64_t)(b >> 1));
    acc = (uint64_t)(acc + (uint64_t)~ua);
    acc = (uint64_t)(acc + (uint64_t)(UINT64_C(0) - ua));
    acc = (uint64_t)(acc + UINT64_C(1));
    return bitcast_i64(acc);
}

static double flt_reduce_f64(double a, double b) {
    double acc = 0.0;
    acc = acc + (a + b);
    acc = acc + (b - a);
    acc = acc + (a * b);
    acc = acc + (b / a);
    acc = acc + (0.0 - a);
    acc = acc + 1.0;
    return acc;
}

static float flt_reduce_f32(float a, float b) {
    float acc = 0.0f;
    acc = (float)(acc + (float)(a + b));
    acc = (float)(acc + (float)(b - a));
    acc = (float)(acc + (float)(a * b));
    acc = (float)(acc + (float)(b / a));
    acc = (float)(acc + (float)(0.0f - a));
    acc = (float)(acc + 1.0f);
    return acc;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed & UINT64_C(3);

    const uint8_t a8 = (uint8_t)(UINT64_C(7) + s);
    const uint8_t b8 = (uint8_t)(UINT64_C(19) + s);
    h = mix_u8(h, int_cmp_u8(a8, b8));
    h = mix_u8(h, int_reduce_u8(a8, b8));
    h = mix_u8(h, a8 > b8 ? a8 : b8);

    const uint32_t a32 = (uint32_t)(UINT64_C(7) + s);
    const uint32_t b32 = (uint32_t)(UINT64_C(19) + s);
    h = mix_u8(h, int_cmp_u32(a32, b32));
    h = mix_u32(h, int_reduce_u32(a32, b32));
    h = mix_u32(h, a32 > b32 ? a32 : b32);

    const uint64_t a64 = UINT64_C(7) + s;
    const uint64_t b64 = UINT64_C(19) + s;
    h = mix_u8(h, int_cmp_u64(a64, b64));
    h = mix_u64(h, int_reduce_u64(a64, b64));
    h = mix_u64(h, a64 > b64 ? a64 : b64);

    const int64_t ai = (int64_t)(UINT64_C(7) + s);
    const int64_t bi = (int64_t)(UINT64_C(19) + s);
    h = mix_u8(h, int_cmp_i64(ai, bi));
    h = mix_i64(h, int_reduce_i64(ai, bi));
    h = mix_i64(h, ai > bi ? ai : bi);

    const double af = (double)(UINT64_C(7) + s);
    const double bf = (double)(UINT64_C(19) + s);
    h = mix_u8(h, flt_cmp_f64(af, bf));
    h = mix_f64(h, flt_reduce_f64(af, bf));
    h = mix_f64(h, af > bf ? af : bf);

    const float a32f = (float)(UINT64_C(7) + s);
    const float b32f = (float)(UINT64_C(19) + s);
    h = mix_u8(h, flt_cmp_f32(a32f, b32f));
    h = mix_f32(h, flt_reduce_f32(a32f, b32f));
    h = mix_f32(h, a32f > b32f ? a32f : b32f);

    h = mix_u64(h, sizeof(uint8_t));
    h = mix_u64(h, sizeof(uint32_t));
    h = mix_u64(h, sizeof(double));
    h = mix_u64(h, (uint64_t)ai);
    {
        uint64_t afbits;
        memcpy(&afbits, &af, sizeof afbits);
        h = mix_u64(h, afbits);
    }

    return h;
}
