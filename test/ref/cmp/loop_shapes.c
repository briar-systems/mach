#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t acc = s;
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        acc = (uint32_t)(acc + (uint32_t)(i * UINT32_C(3) + UINT32_C(1)));
        h = mix_u32(h, acc);
    }

    uint32_t w = (uint32_t)(UINT32_C(98) + s);
    while (w > UINT32_C(0)) {
        w = (uint32_t)(w - UINT32_C(7));
        h = mix_u32(h, w);
    }

    for (uint32_t oi = 0; oi < UINT32_C(6); oi = (uint32_t)(oi + UINT32_C(1))) {
        for (uint32_t ij = 0; ij < UINT32_C(6); ) {
            if (ij == UINT32_C(3)) { ij = (uint32_t)(ij + UINT32_C(1)); continue; }
            h = mix_u32(h, (uint32_t)(oi * UINT32_C(6) + ij + s));
            ij = (uint32_t)(ij + UINT32_C(1));
        }
    }

    uint32_t e = s;
    for (;;) {
        if (!(e < UINT32_C(1000000))) { break; }
        h = mix_u32(h, e);
        if (e == (uint32_t)(UINT32_C(9) + s)) { break; }
        e = (uint32_t)(e + UINT32_C(1));
    }

    uint32_t p0 = (uint32_t)(UINT32_C(1) + s);
    uint32_t p1 = UINT32_C(1);
    for (uint32_t f = 0; f < UINT32_C(20); f = (uint32_t)(f + UINT32_C(1))) {
        const uint32_t nx = (uint32_t)(p0 + p1);
        h = mix_u32(h, nx);
        p0 = p1;
        p1 = nx;
    }

    return h;
}
