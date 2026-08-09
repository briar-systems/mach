#include "corpus.h"

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static float as_f32(uint64_t v) {
    return ((float)(uint32_t)(v & UINT64_C(4095)) - 2048.0f) / 8.0f;
}

static double as_f64(uint64_t v) {
    return ((double)(uint32_t)(v & UINT64_C(1048575)) - 524288.0) / 64.0;
}

static uint64_t takef16(float a0, double a1, float a2, double a3,
                        float a4, double a5, float a6, double a7,
                        float a8, double a9, float a10, double a11,
                        float a12, double a13, float a14, double a15) {
    uint64_t h = fold_init();
    h = mix_f32(h, a0);
    h = mix_f64(h, a1);
    h = mix_f32(h, a2);
    h = mix_f64(h, a3);
    h = mix_f32(h, a4);
    h = mix_f64(h, a5);
    h = mix_f32(h, a6);
    h = mix_f64(h, a7);
    h = mix_f32(h, a8);
    h = mix_f64(h, a9);
    h = mix_f32(h, a10);
    h = mix_f64(h, a11);
    h = mix_f32(h, a12);
    h = mix_f64(h, a13);
    h = mix_f32(h, a14);
    h = mix_f64(h, a15);
    h = mix_f32(h, a0 + a2 * a4 - a6);
    h = mix_f64(h, a1 + a3 * a5 - a7);
    h = mix_f32(h, a8 - a10 + a12 * a14);
    h = mix_f64(h, a9 - a11 + a13 * a15);
    return h;
}

static uint64_t takef16s(float b0, float b1, float b2, float b3, float b4,
                         float b5, float b6, float b7, float b8, float b9,
                         float b10, float b11, float b12, float b13, float b14,
                         float b15) {
    uint64_t h = fold_init();
    h = mix_f32(h, b0);
    h = mix_f32(h, b1);
    h = mix_f32(h, b2);
    h = mix_f32(h, b3);
    h = mix_f32(h, b4);
    h = mix_f32(h, b5);
    h = mix_f32(h, b6);
    h = mix_f32(h, b7);
    h = mix_f32(h, b8);
    h = mix_f32(h, b9);
    h = mix_f32(h, b10);
    h = mix_f32(h, b11);
    h = mix_f32(h, b12);
    h = mix_f32(h, b13);
    h = mix_f32(h, b14);
    h = mix_f32(h, b15);
    h = mix_f32(h, b0 + b15);
    h = mix_f32(h, b7 - b12);
    h = mix_f32(h, b3 * b14);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[20];
    for (uint64_t i = 0; i < UINT64_C(20); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    float f[20];
    double d[20];
    for (uint64_t i = 0; i < UINT64_C(20); i = (uint64_t)(i + UINT64_C(1))) {
        f[i] = as_f32(v[i]);
        d[i] = as_f64(v[i]);
    }

    uint64_t r = 0;
    for (uint64_t pass = 0; pass < UINT64_C(3); pass = (uint64_t)(pass + UINT64_C(1))) {
        const float bias = (float)(uint32_t)(pass & UINT64_C(3)) / 4.0f;

        r = takef16(f[0] + bias, d[1], f[2], d[3], f[4], d[5], f[6], d[7],
                    f[8], d[9], f[10], d[11], f[12], d[13], f[14], d[15]);
        h = mix_u64(h, r);

        r = takef16s(f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8], f[9],
                     f[10], f[11], f[12], f[13], f[14], f[15] + bias);
        h = mix_u64(h, r);
    }

    h = mix_u64(h, takef16(
        as_f32(takef16s(f[19], f[18], f[17], f[16], f[15], f[14], f[13], f[12],
                        f[11], f[10], f[9], f[8], f[7], f[6], f[5], f[4])),
        d[0], f[1], d[2], f[3], d[4], f[5], d[6],
        as_f32(r), d[8], f[9], d[10], f[11], d[12], f[13], d[14]));

    return h;
}
