#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    uint8_t a = (uint8_t)(UINT8_C(240) + s);
    for (uint8_t i = 0; i < UINT8_C(32); i = (uint8_t)(i + UINT8_C(1))) {
        a = (uint8_t)(a + (uint8_t)(i * UINT8_C(7) + UINT8_C(1)));
        h = mix_u8(h, a);
        a = (uint8_t)(a - (uint8_t)(i * UINT8_C(3) + UINT8_C(2)));
        h = mix_u8(h, a);
    }

    uint8_t b = s;
    for (uint8_t j = 0; j < UINT8_C(8); j = (uint8_t)(j + UINT8_C(1))) {
        b = (uint8_t)(b - (uint8_t)(j * UINT8_C(181) + UINT8_C(1)));
        h = mix_u8(h, b);
    }

    const uint8_t n = (uint8_t)(UINT8_C(0) - a);
    h = mix_u8(h, n);
    h = mix_u8(h, (uint8_t)(UINT8_C(0) - n));
    h = mix_u8(h, (uint8_t)(UINT8_C(0) - s));
    h = mix_u8(h, (uint8_t)(UINT8_C(127) + a));
    h = mix_u8(h, (uint8_t)(UINT8_C(128) - a));
    h = mix_u8(h, (uint8_t)(UINT8_C(255) + a));
    h = mix_u8(h, (uint8_t)(a + b));
    h = mix_u8(h, (uint8_t)(a - b));
    h = mix_u8(h, (uint8_t)(b - a));
    return h;
}
