/* f32 and f64 add, sub, mul, div, negate and the truncated remainder.
 * one noinline part per width. */
#include "corpus.h"

/* the reference stores every intermediate into a named float, so no result can be
 * carried in a wider evaluation format than the case computes it in. */

static float opaque_f32_arith_f32(uint32_t bits, uint32_t s) {
    uint32_t b = (uint32_t)(bits + s);
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold32_arith_f32(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

/* mach's float `%` is the truncated remainder rem = x - trunc(x / y) * y, with the
 * truncation taken through i64. every operand pair the case feeds it has an exact
 * quotient truncation and an exact product, so this agrees with the IEEE remainder
 * and the oracle carries no lowering detail of its own. the i64 conversion is in
 * range for every pair, so it is defined in C. */
static float frem32_arith_f32(float x, float y) {
    float q = x / y;
    int64_t t = (int64_t)q;
    float tf = (float)t;
    float p = tf * y;
    return x - p;
}

static uint64_t arith_f32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const float one = opaque_f32_arith_f32(UINT32_C(0x3F800000), s);

    const float a = 1.5f * one;
    const float b = 0.25f * one;
    h = fold32_arith_f32(h, a + b);
    h = fold32_arith_f32(h, a - b);
    h = fold32_arith_f32(h, b - a);
    h = fold32_arith_f32(h, a * b);
    h = fold32_arith_f32(h, a / b);
    h = fold32_arith_f32(h, b / a);
    h = fold32_arith_f32(h, -a);
    h = fold32_arith_f32(h, 0.0f - a);
    h = fold32_arith_f32(h, a - a);
    h = fold32_arith_f32(h, (float)(0.0f - a) + a);

    const float three = 3.0f * one;
    const float third = one / three;
    h = fold32_arith_f32(h, third);
    h = fold32_arith_f32(h, third * three);
    h = fold32_arith_f32(h, (float)((float)(one - third) - third) - third);
    h = fold32_arith_f32(h, one / 49.0f);
    h = fold32_arith_f32(h, one / 1048576.5f);
    h = fold32_arith_f32(h, 1048576.5f / three);

    const float p24 = 16777216.0f * one;
    h = fold32_arith_f32(h, p24 + one);
    h = fold32_arith_f32(h, (float)(p24 + one) + one);
    h = fold32_arith_f32(h, p24 + (float)(one + one));
    h = fold32_arith_f32(h, p24 - one);
    h = fold32_arith_f32(h, (float)(p24 + one) - p24);

    float acc = 0.0f * one;
    for (uint32_t i = 0; i < UINT32_C(24); i = (uint32_t)(i + UINT32_C(1))) {
        acc = acc + third;
        acc = acc * 1.5f;
        h = fold32_arith_f32(h, acc);
    }

    const float fmax = opaque_f32_arith_f32(UINT32_C(0x7F7FFFFF), s);
    h = fold32_arith_f32(h, fmax);
    h = fold32_arith_f32(h, fmax / 2.0f);
    h = fold32_arith_f32(h, fmax + fmax);
    h = fold32_arith_f32(h, fmax * 2.0f);
    h = fold32_arith_f32(h, 0.0f - (float)(fmax * 2.0f));
    h = fold32_arith_f32(h, fmax * fmax);
    h = fold32_arith_f32(h, fmax - fmax);

    const float fmin = opaque_f32_arith_f32(UINT32_C(0x00800000), s);
    const float tiny = opaque_f32_arith_f32(UINT32_C(0x00000001), s);
    h = fold32_arith_f32(h, fmin * 0.5f);
    h = fold32_arith_f32(h, fmin / 2.0f);
    h = fold32_arith_f32(h, fmin - tiny);
    h = fold32_arith_f32(h, fmin * one);
    h = fold32_arith_f32(h, tiny + tiny);
    h = fold32_arith_f32(h, tiny * 0.5f);
    h = fold32_arith_f32(h, tiny * 0.75f);
    h = fold32_arith_f32(h, tiny * 16777216.0f);
    h = fold32_arith_f32(h, tiny / fmin);

    const float r = 5.5f * one;
    const float q = 3.0f * one;
    h = fold32_arith_f32(h, frem32_arith_f32(r, q));
    h = fold32_arith_f32(h, frem32_arith_f32(0.0f - r, q));
    h = fold32_arith_f32(h, frem32_arith_f32(r, 0.0f - q));
    h = fold32_arith_f32(h, frem32_arith_f32(0.0f - r, 0.0f - q));
    h = fold32_arith_f32(h, frem32_arith_f32(7.0f * one, 0.5f));
    h = fold32_arith_f32(h, frem32_arith_f32(one, q));
    h = fold32_arith_f32(h, frem32_arith_f32(q, q));
    h = fold32_arith_f32(h, frem32_arith_f32(1048576.0f * one, q));
    return h;
}

static double opaque_f64_arith_f64(uint64_t bits, uint64_t s) {
    uint64_t b = (uint64_t)(bits + s);
    double f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold64_arith_f64(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

/* mach's float `%` is the truncated remainder rem = x - trunc(x / y) * y, with the
 * truncation taken through i64. every operand pair the case feeds it has an exact
 * quotient truncation and an exact product, so this agrees with the IEEE remainder
 * and the oracle carries no lowering detail of its own. the i64 conversion is in
 * range for every pair, so it is defined in C. */
static double frem64_arith_f64(double x, double y) {
    double q = x / y;
    int64_t t = (int64_t)q;
    double tf = (double)t;
    double p = tf * y;
    return x - p;
}

static uint64_t arith_f64(uint64_t seed) {
    uint64_t h = fold_init();

    const double one = opaque_f64_arith_f64(UINT64_C(0x3FF0000000000000), seed);

    const double a = 1.5 * one;
    const double b = 0.25 * one;
    h = fold64_arith_f64(h, a + b);
    h = fold64_arith_f64(h, a - b);
    h = fold64_arith_f64(h, b - a);
    h = fold64_arith_f64(h, a * b);
    h = fold64_arith_f64(h, a / b);
    h = fold64_arith_f64(h, b / a);
    h = fold64_arith_f64(h, -a);
    h = fold64_arith_f64(h, 0.0 - a);
    h = fold64_arith_f64(h, a - a);
    h = fold64_arith_f64(h, (double)(0.0 - a) + a);

    const double three = 3.0 * one;
    const double third = one / three;
    h = fold64_arith_f64(h, third);
    h = fold64_arith_f64(h, third * three);
    h = fold64_arith_f64(h, (double)((double)(one - third) - third) - third);
    h = fold64_arith_f64(h, one / 49.0);
    h = fold64_arith_f64(h, one / 1048576.5);
    h = fold64_arith_f64(h, 1048576.5 / three);

    const double p53 = 9007199254740992.0 * one;
    h = fold64_arith_f64(h, p53 + one);
    h = fold64_arith_f64(h, (double)(p53 + one) + one);
    h = fold64_arith_f64(h, p53 + (double)(one + one));
    h = fold64_arith_f64(h, p53 - one);
    h = fold64_arith_f64(h, (double)(p53 + one) - p53);

    double acc = 0.0 * one;
    for (uint64_t i = 0; i < UINT64_C(24); i = (uint64_t)(i + UINT64_C(1))) {
        acc = acc + third;
        acc = acc * 1.5;
        h = fold64_arith_f64(h, acc);
    }

    const double dmax = opaque_f64_arith_f64(UINT64_C(0x7FEFFFFFFFFFFFFF), seed);
    h = fold64_arith_f64(h, dmax);
    h = fold64_arith_f64(h, dmax / 2.0);
    h = fold64_arith_f64(h, dmax + dmax);
    h = fold64_arith_f64(h, dmax * 2.0);
    h = fold64_arith_f64(h, 0.0 - (double)(dmax * 2.0));
    h = fold64_arith_f64(h, dmax * dmax);
    h = fold64_arith_f64(h, dmax - dmax);

    const double dmin = opaque_f64_arith_f64(UINT64_C(0x0010000000000000), seed);
    const double tiny = opaque_f64_arith_f64(UINT64_C(0x0000000000000001), seed);
    h = fold64_arith_f64(h, dmin * 0.5);
    h = fold64_arith_f64(h, dmin / 2.0);
    h = fold64_arith_f64(h, dmin - tiny);
    h = fold64_arith_f64(h, dmin * one);
    h = fold64_arith_f64(h, tiny + tiny);
    h = fold64_arith_f64(h, tiny * 0.5);
    h = fold64_arith_f64(h, tiny * 0.75);
    h = fold64_arith_f64(h, tiny * 9007199254740992.0);
    h = fold64_arith_f64(h, tiny / dmin);

    const double r = 5.5 * one;
    const double q = 3.0 * one;
    h = fold64_arith_f64(h, frem64_arith_f64(r, q));
    h = fold64_arith_f64(h, frem64_arith_f64(0.0 - r, q));
    h = fold64_arith_f64(h, frem64_arith_f64(r, 0.0 - q));
    h = fold64_arith_f64(h, frem64_arith_f64(0.0 - r, 0.0 - q));
    h = fold64_arith_f64(h, frem64_arith_f64(7.0 * one, 0.5));
    h = fold64_arith_f64(h, frem64_arith_f64(one, q));
    h = fold64_arith_f64(h, frem64_arith_f64(q, q));
    h = fold64_arith_f64(h, frem64_arith_f64(1048576.0 * one, q));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, arith_f32(seed));
    h = mix_u64(h, arith_f64(seed));
    return h;
}
