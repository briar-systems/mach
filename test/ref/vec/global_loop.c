#include "corpus.h"

/* every probe lane by lane: wrapping unsigned sums of the masked or combined
 * input, and a float sum of products that are exact multiples of 1/16, so
 * every rounding matches. every result lane folds in order. */

static const uint64_t K4[4] = {3, 5, 7, 9};
static const uint64_t K8[8] = {0xff, 0xf0f, 0x3333, 0x5555, 0xaaaa, 0xcccc, 0xf0f0, 0xffff};
static const uint64_t K16[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static const uint64_t K32[32] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x100,
    0x101, 0x202, 0x303, 0x404, 0x505, 0x606, 0x707, 0x808, 0x909, 0xa0a, 0xb0b, 0xc0c, 0xd0d, 0xe0e, 0xf0f, 0x1010
};
static const double F16[16] = {
    0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, -0.5, -1.0, -1.5, -2.0, -2.5, -3.0, -3.5, -4.0
};

static uint64_t next(uint64_t *s) {
    *s = *s * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return *s >> 29;
}

static uint64_t fold_lanes(uint64_t h, const uint64_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u64(h, p[k]); }
    return h;
}

static uint64_t masked(uint64_t h, const uint64_t *buf, const uint64_t *key, unsigned w, uint64_t n) {
    uint64_t c[32] = {0};
    for (uint64_t i = 0; i < n; i++) {
        for (unsigned l = 0; l < w; l++) { c[l] += buf[i * w + l] & key[l]; }
    }
    return fold_lanes(h, c, w);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t s = seed + UINT64_C(0x9E3779B97F4A7C15);
    uint64_t buf[128];
    const uint64_t n = 3 + (seed & 1);

    for (unsigned k = 0; k < 128; k++) { buf[k] = next(&s); }
    h = masked(h, buf, K4, 4, n);
    h = masked(h, buf, K8, 8, n);
    h = masked(h, buf, K16, 16, n);
    h = masked(h, buf, K32, 32, n);
    {
        uint64_t c[16] = {0};
        for (uint64_t i = 0; i < n; i++) {
            for (unsigned l = 0; l < 16; l++) { c[l] = (c[l] + K16[l]) ^ buf[i * 16 + l]; }
        }
        h = fold_lanes(h, c, 16);
    }

    double x[64];
    for (unsigned k = 0; k < 64; k++) { x[k] = (double)((int64_t)(next(&s) % 2001u) - 1000) / 8.0; }
    {
        double c[16];
        for (unsigned l = 0; l < 16; l++) { c[l] = F16[l] - F16[l]; }
        for (uint64_t i = 0; i < n; i++) {
            for (unsigned l = 0; l < 16; l++) { c[l] = c[l] + x[i * 16 + l] * F16[l]; }
        }
        uint64_t bits[16];
        memcpy(bits, c, sizeof bits);
        h = fold_lanes(h, bits, 16);
    }
    return h;
}
