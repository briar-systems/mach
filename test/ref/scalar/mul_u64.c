#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(11400714819323198485) + s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a * (uint64_t)(UINT64_C(2) * i + UINT64_C(3)));
        h = mix_u64(h, a);
    }

    uint64_t b = (uint64_t)(UINT64_C(18446744073709551553) + s);
    h = mix_u64(h, (uint64_t)(b * b));
    h = mix_u64(h, (uint64_t)(b * UINT64_C(18446744073709551615)));
    h = mix_u64(h, (uint64_t)(a * b));
    h = mix_u64(h, (uint64_t)(b * a));

    uint64_t c = (uint64_t)(UINT64_C(4294967311) + s);
    h = mix_u64(h, (uint64_t)(c * c));
    h = mix_u64(h, (uint64_t)(c * UINT64_C(4294967295)));
    return h;
}
