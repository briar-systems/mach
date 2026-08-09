#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    const uint64_t UMAX = UINT64_C(18446744073709551615);
    const uint64_t HALF = UINT64_C(9223372036854775808);
    const uint64_t HMAX = UINT64_C(9223372036854775807);

    const uint64_t av[7] = { 0, 0, UMAX, HMAX, HALF, UMAX, HALF };
    const uint64_t bv[7] = { 0, UMAX, 0, HALF, HMAX, UMAX, HALF };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t a = (uint64_t)(av[k] + s);
        const uint64_t b = bv[k];
        h = mix_u8(h, (uint8_t)(a == b));
        h = mix_u8(h, (uint8_t)(a != b));
        h = mix_u8(h, (uint8_t)(a < b));
        h = mix_u8(h, (uint8_t)(a > b));
        h = mix_u8(h, (uint8_t)(a <= b));
        h = mix_u8(h, (uint8_t)(a >= b));
    }
    return h;
}
