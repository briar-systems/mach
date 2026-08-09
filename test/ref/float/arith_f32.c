#include "corpus.h"

/* the reference stores every intermediate into a named float, so no result can be
 * carried in a wider evaluation format than the case computes it in. */

static float opaque_f32(uint32_t bits, uint32_t s) {
    uint32_t b = (uint32_t)(bits + s);
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold32(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

/* mach's float `%` is the truncated remainder rem = x - trunc(x / y) * y, with the
 * truncation taken through i64. every operand pair the case feeds it has an exact
 * quotient truncation and an exact product, so this agrees with the IEEE remainder
 * and the oracle carries no lowering detail of its own. the i64 conversion is in
 * range for every pair, so it is defined in C. */
static float frem32(float x, float y) {
    float q = x / y;
    int64_t t = (int64_t)q;
    float tf = (float)t;
    float p = tf * y;
    return x - p;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const float one = opaque_f32(UINT32_C(0x3F800000), s);

    const float a = 1.5f * one;
    const float b = 0.25f * one;
    h = fold32(h, a + b);
    h = fold32(h, a - b);
    h = fold32(h, b - a);
    h = fold32(h, a * b);
    h = fold32(h, a / b);
    h = fold32(h, b / a);
    h = fold32(h, -a);
    h = fold32(h, 0.0f - a);
    h = fold32(h, a - a);
    h = fold32(h, (float)(0.0f - a) + a);

    const float three = 3.0f * one;
    const float third = one / three;
    h = fold32(h, third);
    h = fold32(h, third * three);
    h = fold32(h, (float)((float)(one - third) - third) - third);
    h = fold32(h, one / 49.0f);
    h = fold32(h, one / 1048576.5f);
    h = fold32(h, 1048576.5f / three);

    const float p24 = 16777216.0f * one;
    h = fold32(h, p24 + one);
    h = fold32(h, (float)(p24 + one) + one);
    h = fold32(h, p24 + (float)(one + one));
    h = fold32(h, p24 - one);
    h = fold32(h, (float)(p24 + one) - p24);

    float acc = 0.0f * one;
    for (uint32_t i = 0; i < UINT32_C(24); i = (uint32_t)(i + UINT32_C(1))) {
        acc = acc + third;
        acc = acc * 1.5f;
        h = fold32(h, acc);
    }

    const float fmax = opaque_f32(UINT32_C(0x7F7FFFFF), s);
    h = fold32(h, fmax);
    h = fold32(h, fmax / 2.0f);
    h = fold32(h, fmax + fmax);
    h = fold32(h, fmax * 2.0f);
    h = fold32(h, 0.0f - (float)(fmax * 2.0f));
    h = fold32(h, fmax * fmax);
    h = fold32(h, fmax - fmax);

    const float fmin = opaque_f32(UINT32_C(0x00800000), s);
    const float tiny = opaque_f32(UINT32_C(0x00000001), s);
    h = fold32(h, fmin * 0.5f);
    h = fold32(h, fmin / 2.0f);
    h = fold32(h, fmin - tiny);
    h = fold32(h, fmin * one);
    h = fold32(h, tiny + tiny);
    h = fold32(h, tiny * 0.5f);
    h = fold32(h, tiny * 0.75f);
    h = fold32(h, tiny * 16777216.0f);
    h = fold32(h, tiny / fmin);

    const float r = 5.5f * one;
    const float q = 3.0f * one;
    h = fold32(h, frem32(r, q));
    h = fold32(h, frem32(0.0f - r, q));
    h = fold32(h, frem32(r, 0.0f - q));
    h = fold32(h, frem32(0.0f - r, 0.0f - q));
    h = fold32(h, frem32(7.0f * one, 0.5f));
    h = fold32(h, frem32(one, q));
    h = fold32(h, frem32(q, q));
    h = fold32(h, frem32(1048576.0f * one, q));
    return h;
}
