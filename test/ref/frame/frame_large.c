#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a[640] = { 0 };
    uint32_t b[512] = { 0 };
    uint8_t c[1024] = { 0 };

    h = mix_u64(h, a[0]);
    h = mix_u64(h, a[639]);
    h = mix_u32(h, b[0]);
    h = mix_u32(h, b[511]);
    h = mix_u8(h, c[0]);
    h = mix_u8(h, c[1023]);

    for (uint64_t i = 0; i < UINT64_C(640); i = (uint64_t)(i + UINT64_C(1))) {
        a[i] = (uint64_t)(((uint64_t)(i + s) * UINT64_C(6364136223846793005)) ^ (uint64_t)(i << 17));
    }

    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        b[i] = (uint32_t)((uint64_t)((uint64_t)(i * UINT64_C(2654435761)) + s) ^ (uint64_t)(i >> 3));
    }

    for (uint64_t i = 0; i < UINT64_C(1024); i = (uint64_t)(i + UINT64_C(1))) {
        c[i] = (uint8_t)((uint64_t)((uint64_t)(i * UINT64_C(131)) + s) ^ (uint64_t)(i >> 2));
    }

    for (uint64_t j = 0; j < UINT64_C(64); j = (uint64_t)(j + UINT64_C(1))) {
        h = mix_u64(h, a[(uint64_t)((uint64_t)(j * UINT64_C(9) + s) % UINT64_C(640))]);
        h = mix_u32(h, b[(uint64_t)((uint64_t)(j * UINT64_C(13) + s) % UINT64_C(512))]);
        h = mix_u8(h, c[(uint64_t)((uint64_t)(j * UINT64_C(29) + s) % UINT64_C(1024))]);
    }

    a[(uint64_t)(UINT64_C(639) - (s & UINT64_C(1)))] = (uint64_t)(a[639] + UINT64_C(1));
    b[(uint64_t)(UINT64_C(511) - (s & UINT64_C(3)))] = (uint32_t)(b[511] + UINT32_C(7));
    c[(uint64_t)(UINT64_C(1023) - (s & UINT64_C(7)))] = (uint8_t)(c[1023] + UINT32_C(11));

    h = mix_u64(h, a[638]);
    h = mix_u64(h, a[639]);
    h = mix_u32(h, b[508]);
    h = mix_u32(h, b[511]);
    h = mix_u8(h, c[1016]);
    h = mix_u8(h, c[1023]);
    return h;
}
