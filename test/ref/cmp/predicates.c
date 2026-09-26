/* all six integer predicates over boundary pairs at 32 and 64 bits, signed and unsigned,
 * one noinline part per type. */
#include "corpus.h"

static uint64_t cmp_u32(uint64_t seed) {
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

/* signed mach arithmetic is expressed here as the two's-complement identity on
 * the matching unsigned type: cast to unsigned, add mod 2^32, bit-reinterpret
 * back through memcpy. every step is defined by the C standard, so a disagreement
 * always indicts mach, never this file. */
static inline int32_t bitcast_i32_cmp_i32(uint32_t v) {
    int32_t r;
    memcpy(&r, &v, sizeof r);
    return r;
}

static uint64_t cmp_i32(uint64_t seed) {
    uint64_t h = fold_init();
    const int32_t s = bitcast_i32_cmp_i32((uint32_t)seed);

    const int32_t IMIN = bitcast_i32_cmp_i32(UINT32_C(2147483648));
    const int32_t IMAX = INT32_C(2147483647);

    const int32_t av[7] = { 0, 0, -1, IMAX, IMIN, -1, IMIN };
    const int32_t bv[7] = { 0, -1, 0, IMIN, IMAX, -1, IMIN };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const int32_t a = bitcast_i32_cmp_i32((uint32_t)((uint32_t)av[k] + (uint32_t)s));
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

static uint64_t cmp_u64(uint64_t seed) {
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

/* signed mach arithmetic is expressed here as the two's-complement identity on
 * the matching unsigned type: cast to unsigned, add mod 2^64, bit-reinterpret
 * back through memcpy. every step is defined by the C standard, so a disagreement
 * always indicts mach, never this file. */
static inline int64_t bitcast_i64_cmp_i64(uint64_t v) {
    int64_t r;
    memcpy(&r, &v, sizeof r);
    return r;
}

static uint64_t cmp_i64(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = bitcast_i64_cmp_i64(seed);

    const int64_t IMIN = bitcast_i64_cmp_i64(UINT64_C(9223372036854775808));
    const int64_t IMAX = INT64_C(9223372036854775807);

    const int64_t av[7] = { 0, 0, -1, IMAX, IMIN, -1, IMIN };
    const int64_t bv[7] = { 0, -1, 0, IMIN, IMAX, -1, IMIN };

    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        const int64_t a = bitcast_i64_cmp_i64((uint64_t)av[k] + (uint64_t)s);
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

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, cmp_u32(seed));
    h = mix_u64(h, cmp_i32(seed));
    h = mix_u64(h, cmp_u64(seed));
    h = mix_u64(h, cmp_i64(seed));
    return h;
}
