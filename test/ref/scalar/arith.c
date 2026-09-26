/* add and sub chains that cross the wrap boundary in both directions, plus negation,
 * at every integer width and signedness, one noinline part per type. the narrow signed
 * parts keep the re-extension after narrow arithmetic live. */
#include "corpus.h"

static uint64_t arith_u8(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    uint8_t a = (uint8_t)(UINT8_C(240) + s);
    for (uint8_t i = 0; i < UINT8_C(32); i = (uint8_t)(i + UINT8_C(1))) {
        a = (uint8_t)(a + (uint8_t)(i * UINT8_C(7) + UINT8_C(1)));
        h = mix_u8(h, a);
        a = (uint8_t)(a - (uint8_t)(i * UINT8_C(3) + UINT8_C(2)));
        h = mix_u8(h, a);
    }

    uint8_t b = s;
    for (uint8_t j = 0; j < UINT8_C(8); j = (uint8_t)(j + UINT8_C(1))) {
        b = (uint8_t)(b - (uint8_t)(j * UINT8_C(181) + UINT8_C(1)));
        h = mix_u8(h, b);
    }

    const uint8_t n = (uint8_t)(UINT8_C(0) - a);
    h = mix_u8(h, n);
    h = mix_u8(h, (uint8_t)(UINT8_C(0) - n));
    h = mix_u8(h, (uint8_t)(UINT8_C(0) - s));
    h = mix_u8(h, (uint8_t)(UINT8_C(127) + a));
    h = mix_u8(h, (uint8_t)(UINT8_C(128) - a));
    h = mix_u8(h, (uint8_t)(UINT8_C(255) + a));
    h = mix_u8(h, (uint8_t)(a + b));
    h = mix_u8(h, (uint8_t)(a - b));
    h = mix_u8(h, (uint8_t)(b - a));
    return h;
}

/* signed add/sub/negate wraps in two's complement, identical to arith_u8 at the
 * bit level, so every step is done on the uint8_t bit pattern and only bitcast
 * to int8_t at the fold call, per the reference rule against signed overflow UB. */
static inline int8_t as_i8_arith_i8(uint8_t v) { int8_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t arith_i8(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    uint8_t a = (uint8_t)(UINT8_C(240) + s);
    for (uint8_t i = 0; i < UINT8_C(32); i = (uint8_t)(i + UINT8_C(1))) {
        a = (uint8_t)(a + (uint8_t)(i * UINT8_C(7) + UINT8_C(1)));
        h = mix_i8(h, as_i8_arith_i8(a));
        a = (uint8_t)(a - (uint8_t)(i * UINT8_C(3) + UINT8_C(2)));
        h = mix_i8(h, as_i8_arith_i8(a));
    }

    uint8_t b = s;
    for (uint8_t j = 0; j < UINT8_C(8); j = (uint8_t)(j + UINT8_C(1))) {
        b = (uint8_t)(b - (uint8_t)(j * UINT8_C(53) + UINT8_C(1)));
        h = mix_i8(h, as_i8_arith_i8(b));
    }

    const uint8_t n = (uint8_t)(UINT8_C(0) - a);
    h = mix_i8(h, as_i8_arith_i8(n));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(UINT8_C(0) - n)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(UINT8_C(0) - s)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(UINT8_C(127) + a)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(UINT8_C(128) - a)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(UINT8_C(255) + a)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(a + b)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(a - b)));
    h = mix_i8(h, as_i8_arith_i8((uint8_t)(b - a)));
    return h;
}

static uint64_t arith_u16(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    uint16_t a = (uint16_t)(UINT16_C(65520) + s);
    for (uint16_t i = 0; i < UINT16_C(32); i = (uint16_t)(i + UINT16_C(1))) {
        a = (uint16_t)(a + (uint16_t)(i * UINT16_C(7) + UINT16_C(1)));
        h = mix_u16(h, a);
        a = (uint16_t)(a - (uint16_t)(i * UINT16_C(3) + UINT16_C(2)));
        h = mix_u16(h, a);
    }

    uint16_t b = s;
    for (uint16_t j = 0; j < UINT16_C(8); j = (uint16_t)(j + UINT16_C(1))) {
        b = (uint16_t)(b - (uint16_t)(j * UINT16_C(43981) + UINT16_C(1)));
        h = mix_u16(h, b);
    }

    const uint16_t n = (uint16_t)(UINT16_C(0) - a);
    h = mix_u16(h, n);
    h = mix_u16(h, (uint16_t)(UINT16_C(0) - n));
    h = mix_u16(h, (uint16_t)(UINT16_C(0) - s));
    h = mix_u16(h, (uint16_t)(UINT16_C(32767) + a));
    h = mix_u16(h, (uint16_t)(UINT16_C(32768) - a));
    h = mix_u16(h, (uint16_t)(UINT16_C(65535) + a));
    h = mix_u16(h, (uint16_t)(a + b));
    h = mix_u16(h, (uint16_t)(a - b));
    h = mix_u16(h, (uint16_t)(b - a));
    return h;
}

/* signed add/sub/negate wraps in two's complement, identical to arith_u16 at the
 * bit level, so every step is done on the uint16_t bit pattern and only bitcast
 * to int16_t at the fold call, per the reference rule against signed overflow UB. */
