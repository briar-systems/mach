#include "corpus.h"

static double bits_to_f64(uint64_t b) {
    double f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static float bits_to_f32(uint32_t b) {
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold64(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint64_t pat[12] = {
        UINT64_C(0xFFF0000000000000),
        UINT64_C(0xBFF0000000000000),
        UINT64_C(0x8010000000000000),
        UINT64_C(0x8000000000000001),
        UINT64_C(0x8000000000000000),
        UINT64_C(0x0000000000000000),
        UINT64_C(0x0000000000000001),
        UINT64_C(0x3FF0000000000000),
        UINT64_C(0x7FEFFFFFFFFFFFFF),
        UINT64_C(0x7FF0000000000000),
        UINT64_C(0x7FF8000000000000),
        UINT64_C(0x7FF0000000000001)
    };

    double v[12];
    for (uint64_t k = 0; k < UINT64_C(12); k = (uint64_t)(k + UINT64_C(1))) {
        v[k] = bits_to_f64((uint64_t)(pat[k] + seed));
    }

    for (uint64_t i = 0; i < UINT64_C(12); i = (uint64_t)(i + UINT64_C(1))) {
        for (uint64_t j = 0; j < UINT64_C(12); j = (uint64_t)(j + UINT64_C(1))) {
            const double x = v[i];
            const double y = v[j];

            h = mix_u8(h, (uint8_t)(x == y));
            h = mix_u8(h, (uint8_t)(x != y));
            h = mix_u8(h, (uint8_t)(x < y));
            h = mix_u8(h, (uint8_t)(x <= y));
            h = mix_u8(h, (uint8_t)(x > y));
            h = mix_u8(h, (uint8_t)(x >= y));

            uint8_t c = 0;
            if (x < y)  { c = (uint8_t)(c + UINT32_C(1)); }
            if (x <= y) { c = (uint8_t)(c + UINT32_C(2)); }
            if (x > y)  { c = (uint8_t)(c + UINT32_C(4)); }
            if (x >= y) { c = (uint8_t)(c + UINT32_C(8)); }
            if (x == y) { c = (uint8_t)(c + UINT32_C(16)); }
            if (x != y) { c = (uint8_t)(c + UINT32_C(32)); }
            h = mix_u8(h, c);

            h = mix_u8(h, (uint8_t)((x < y) && (y < x)));
            h = mix_u8(h, (uint8_t)((x < y) || (y < x)));
            h = mix_u8(h, (uint8_t)(!(x < y)));
            h = mix_u8(h, (uint8_t)((x == x) && (y == y)));
            h = mix_u8(h, (uint8_t)((x != x) || (y != y)));
        }
    }

    const uint32_t nat[8] = {
        UINT32_C(0xFF800000),
        UINT32_C(0xBF800000),
        UINT32_C(0x80000000),
        UINT32_C(0x00000000),
        UINT32_C(0x00000001),
        UINT32_C(0x3F800000),
        UINT32_C(0x7F800000),
        UINT32_C(0x7FC00000)
    };

    float w[8];
    for (uint32_t n = 0; n < UINT32_C(8); n = (uint32_t)(n + UINT32_C(1))) {
        w[n] = bits_to_f32((uint32_t)(nat[n] + s));
    }

    for (uint64_t a = 0; a < UINT64_C(12); a = (uint64_t)(a + UINT64_C(1))) {
        for (uint64_t b = 0; b < UINT64_C(8); b = (uint64_t)(b + UINT64_C(1))) {
            const double x = v[a];
            const double y = (double)w[b];
            h = mix_u8(h, (uint8_t)(x == y));
            h = mix_u8(h, (uint8_t)(x != y));
            h = mix_u8(h, (uint8_t)(x < y));
            h = mix_u8(h, (uint8_t)(x <= y));
            h = mix_u8(h, (uint8_t)(y < x));
            h = mix_u8(h, (uint8_t)(y <= x));
        }
    }

    double lo = v[0];
    double hi = v[0];
    for (uint64_t m = 1; m < UINT64_C(12); m = (uint64_t)(m + UINT64_C(1))) {
        if (v[m] < lo) { lo = v[m]; }
        if (v[m] > hi) { hi = v[m]; }
    }
    h = fold64(h, lo);
    h = fold64(h, hi);

    uint32_t trues = 0;
    for (uint64_t p = 0; p < UINT64_C(12); p = (uint64_t)(p + UINT64_C(1))) {
        for (uint64_t q = 0; q < UINT64_C(12); q = (uint64_t)(q + UINT64_C(1))) {
            trues = (uint32_t)(trues + (uint32_t)(v[p] <  v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] <= v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] == v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] != v[q]));
        }
    }
    h = mix_u32(h, trues);
    return h;
}
