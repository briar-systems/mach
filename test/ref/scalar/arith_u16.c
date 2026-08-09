#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    uint16_t a = (uint16_t)(UINT16_C(65520) + s);
    for (uint16_t i = 0; i < UINT16_C(32); i = (uint16_t)(i + UINT16_C(1))) {
        a = (uint16_t)(a + (uint16_t)(i * UINT16_C(7) + UINT16_C(1)));
        h = mix_u16(h, a);
        a = (uint16_t)(a - (uint16_t)(i * UINT16_C(3) + UINT16_C(2)));
        h = mix_u16(h, a);
    }

    uint16_t b = s;
    for (uint16_t j = 0; j < UINT16_C(8); j = (uint16_t)(j + UINT16_C(1))) {
        b = (uint16_t)(b - (uint16_t)(j * UINT16_C(43981) + UINT16_C(1)));
        h = mix_u16(h, b);
    }

    const uint16_t n = (uint16_t)(UINT16_C(0) - a);
    h = mix_u16(h, n);
    h = mix_u16(h, (uint16_t)(UINT16_C(0) - n));
    h = mix_u16(h, (uint16_t)(UINT16_C(0) - s));
    h = mix_u16(h, (uint16_t)(UINT16_C(32767) + a));
    h = mix_u16(h, (uint16_t)(UINT16_C(32768) - a));
    h = mix_u16(h, (uint16_t)(UINT16_C(65535) + a));
    h = mix_u16(h, (uint16_t)(a + b));
    h = mix_u16(h, (uint16_t)(a - b));
    h = mix_u16(h, (uint16_t)(b - a));
    return h;
}
