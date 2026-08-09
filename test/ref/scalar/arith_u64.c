#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(18446744073709551600) + s);
    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a + (uint64_t)(i * UINT64_C(7) + UINT64_C(1)));
        h = mix_u64(h, a);
        a = (uint64_t)(a - (uint64_t)(i * UINT64_C(3) + UINT64_C(2)));
        h = mix_u64(h, a);
    }

    uint64_t b = s;
    for (uint64_t j = 0; j < UINT64_C(8); j = (uint64_t)(j + UINT64_C(1))) {
        b = (uint64_t)(b - (uint64_t)(j * UINT64_C(1311768467294899695) + UINT64_C(1)));
        h = mix_u64(h, b);
    }

    const uint64_t n = (uint64_t)(UINT64_C(0) - a);
    h = mix_u64(h, n);
    h = mix_u64(h, (uint64_t)(UINT64_C(0) - n));
    h = mix_u64(h, (uint64_t)(UINT64_C(0) - s));
    h = mix_u64(h, (uint64_t)(UINT64_C(9223372036854775807) + a));
    h = mix_u64(h, (uint64_t)(UINT64_C(9223372036854775808) - a));
    h = mix_u64(h, (uint64_t)(UINT64_C(18446744073709551615) + a));
    h = mix_u64(h, (uint64_t)(a + b));
    h = mix_u64(h, (uint64_t)(a - b));
    h = mix_u64(h, (uint64_t)(b - a));
    return h;
}
