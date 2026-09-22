#include "corpus.h"

/* each probe gathers `lanes` consecutive elements starting at `i` (or at the
 * listed offsets for the permuted, stride-two and mixed shapes) and folds them
 * in lane order. the vector load and the lane path read the same bytes. */

static uint64_t fold_f32s(uint64_t h, const float *p, const unsigned *off, unsigned lanes) {
    for (unsigned k = 0; k < lanes; k++) { h = mix_f32(h, p[off[k]]); }
    return h;
}

static uint64_t gather_f32(uint64_t h, const float *p, uint64_t i) {
    unsigned off[4] = { (unsigned)i, (unsigned)i + 1u, (unsigned)i + 2u, (unsigned)i + 3u };
    return fold_f32s(h, p, off, 4u);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t at = seed & 3u;
    {
        float f[16], d[16];
        for (unsigned k = 0; k < 16u; k++) {
            f[k] = ((float)k - 5.5f) * 1.25f;
            d[k] = (float)k * -0.5f + (float)seed;
        }
        h = gather_f32(h, f, at);
        h = gather_f32(h, f, at + 4u);
        h = gather_f32(h, f, 2u);
        {
            float a = 1.5f + (float)seed, b = -2.25f;
            float arr[8] = { a, b, a + b, a - b, a * b, b - a, a + a, b + b };
            h = gather_f32(h, arr, at);
        }
        {
            unsigned off[4] = { (unsigned)at, (unsigned)at + 2u, (unsigned)at + 1u, (unsigned)at + 3u };
            h = fold_f32s(h, f, off, 4u);
        }
        {
            unsigned off[4] = { (unsigned)at, (unsigned)at + 2u, (unsigned)at + 4u, (unsigned)at + 6u };
            h = fold_f32s(h, f, off, 4u);
        }
        h = mix_f32(h, f[at]);
        h = mix_f32(h, f[at + 1u]);
        h = mix_f32(h, f[at + 2u]);
        h = mix_f32(h, d[at + 3u]);
    }
    {
        double f[8];
        for (unsigned k = 0; k < 8u; k++) { f[k] = ((double)k - 3.5) * 0.75 + (double)seed; }
        h = mix_f64(h, f[at]);
        h = mix_f64(h, f[at + 1u]);
        h = mix_f64(h, f[at + 2u]);
        h = mix_f64(h, f[at + 3u]);
    }
    {
        int32_t a[16];
        uint32_t u[16];
        for (unsigned k = 0; k < 16u; k++) {
            a[k] = (int32_t)(((int64_t)k - 8) * 1000003 + (int64_t)(int32_t)(uint32_t)seed);
            u[k] = (uint32_t)((uint64_t)k * UINT64_C(2654435761) + seed);
        }
        for (unsigned k = 0; k < 4u; k++) { h = mix_i32(h, a[at + k]); }
        for (unsigned k = 0; k < 4u; k++) { h = mix_u32(h, u[at + 1u + k]); }
    }
    {
        int64_t a[8];
        for (unsigned k = 0; k < 8u; k++) {
            a[k] = (int64_t)(((uint64_t)((int64_t)k - 4) * UINT64_C(1000000007)) + seed);
        }
        h = mix_i64(h, a[at]);
        h = mix_i64(h, a[at + 1u]);
    }
    {
        int16_t a[16];
        for (unsigned k = 0; k < 16u; k++) {
            a[k] = (int16_t)(uint16_t)(((uint32_t)((int32_t)k - 8) * 2001u) + (uint16_t)seed);
        }
        for (unsigned k = 0; k < 8u; k++) { h = mix_i16(h, a[at + k]); }
    }
    {
        uint8_t a[32];
        for (unsigned k = 0; k < 32u; k++) { a[k] = (uint8_t)(k * 37u + seed); }
        for (unsigned k = 0; k < 16u; k++) { h = mix_u8(h, a[at + k]); }
        for (unsigned k = 0; k < 16u; k++) { h = mix_u8(h, a[at + 16u + k]); }
    }
    return h;
}
