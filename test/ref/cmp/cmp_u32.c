#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t UMAX = UINT32_C(4294967295);
    const uint32_t HALF = UINT32_C(2147483648);
    const uint32_t HMAX = UINT32_C(2147483647);

    const uint32_t av[7] = { 0, 0, UMAX, HMAX, HALF, UMAX, HALF };
    const uint32_t bv[7] = { 0, UMAX, 0, HALF, HMAX, UMAX, HALF };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const uint32_t a = (uint32_t)(av[k] + s);
        const uint32_t b = bv[k];
        h = mix_u8(h, (uint8_t)(a == b));
        h = mix_u8(h, (uint8_t)(a != b));
        h = mix_u8(h, (uint8_t)(a < b));
        h = mix_u8(h, (uint8_t)(a > b));
        h = mix_u8(h, (uint8_t)(a <= b));
        h = mix_u8(h, (uint8_t)(a >= b));
    }
    return h;
}
