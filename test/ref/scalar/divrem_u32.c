#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t d = (uint32_t)(UINT32_C(4294967295) - s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(131) + UINT32_C(3) + s);
        const uint32_t q = (uint32_t)(d / v);
        const uint32_t r = (uint32_t)(d % v);
        h = mix_u32(h, q);
        h = mix_u32(h, r);
        h = mix_u32(h, (uint32_t)((uint32_t)(v * q) + r));
        d = (uint32_t)(d - v);
    }

    const uint32_t big = (uint32_t)(UINT32_C(4294967293) + s);
    const uint32_t small = (uint32_t)(UINT32_C(3) + s);
    const uint32_t q0 = (uint32_t)(big / small);
    const uint32_t r0 = (uint32_t)(big % small);
    h = mix_u32(h, q0);
    h = mix_u32(h, r0);
    h = mix_u32(h, (uint32_t)((uint32_t)(small * q0) + r0));

    const uint32_t q1 = (uint32_t)(small / big);
    const uint32_t r1 = (uint32_t)(small % big);
    h = mix_u32(h, q1);
    h = mix_u32(h, r1);
    h = mix_u32(h, (uint32_t)((uint32_t)(big * q1) + r1));
    return h;
}
