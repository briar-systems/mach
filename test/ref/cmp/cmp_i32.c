#include "corpus.h"

/* signed mach arithmetic is expressed here as the two's-complement identity on
 * the matching unsigned type: cast to unsigned, add mod 2^32, bit-reinterpret
 * back through memcpy. every step is defined by the C standard, so a disagreement
 * always indicts mach, never this file. */
static inline int32_t bitcast_i32(uint32_t v) {
    int32_t r;
    memcpy(&r, &v, sizeof r);
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int32_t s = bitcast_i32((uint32_t)seed);

    const int32_t IMIN = bitcast_i32(UINT32_C(2147483648));
    const int32_t IMAX = INT32_C(2147483647);

    const int32_t av[7] = { 0, 0, -1, IMAX, IMIN, -1, IMIN };
    const int32_t bv[7] = { 0, -1, 0, IMIN, IMAX, -1, IMIN };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const int32_t a = bitcast_i32((uint32_t)((uint32_t)av[k] + (uint32_t)s));
        const int32_t b = bv[k];
        h = mix_u8(h, (uint8_t)(a == b));
        h = mix_u8(h, (uint8_t)(a != b));
        h = mix_u8(h, (uint8_t)(a < b));
        h = mix_u8(h, (uint8_t)(a > b));
        h = mix_u8(h, (uint8_t)(a <= b));
        h = mix_u8(h, (uint8_t)(a >= b));
    }
    return h;
}
