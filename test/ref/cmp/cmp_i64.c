#include "corpus.h"

/* signed mach arithmetic is expressed here as the two's-complement identity on
 * the matching unsigned type: cast to unsigned, add mod 2^64, bit-reinterpret
 * back through memcpy. every step is defined by the C standard, so a disagreement
 * always indicts mach, never this file. */
static inline int64_t bitcast_i64(uint64_t v) {
    int64_t r;
    memcpy(&r, &v, sizeof r);
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = bitcast_i64(seed);

    const int64_t IMIN = bitcast_i64(UINT64_C(9223372036854775808));
    const int64_t IMAX = INT64_C(9223372036854775807);

    const int64_t av[7] = { 0, 0, -1, IMAX, IMIN, -1, IMIN };
    const int64_t bv[7] = { 0, -1, 0, IMIN, IMAX, -1, IMIN };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const int64_t a = bitcast_i64((uint64_t)av[k] + (uint64_t)s);
        const int64_t b = bv[k];
        h = mix_u8(h, (uint8_t)(a == b));
        h = mix_u8(h, (uint8_t)(a != b));
        h = mix_u8(h, (uint8_t)(a < b));
        h = mix_u8(h, (uint8_t)(a > b));
        h = mix_u8(h, (uint8_t)(a <= b));
        h = mix_u8(h, (uint8_t)(a >= b));
    }
    return h;
}
