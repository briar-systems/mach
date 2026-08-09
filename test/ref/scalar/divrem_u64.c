#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t d = (uint64_t)(UINT64_C(18446744073709551615) - s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(131) + UINT64_C(3) + s);
        const uint64_t q = (uint64_t)(d / v);
        const uint64_t r = (uint64_t)(d % v);
        h = mix_u64(h, q);
        h = mix_u64(h, r);
        h = mix_u64(h, (uint64_t)((uint64_t)(v * q) + r));
        d = (uint64_t)(d - v);
    }

    const uint64_t big = (uint64_t)(UINT64_C(18446744073709551613) + s);
    const uint64_t small = (uint64_t)(UINT64_C(3) + s);
    const uint64_t q0 = (uint64_t)(big / small);
    const uint64_t r0 = (uint64_t)(big % small);
    h = mix_u64(h, q0);
    h = mix_u64(h, r0);
    h = mix_u64(h, (uint64_t)((uint64_t)(small * q0) + r0));

    const uint64_t q1 = (uint64_t)(small / big);
    const uint64_t r1 = (uint64_t)(small % big);
    h = mix_u64(h, q1);
    h = mix_u64(h, r1);
    h = mix_u64(h, (uint64_t)((uint64_t)(big * q1) + r1));
    return h;
}
