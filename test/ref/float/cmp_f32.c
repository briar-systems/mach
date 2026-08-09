#include "corpus.h"

static float bits_to_f32(uint32_t b) {
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold32(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t pat[12] = {
        UINT32_C(0xFF800000),
        UINT32_C(0xBF800000),
        UINT32_C(0x80800000),
        UINT32_C(0x80000001),
        UINT32_C(0x80000000),
        UINT32_C(0x00000000),
        UINT32_C(0x00000001),
        UINT32_C(0x3F800000),
        UINT32_C(0x7F7FFFFF),
        UINT32_C(0x7F800000),
        UINT32_C(0x7FC00000),
        UINT32_C(0x7F800001)
    };

    float v[12];
    for (uint32_t k = 0; k < UINT32_C(12); k = (uint32_t)(k + UINT32_C(1))) {
        v[k] = bits_to_f32((uint32_t)(pat[k] + s));
    }

    for (uint32_t i = 0; i < UINT32_C(12); i = (uint32_t)(i + UINT32_C(1))) {
        for (uint32_t j = 0; j < UINT32_C(12); j = (uint32_t)(j + UINT32_C(1))) {
            const float x = v[i];
            const float y = v[j];

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

    float lo = v[0];
    float hi = v[0];
    for (uint32_t m = 1; m < UINT32_C(12); m = (uint32_t)(m + UINT32_C(1))) {
        if (v[m] < lo) { lo = v[m]; }
        if (v[m] > hi) { hi = v[m]; }
    }
    h = fold32(h, lo);
    h = fold32(h, hi);

    uint32_t trues = 0;
    for (uint32_t p = 0; p < UINT32_C(12); p = (uint32_t)(p + UINT32_C(1))) {
        for (uint32_t q = 0; q < UINT32_C(12); q = (uint32_t)(q + UINT32_C(1))) {
            trues = (uint32_t)(trues + (uint32_t)(v[p] <  v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] <= v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] == v[q]));
            trues = (uint32_t)(trues + (uint32_t)(v[p] != v[q]));
        }
    }
    h = mix_u32(h, trues);
    return h;
}
