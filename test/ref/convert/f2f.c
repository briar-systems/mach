#include "corpus.h"
#include <string.h>

static float f2f_narrow(uint64_t b) {
    double d;
    memcpy(&d, &b, sizeof d);
    return (float)d;
}

static double f2f_widen(uint32_t b) {
    float f;
    memcpy(&f, &b, sizeof f);
    return (double)f;
}

static uint32_t f2f_f32_bits(float f) {
    uint32_t b;
    memcpy(&b, &f, sizeof b);
    return b;
}

static uint64_t f2f_f64_bits(double d) {
    uint64_t b;
    memcpy(&b, &d, sizeof b);
    return b;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const double z = (double)seed;
    const float zf = (float)seed;

    const double d1 = 1.0000000596046448 + z;
    const float f1 = (float)d1;
    const double d1b = (double)f1;
    h = mix_f64(h, d1);
    h = mix_f32(h, f1);
    h = mix_f64(h, d1b);

    const double d2 = 0.1 + z;
    const float f2 = (float)d2;
    const double d2b = (double)f2;
    h = mix_f64(h, d2);
    h = mix_f32(h, f2);
    h = mix_f64(h, d2b);

    const double d3 = 16777217.0 + z;
    const float f3 = (float)d3;
    h = mix_f64(h, d3);
    h = mix_f32(h, f3);

    const float f4 = 3.4028235e38f + zf;
    const double d4 = (double)f4;
    h = mix_f32(h, f4);
    h = mix_f64(h, d4);

    /* the binary16 NaN 0x7F55 widened exactly, then narrowed */
    const uint32_t s32 = (uint32_t)seed;
    const uint32_t n[3] = {
        f2f_f32_bits(f2f_narrow(UINT64_C(0x7FF4000000000001) + seed)),
        f2f_f32_bits(f2f_narrow(UINT64_C(0xFFF8040000000000) + seed)),
        f2f_f32_bits(f2f_narrow(UINT64_C(0x7FF0000020000000) + seed)),
    };
    const uint64_t w = f2f_f64_bits(f2f_widen(UINT32_C(0x7FA00001) + s32));
    const uint32_t hf = f2f_f32_bits(f2f_narrow(UINT64_C(0x7FFD540000000000) + seed));
    h = mix_u32(h, n[0]);
    h = mix_u32(h, n[1]);
    h = mix_u32(h, n[2]);
    h = mix_u64(h, w);
    h = mix_u32(h, hf);

    h = mix_u32(h, (uint32_t)(n[0] + s32));
    h = mix_u32(h, (uint32_t)(n[1] + s32));
    h = mix_u32(h, (uint32_t)(n[2] + s32));
    h = mix_u64(h, w + seed);
    h = mix_u32(h, (uint32_t)(hf + s32));

    return h;
}
