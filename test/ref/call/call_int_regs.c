#include "corpus.h"

/* mach's `u16:~i16` reads a bit pattern as the signed type of the same width. the
 * C conversion of an out-of-range unsigned value to a signed type is not UB but is
 * only two's-complement by implementation definition, so the reference goes through
 * memcpy and is defined by the standard alone. */
static inline int8_t as_i8(uint8_t v) { int8_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int16_t as_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t take16(uint8_t a0, int8_t a1, uint16_t a2, int16_t a3,
                       uint32_t a4, int32_t a5, uint64_t a6, int64_t a7,
                       uint8_t a8, int8_t a9, uint16_t a10, int16_t a11,
                       uint32_t a12, int32_t a13, uint64_t a14, int64_t a15) {
    uint64_t h = fold_init();
    h = mix_u8(h, a0);
    h = mix_i8(h, a1);
    h = mix_u16(h, a2);
    h = mix_i16(h, a3);
    h = mix_u32(h, a4);
    h = mix_i32(h, a5);
    h = mix_u64(h, a6);
    h = mix_i64(h, a7);
    h = mix_u8(h, a8);
    h = mix_i8(h, a9);
    h = mix_u16(h, a10);
    h = mix_i16(h, a11);
    h = mix_u32(h, a12);
    h = mix_i32(h, a13);
    h = mix_u64(h, a14);
    h = mix_i64(h, a15);
    return h;
}

static uint64_t take16n(uint8_t b0, uint8_t b1, int8_t b2, int8_t b3,
                        uint16_t b4, uint16_t b5, int16_t b6, int16_t b7,
                        uint8_t b8, int8_t b9, uint16_t b10, int16_t b11,
                        uint8_t b12, int8_t b13, uint16_t b14, int16_t b15) {
    uint64_t h = fold_init();
    h = mix_u8(h, b0);
    h = mix_u8(h, b1);
    h = mix_i8(h, b2);
    h = mix_i8(h, b3);
    h = mix_u16(h, b4);
    h = mix_u16(h, b5);
    h = mix_i16(h, b6);
    h = mix_i16(h, b7);
    h = mix_u8(h, b8);
    h = mix_i8(h, b9);
    h = mix_u16(h, b10);
    h = mix_i16(h, b11);
    h = mix_u8(h, b12);
    h = mix_i8(h, b13);
    h = mix_u16(h, b14);
    h = mix_i16(h, b15);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[20];
    for (uint64_t i = 0; i < UINT64_C(20); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    uint64_t r = 0;
    for (uint64_t pass = 0; pass < UINT64_C(3); pass = (uint64_t)(pass + UINT64_C(1))) {
        const uint64_t d = (uint64_t)(pass * UINT64_C(7) + seed);

        r = take16((uint8_t)v[0], as_i8((uint8_t)v[1]), (uint16_t)v[2], as_i16((uint16_t)v[3]),
                   (uint32_t)v[4], as_i32((uint32_t)v[5]), (uint64_t)(v[6] + d), as_i64(v[7]),
                   (uint8_t)v[8], as_i8((uint8_t)v[9]), (uint16_t)v[10], as_i16((uint16_t)v[11]),
                   (uint32_t)v[12], as_i32((uint32_t)v[13]), (uint64_t)(v[14] + d), as_i64(v[15]));
        h = mix_u64(h, r);

        r = take16n((uint8_t)v[0], (uint8_t)v[1], as_i8((uint8_t)v[2]), as_i8((uint8_t)v[3]),
                    (uint16_t)v[4], (uint16_t)v[5], as_i16((uint16_t)v[6]), as_i16((uint16_t)v[7]),
                    (uint8_t)(v[8] + d), as_i8((uint8_t)v[9]), (uint16_t)v[10], as_i16((uint16_t)v[11]),
                    (uint8_t)v[12], as_i8((uint8_t)v[13]), (uint16_t)v[14], as_i16((uint16_t)(v[15] + d)));
        h = mix_u64(h, r);
    }

    h = mix_u64(h, take16(
        (uint8_t)take16((uint8_t)v[0], as_i8((uint8_t)v[1]), (uint16_t)v[2], as_i16((uint16_t)v[3]),
                        (uint32_t)v[4], as_i32((uint32_t)v[5]), v[6], as_i64(v[7]),
                        (uint8_t)v[8], as_i8((uint8_t)v[9]), (uint16_t)v[10], as_i16((uint16_t)v[11]),
                        (uint32_t)v[12], as_i32((uint32_t)v[13]), v[14], as_i64(v[15])),
        as_i8((uint8_t)v[1]), (uint16_t)v[2], as_i16((uint16_t)v[3]),
        (uint32_t)r, as_i32((uint32_t)v[5]), v[6], as_i64(v[7]),
        (uint8_t)take16n((uint8_t)v[16], (uint8_t)v[17], as_i8((uint8_t)v[18]), as_i8((uint8_t)v[19]),
                         (uint16_t)v[4], (uint16_t)v[5], as_i16((uint16_t)v[6]), as_i16((uint16_t)v[7]),
                         (uint8_t)v[8], as_i8((uint8_t)v[9]), (uint16_t)v[10], as_i16((uint16_t)v[11]),
                         (uint8_t)v[12], as_i8((uint8_t)v[13]), (uint16_t)v[14], as_i16((uint16_t)v[15])),
        as_i8((uint8_t)v[9]), (uint16_t)v[10], as_i16((uint16_t)v[11]),
        (uint32_t)v[12], as_i32((uint32_t)v[13]), v[14], as_i64(v[15])));

    return h;
}
