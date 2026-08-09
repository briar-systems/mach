#include "corpus.h"

/* mach monomorphizes a pack per call-site type-list, so the translation of a
 * pack-tailed function is one concrete function per instance the case creates.
 * each instance below carries the element types of its call site in order, and
 * `va.len` is the constant that instance folds last. */

static inline int16_t as_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold_pack_0(uint64_t h) {
    return mix_u64(h, UINT64_C(0));
}

static uint64_t fold_pack_i8(uint64_t h, uint8_t x0, int16_t x1, uint32_t x2,
                             int64_t x3, uint8_t x4, uint16_t x5, int32_t x6,
                             uint64_t x7) {
    uint64_t a = h;
    a = mix_u64(a, (uint64_t)x0);
    a = mix_u64(a, (uint64_t)(int64_t)x1);
    a = mix_u64(a, (uint64_t)x2);
    a = mix_u64(a, (uint64_t)x3);
    a = mix_u64(a, (uint64_t)x4);
    a = mix_u64(a, (uint64_t)x5);
    a = mix_u64(a, (uint64_t)(int64_t)x6);
    a = mix_u64(a, x7);
    return mix_u64(a, UINT64_C(8));
}

static uint64_t fold_pack_u12(uint64_t h, uint64_t x0, uint64_t x1, uint64_t x2,
                              uint64_t x3, uint64_t x4, uint64_t x5, uint64_t x6,
                              uint64_t x7, uint64_t x8, uint64_t x9, uint64_t x10,
                              uint64_t x11) {
    uint64_t a = h;
    a = mix_u64(a, x0);
    a = mix_u64(a, x1);
    a = mix_u64(a, x2);
    a = mix_u64(a, x3);
    a = mix_u64(a, x4);
    a = mix_u64(a, x5);
    a = mix_u64(a, x6);
    a = mix_u64(a, x7);
    a = mix_u64(a, x8);
    a = mix_u64(a, x9);
    a = mix_u64(a, x10);
    a = mix_u64(a, x11);
    return mix_u64(a, UINT64_C(12));
}

static uint64_t fold_pack_f4(uint64_t h, float x0, double x1, float x2, double x3) {
    uint64_t a = h;
    a = mix_f32(a, x0);
    a = mix_f64(a, x1);
    a = mix_f32(a, x2);
    a = mix_f64(a, x3);
    return mix_u64(a, UINT64_C(4));
}

static uint64_t fold_pack_m6(uint64_t h, uint64_t x0, float x1, uint32_t x2,
                             double x3, int16_t x4, uint64_t x5) {
    uint64_t a = h;
    a = mix_u64(a, x0);
    a = mix_f32(a, x1);
    a = mix_u64(a, (uint64_t)x2);
    a = mix_f64(a, x3);
    a = mix_u64(a, (uint64_t)(int64_t)x4);
    a = mix_u64(a, x5);
    return mix_u64(a, UINT64_C(6));
}

static uint64_t fold_pack_u1(uint64_t h, uint64_t x0) {
    uint64_t a = mix_u64(h, x0);
    return mix_u64(a, UINT64_C(1));
}

static uint64_t fold_pack_f1(uint64_t h, float x0) {
    uint64_t a = mix_f32(h, x0);
    return mix_u64(a, UINT64_C(1));
}

static uint64_t fold_pack_m3(uint64_t h, uint64_t x0, float x1, double x2) {
    uint64_t a = h;
    a = mix_u64(a, x0);
    a = mix_f32(a, x1);
    a = mix_f64(a, x2);
    return mix_u64(a, UINT64_C(3));
}

static uint64_t bias_pack_i4(uint64_t h, uint64_t base, uint8_t x0, uint32_t x1,
                             uint64_t x2, uint16_t x3) {
    uint64_t a = mix_u64(h, base);
    a = mix_u64(a, (uint64_t)((uint64_t)x0 + base));
    a = mix_u64(a, (uint64_t)((uint64_t)x1 + base));
    a = mix_u64(a, (uint64_t)(x2 + base));
    a = mix_u64(a, (uint64_t)((uint64_t)x3 + base));
    return mix_u64(a, UINT64_C(4));
}