static inline int16_t as_i16_arith_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t arith_i16(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    uint16_t a = (uint16_t)(UINT16_C(65520) + s);
    for (uint16_t i = 0; i < UINT16_C(32); i = (uint16_t)(i + UINT16_C(1))) {
        a = (uint16_t)(a + (uint16_t)(i * UINT16_C(7) + UINT16_C(1)));
        h = mix_i16(h, as_i16_arith_i16(a));
        a = (uint16_t)(a - (uint16_t)(i * UINT16_C(3) + UINT16_C(2)));
        h = mix_i16(h, as_i16_arith_i16(a));
    }

    uint16_t b = s;
    for (uint16_t j = 0; j < UINT16_C(8); j = (uint16_t)(j + UINT16_C(1))) {
        b = (uint16_t)(b - (uint16_t)(j * UINT16_C(12345) + UINT16_C(1)));
        h = mix_i16(h, as_i16_arith_i16(b));
    }

    const uint16_t n = (uint16_t)(UINT16_C(0) - a);
    h = mix_i16(h, as_i16_arith_i16(n));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(UINT16_C(0) - n)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(UINT16_C(0) - s)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(UINT16_C(32767) + a)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(UINT16_C(32768) - a)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(UINT16_C(65535) + a)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(a + b)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(a - b)));
    h = mix_i16(h, as_i16_arith_i16((uint16_t)(b - a)));
    return h;
}

static uint64_t arith_u32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(4294967280) + s);
    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a + (uint32_t)(i * UINT32_C(7) + UINT32_C(1)));
        h = mix_u32(h, a);
        a = (uint32_t)(a - (uint32_t)(i * UINT32_C(3) + UINT32_C(2)));
        h = mix_u32(h, a);
    }

    uint32_t b = s;
    for (uint32_t j = 0; j < UINT32_C(8); j = (uint32_t)(j + UINT32_C(1))) {
        b = (uint32_t)(b - (uint32_t)(j * UINT32_C(305419896) + UINT32_C(1)));
        h = mix_u32(h, b);
    }

    const uint32_t n = (uint32_t)(UINT32_C(0) - a);
    h = mix_u32(h, n);
    h = mix_u32(h, (uint32_t)(UINT32_C(0) - n));
    h = mix_u32(h, (uint32_t)(UINT32_C(0) - s));
    h = mix_u32(h, (uint32_t)(UINT32_C(2147483647) + a));
    h = mix_u32(h, (uint32_t)(UINT32_C(2147483648) - a));
    h = mix_u32(h, (uint32_t)(UINT32_C(4294967295) + a));
    h = mix_u32(h, (uint32_t)(a + b));
    h = mix_u32(h, (uint32_t)(a - b));
    h = mix_u32(h, (uint32_t)(b - a));
    return h;
}

/* signed add/sub/negate wraps in two's complement, identical to arith_u32 at the
 * bit level, so every step is done on the uint32_t bit pattern and only bitcast
 * to int32_t at the fold call, per the reference rule against signed overflow UB. */
static inline int32_t as_i32_arith_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t arith_i32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(4294967280) + s);
    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a + (uint32_t)(i * UINT32_C(7) + UINT32_C(1)));
        h = mix_i32(h, as_i32_arith_i32(a));
        a = (uint32_t)(a - (uint32_t)(i * UINT32_C(3) + UINT32_C(2)));
        h = mix_i32(h, as_i32_arith_i32(a));
    }

    uint32_t b = s;
    for (uint32_t j = 0; j < UINT32_C(8); j = (uint32_t)(j + UINT32_C(1))) {
        b = (uint32_t)(b - (uint32_t)(j * UINT32_C(305419896) + UINT32_C(1)));
        h = mix_i32(h, as_i32_arith_i32(b));
    }

    const uint32_t n = (uint32_t)(UINT32_C(0) - a);
    h = mix_i32(h, as_i32_arith_i32(n));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(UINT32_C(0) - n)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(UINT32_C(0) - s)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(UINT32_C(2147483647) + a)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(UINT32_C(2147483648) - a)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(UINT32_C(4294967295) + a)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(a + b)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(a - b)));
    h = mix_i32(h, as_i32_arith_i32((uint32_t)(b - a)));
    return h;
}

static uint64_t arith_u64(uint64_t seed) {
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

/* signed add/sub/negate wraps in two's complement, identical to arith_u64 at the
 * bit level, so every step is done on the uint64_t bit pattern and only bitcast
 * to int64_t at the fold call, per the reference rule against signed overflow UB. */
static inline int64_t as_i64_arith_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t arith_i64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(18446744073709551600) + s);
    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a + (uint64_t)(i * UINT64_C(7) + UINT64_C(1)));
        h = mix_i64(h, as_i64_arith_i64(a));
        a = (uint64_t)(a - (uint64_t)(i * UINT64_C(3) + UINT64_C(2)));
        h = mix_i64(h, as_i64_arith_i64(a));
    }

    uint64_t b = s;
    for (uint64_t j = 0; j < UINT64_C(8); j = (uint64_t)(j + UINT64_C(1))) {
        b = (uint64_t)(b - (uint64_t)(j * UINT64_C(1311768467294899695) + UINT64_C(1)));
        h = mix_i64(h, as_i64_arith_i64(b));
    }

    const uint64_t n = (uint64_t)(UINT64_C(0) - a);
    h = mix_i64(h, as_i64_arith_i64(n));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(UINT64_C(0) - n)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(UINT64_C(0) - s)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(UINT64_C(9223372036854775807) + a)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(UINT64_C(9223372036854775808) - a)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(UINT64_C(18446744073709551615) + a)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(a + b)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(a - b)));
    h = mix_i64(h, as_i64_arith_i64((uint64_t)(b - a)));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, arith_u8(seed));
    h = mix_u64(h, arith_i8(seed));
    h = mix_u64(h, arith_u16(seed));
    h = mix_u64(h, arith_i16(seed));
    h = mix_u64(h, arith_u32(seed));
    h = mix_u64(h, arith_i32(seed));
    h = mix_u64(h, arith_u64(seed));
    h = mix_u64(h, arith_i64(seed));
    return h;
}
