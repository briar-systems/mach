#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(4294967280) + s);
    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a + (uint32_t)(i * UINT32_C(7) + UINT32_C(1)));
        h = mix_u32(h, a);
        a = (uint32_t)(a - (uint32_t)(i * UINT32_C(3) + UINT32_C(2)));
        h = mix_u32(h, a);
    }

    uint32_t b = s;
    for (uint32_t j = 0; j < UINT32_C(8); j = (uint32_t)(j + UINT32_C(1))) {
        b = (uint32_t)(b - (uint32_t)(j * UINT32_C(305419896) + UINT32_C(1)));
        h = mix_u32(h, b);
    }

    const uint32_t n = (uint32_t)(UINT32_C(0) - a);
    h = mix_u32(h, n);
    h = mix_u32(h, (uint32_t)(UINT32_C(0) - n));
    h = mix_u32(h, (uint32_t)(UINT32_C(0) - s));
    h = mix_u32(h, (uint32_t)(UINT32_C(2147483647) + a));
    h = mix_u32(h, (uint32_t)(UINT32_C(2147483648) - a));
    h = mix_u32(h, (uint32_t)(UINT32_C(4294967295) + a));
    h = mix_u32(h, (uint32_t)(a + b));
    h = mix_u32(h, (uint32_t)(a - b));
    h = mix_u32(h, (uint32_t)(b - a));
    return h;
}