static uint64_t bias_pack_m3(uint64_t h, uint64_t base, float x0, uint64_t x1, double x2) {
    uint64_t a = mix_u64(h, base);
    a = mix_f32(a, x0);
    a = mix_u64(a, (uint64_t)(x1 + base));
    a = mix_f64(a, x2);
    return mix_u64(a, UINT64_C(3));
}

static uint64_t fold_pack_i4(uint64_t h, uint8_t x0, int16_t x1, uint32_t x2, int64_t x3) {
    uint64_t a = h;
    a = mix_u64(a, (uint64_t)x0);
    a = mix_u64(a, (uint64_t)(int64_t)x1);
    a = mix_u64(a, (uint64_t)x2);
    a = mix_u64(a, (uint64_t)x3);
    return mix_u64(a, UINT64_C(4));
}

static uint64_t fwd_pack_0(uint64_t h) { return fold_pack_0(h); }

static uint64_t fwd_pack_i4(uint64_t h, uint8_t x0, int16_t x1, uint32_t x2, int64_t x3) {
    return fold_pack_i4(h, x0, x1, x2, x3);
}

static uint64_t fwd_pack_m3(uint64_t h, uint64_t x0, float x1, double x2) {
    return fold_pack_m3(h, x0, x1, x2);
}

static uint64_t fwd2_pack_0(uint64_t h) { return fwd_pack_0(h); }

static uint64_t fwd2_pack_i4(uint64_t h, uint8_t x0, int16_t x1, uint32_t x2, int64_t x3) {
    return fwd_pack_i4(h, x0, x1, x2, x3);
}

static uint64_t fwd2_pack_m3(uint64_t h, uint64_t x0, float x1, double x2) {
    return fwd_pack_m3(h, x0, x1, x2);
}

static uint64_t fwd_bias_i4(uint64_t h, uint64_t base, uint8_t x0, uint32_t x1,
                            uint64_t x2, uint16_t x3) {
    return bias_pack_i4(h, (uint64_t)(base * UINT64_C(3)), x0, x1, x2, x3);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[12];
    for (uint64_t i = 0; i < UINT64_C(12); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    h = fold_pack_0(h);
    h = fwd_pack_0(h);
    h = fwd2_pack_0(h);

    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t d = (uint64_t)(v[k] + seed);

        const uint8_t e8 = (uint8_t)d;
        const int16_t e16 = as_i16((uint16_t)d);
        const uint32_t e32 = (uint32_t)d;
        const int64_t e64 = as_i64(d);
        const uint8_t g8 = (uint8_t)(d >> 7);
        const uint16_t g16 = (uint16_t)(d >> 3);
        const int32_t g32 = as_i32((uint32_t)(d >> 11));
        const float p32 = (float)(d & UINT64_C(4095)) / 8.0f;
        const float q32 = (float)(d & UINT64_C(255)) - 128.0f;
        const float r32 = (float)(d & UINT64_C(1023)) / 4.0f;
        const float s32 = (float)(d & UINT64_C(511)) / 2.0f;
        const double p64 = (double)(d & UINT64_C(65535)) / 64.0;
        const double q64 = (double)(d & UINT64_C(262143)) / 256.0;
        const double r64 = (double)(d & UINT64_C(8191)) / 2.0;

        h = fold_pack_i8(h, e8, e16, e32, e64, g8, g16, g32, (uint64_t)(d * UINT64_C(3)));

        h = fold_pack_u12(h, (uint64_t)(v[0] + d), v[1], v[2], v[3], v[4], v[5],
                          v[6], v[7], v[8], v[9], v[10], v[11]);

        h = fold_pack_f4(h, p32, p64, q32, q64);

        h = fold_pack_m6(h, d, p32, e32, p64, e16, (uint64_t)(d * UINT64_C(5)));

        h = bias_pack_i4(h, d, e8, e32, (uint64_t)(d * UINT64_C(7)), g16);
        h = bias_pack_m3(h, d, r32, d, r64);

        h = fwd_pack_i4(h, e8, e16, e32, e64);
        h = fwd2_pack_i4(h, e8, e16, e32, e64);
        h = fwd2_pack_m3(h, d, p32, p64);
        h = fwd_bias_i4(h, d, e8, e32, (uint64_t)(d * UINT64_C(7)), g16);

        h = fold_pack_u1(h, d);
        h = fold_pack_f1(h, s32);
    }

    return h;
}
