#include "corpus.h"

/* the constant side is written as macros so it is genuinely compile-time on this
 * side too, and the runtime side as ordinary locals, mirroring the case. every
 * constant stays inside uint64_t range without wrapping, because mach refuses a
 * global initialiser whose comptime fold leaves the type's range and a reference
 * that wrapped would be anchoring a value the constant side cannot express. */

#define KNUTH UINT64_C(2654435761)

#define K0 UINT64_C(305419896)
#define K1 (uint64_t)(K0 * UINT64_C(3) + UINT64_C(7))
#define K2 (uint64_t)((K1 << 13) ^ UINT64_C(0x0F0F0F0F0F))
#define K3 (uint64_t)(K2 ^ (K2 >> 7))
#define K4 (uint64_t)(K3 & UINT64_C(0xFFFFFFFF))
#define K5 (uint64_t)(K4 * KNUTH)
#define K6 (uint64_t)(K5 >> 17)
#define K7 (uint64_t)(K6 + K4)

#define F0 ((double)(1.0 / 3.0))
#define F1 ((double)(F0 * 7.0 - 2.0))
#define F2 ((double)(F1 / 1.1))
#define F3 ((double)(F2 * F2 + F0))

#define G0 ((float)(1.0f / 3.0f))
#define G1 ((float)(G0 * 7.0f - 2.0f))
#define G2 ((float)(G1 / 1.1f))
#define G3 ((float)(G2 * G2 + G0))

#define MODE_MIX UINT8_C(0)
#define MODE_ROT UINT8_C(1)

static uint64_t apply_mix(uint64_t n) {
    return (uint64_t)((uint64_t)(n ^ (n >> 29)) * UINT64_C(1099511628211));
}

static uint64_t apply_rot(uint64_t n) {
    return (uint64_t)((uint64_t)((n << 17) | (n >> 47)) + UINT64_C(12345));
}

static uint64_t apply_rt(uint8_t mode, uint64_t n) {
    uint64_t r = UINT64_C(0);
    if (mode == MODE_MIX) { r = apply_mix(n); }
    else if (mode == MODE_ROT) { r = apply_rot(n); }
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t r0 = (uint64_t)(K0 + seed);
    uint64_t r1 = (uint64_t)(r0 * UINT64_C(3) + UINT64_C(7));
    uint64_t r2 = (uint64_t)((r1 << 13) ^ UINT64_C(0x0F0F0F0F0F));
    uint64_t r3 = (uint64_t)(r2 ^ (r2 >> 7));
    uint64_t r4 = (uint64_t)(r3 & UINT64_C(0xFFFFFFFF));
    uint64_t r5 = (uint64_t)(r4 * KNUTH);
    uint64_t r6 = (uint64_t)(r5 >> 17);
    uint64_t r7 = (uint64_t)(r6 + r4);

    h = mix_u64(h, K0);
    h = mix_u64(h, r0);
    h = mix_u64(h, K1);
    h = mix_u64(h, r1);
    h = mix_u64(h, K2);
    h = mix_u64(h, r2);
    h = mix_u64(h, K3);
    h = mix_u64(h, r3);
    h = mix_u64(h, K4);
    h = mix_u64(h, r4);
    h = mix_u64(h, K5);
    h = mix_u64(h, r5);
    h = mix_u64(h, K6);
    h = mix_u64(h, r6);
    h = mix_u64(h, K7);
    h = mix_u64(h, r7);

    const double zd = (double)seed;
    double d0 = (double)((1.0 + zd) / 3.0);
    double d1 = (double)(d0 * 7.0 - 2.0);
    double d2 = (double)(d1 / 1.1);
    double d3 = (double)(d2 * d2 + d0);

    h = mix_f64(h, F0);
    h = mix_f64(h, d0);
    h = mix_f64(h, F1);
    h = mix_f64(h, d1);
    h = mix_f64(h, F2);
    h = mix_f64(h, d2);
    h = mix_f64(h, F3);
    h = mix_f64(h, d3);

    const float zf = (float)seed;
    float e0 = (float)((1.0f + zf) / 3.0f);
    float e1 = (float)(e0 * 7.0f - 2.0f);
    float e2 = (float)(e1 / 1.1f);
    float e3 = (float)(e2 * e2 + e0);

    h = mix_f32(h, G0);
    h = mix_f32(h, e0);
    h = mix_f32(h, G1);
    h = mix_f32(h, e1);
    h = mix_f32(h, G2);
    h = mix_f32(h, e2);
    h = mix_f32(h, G3);
    h = mix_f32(h, e3);

    uint64_t ct_acc = 0;
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(1) * KNUTH));
    h = mix_u64(h, ct_acc);
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(3) * KNUTH));
    h = mix_u64(h, ct_acc);
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(7) * KNUTH));
    h = mix_u64(h, ct_acc);
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(15) * KNUTH));
    h = mix_u64(h, ct_acc);
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(31) * KNUTH));
    h = mix_u64(h, ct_acc);
    ct_acc = (uint64_t)(ct_acc ^ (uint64_t)(UINT64_C(63) * KNUTH));
    h = mix_u64(h, ct_acc);

    uint64_t taps[6];
    taps[0] = (uint64_t)(UINT64_C(1) + seed);
    taps[1] = (uint64_t)(UINT64_C(3) + seed);
    taps[2] = (uint64_t)(UINT64_C(7) + seed);
    taps[3] = (uint64_t)(UINT64_C(15) + seed);
    taps[4] = (uint64_t)(UINT64_C(31) + seed);
    taps[5] = (uint64_t)(UINT64_C(63) + seed);

    uint64_t rt_acc = 0;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        rt_acc = (uint64_t)(rt_acc ^ (uint64_t)(taps[i] * KNUTH));
        h = mix_u64(h, rt_acc);
    }

    h = mix_u64(h, (K7 < K5) ? UINT64_C(11) : UINT64_C(22));
    h = mix_u64(h, (r7 < r5) ? UINT64_C(11) : UINT64_C(22));

    h = mix_u64(h, (K4 > UINT64_C(0xFFFF)) ? UINT64_C(33) : UINT64_C(44));
    h = mix_u64(h, (r4 > UINT64_C(0xFFFF)) ? UINT64_C(33) : UINT64_C(44));

    const uint8_t m0 = (uint8_t)(MODE_MIX + (uint8_t)seed);
    const uint8_t m1 = (uint8_t)(MODE_ROT + (uint8_t)seed);

    h = mix_u64(h, apply_mix(r7));
    h = mix_u64(h, apply_rt(m0, r7));
    h = mix_u64(h, apply_rot(r7));
    h = mix_u64(h, apply_rt(m1, r7));
    h = mix_u64(h, apply_mix(r4));
    h = mix_u64(h, apply_rt(m0, r4));
    h = mix_u64(h, apply_rot(r4));
    h = mix_u64(h, apply_rt(m1, r4));

    return h;
}
