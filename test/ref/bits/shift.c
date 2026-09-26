/* shifts at every width and signedness: << and >> at counts 0, 1 and w-1, then counts
 * at and past the width, which saturate (#3756). one noinline part per type. */
#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint8_t shl8_shift_u8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v << n); }
static inline uint8_t shr8_shift_u8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v >> n); }


static uint64_t shift_u8(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    const uint8_t zero = UINT8_C(0);
    const uint8_t ones = UINT8_C(255);
    const uint8_t even = UINT8_C(170);
    const uint8_t odd  = UINT8_C(85);

    uint8_t a = (uint8_t)(ones ^ s);
    uint8_t b = (uint8_t)(even ^ s);
    uint8_t c = (uint8_t)(odd ^ s);
    uint8_t z = (uint8_t)(zero ^ s);

    h = mix_u8(h, shl8_shift_u8(a, 0));
    h = mix_u8(h, shl8_shift_u8(a, 1));
    h = mix_u8(h, shl8_shift_u8(a, 7));
    h = mix_u8(h, shr8_shift_u8(a, 0));
    h = mix_u8(h, shr8_shift_u8(a, 1));
    h = mix_u8(h, shr8_shift_u8(a, 7));
    h = mix_u8(h, shl8_shift_u8(b, 1));
    h = mix_u8(h, shr8_shift_u8(b, 1));
    h = mix_u8(h, shr8_shift_u8(b, 7));
    h = mix_u8(h, shl8_shift_u8(c, 1));
    h = mix_u8(h, shr8_shift_u8(c, 1));
    h = mix_u8(h, shl8_shift_u8(z, 5));
    h = mix_u8(h, shr8_shift_u8(z, 5));

    for (uint8_t i = 0; i < 8; i = (uint8_t)(i + 1)) {
        h = mix_u8(h, shl8_shift_u8(a, i));
        h = mix_u8(h, shr8_shift_u8(a, i));
        h = mix_u8(h, shl8_shift_u8(b, i));
        h = mix_u8(h, shr8_shift_u8(c, i));
    }

    h = mix_u8(h, shl8_shift_u8(a, 7));
    h = mix_u8(h, shr8_shift_u8(a, 7));
    h = mix_u8(h, shr8_shift_u8(b, 7));
    h = mix_u8(h, shl8_shift_u8(c, 7));
    h = mix_u8(h, shl8_shift_u8(a, 8));
    h = mix_u8(h, shr8_shift_u8(a, 8));
    h = mix_u8(h, shr8_shift_u8(b, 8));
    h = mix_u8(h, shl8_shift_u8(c, 8));
    h = mix_u8(h, shl8_shift_u8(a, 9));
    h = mix_u8(h, shr8_shift_u8(a, 9));
    h = mix_u8(h, shr8_shift_u8(b, 9));
    h = mix_u8(h, shl8_shift_u8(c, 9));
    h = mix_u8(h, shl8_shift_u8(a, 16));
    h = mix_u8(h, shr8_shift_u8(a, 16));
    h = mix_u8(h, shr8_shift_u8(b, 16));
    h = mix_u8(h, shl8_shift_u8(c, 16));
    h = mix_u8(h, shl8_shift_u8(a, 23));
    h = mix_u8(h, shr8_shift_u8(a, 23));
    h = mix_u8(h, shr8_shift_u8(b, 23));
    h = mix_u8(h, shl8_shift_u8(c, 23));
    h = mix_u8(h, shl8_shift_u8(a, 255));
    h = mix_u8(h, shr8_shift_u8(a, 255));
    h = mix_u8(h, shr8_shift_u8(b, 255));
    h = mix_u8(h, shl8_shift_u8(c, 255));
    h = mix_u8(h, shl8_shift_u8(a, 33));
    h = mix_u8(h, shr8_shift_u8(a, 33));
    h = mix_u8(h, shr8_shift_u8(b, 33));
    h = mix_u8(h, shl8_shift_u8(c, 33));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 2u + j + (seed & 3u);
        h = mix_u8(h, shl8_shift_u8(a, n));
        h = mix_u8(h, shr8_shift_u8(a, n));
        h = mix_u8(h, shr8_shift_u8(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint8_t shl8_shift_i8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v << n); }
static inline uint8_t asr8_shift_i8(uint8_t v, uint64_t n) {
    const uint8_t fill = (uint8_t)(UINT8_C(0) - (v >> 7));
    if (n >= 8u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint8_t)((v >> n) | (uint8_t)(fill << (8u - n)));
}


static uint64_t shift_i8(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    const uint8_t zero = UINT8_C(0);
    const uint8_t neg1 = UINT8_C(255);
    const uint8_t even = UINT8_C(170);
    const uint8_t odd  = UINT8_C(85);

    uint8_t a = (uint8_t)(neg1 ^ s);
    uint8_t b = (uint8_t)(even ^ s);
    uint8_t c = (uint8_t)(odd ^ s);
    uint8_t z = (uint8_t)(zero ^ s);

    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 0));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 1));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 7));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 0));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 1));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 7));
    h = mix_i8(h, (int8_t)shl8_shift_i8(b, 1));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 1));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 7));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 1));
    h = mix_i8(h, (int8_t)asr8_shift_i8(c, 1));
    h = mix_i8(h, (int8_t)shl8_shift_i8(z, 5));
    h = mix_i8(h, (int8_t)asr8_shift_i8(z, 5));

    for (uint8_t i = 0; i < 8; i = (uint8_t)(i + 1)) {
        h = mix_i8(h, (int8_t)shl8_shift_i8(a, i));
        h = mix_i8(h, (int8_t)asr8_shift_i8(a, i));
        h = mix_i8(h, (int8_t)shl8_shift_i8(b, i));
        h = mix_i8(h, (int8_t)asr8_shift_i8(c, i));
    }

    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 7));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 7));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 7));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 7));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 8));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 8));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 8));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 8));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 9));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 9));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 9));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 9));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 16));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 16));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 16));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 16));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 23));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 23));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 23));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 23));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 255));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 255));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 255));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 255));
    h = mix_i8(h, (int8_t)shl8_shift_i8(a, 33));
    h = mix_i8(h, (int8_t)asr8_shift_i8(a, 33));
    h = mix_i8(h, (int8_t)asr8_shift_i8(b, 33));
    h = mix_i8(h, (int8_t)shl8_shift_i8(c, 33));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 2u + j + (seed & 3u);
        h = mix_i8(h, (int8_t)shl8_shift_i8(a, n));
        h = mix_i8(h, (int8_t)asr8_shift_i8(a, n));
        h = mix_i8(h, (int8_t)asr8_shift_i8(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint16_t shl16_shift_u16(uint16_t v, uint64_t n) { return n >= 16u ? (uint16_t)0 : (uint16_t)(v << n); }
static inline uint16_t shr16_shift_u16(uint16_t v, uint64_t n) { return n >= 16u ? (uint16_t)0 : (uint16_t)(v >> n); }


static uint64_t shift_u16(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    const uint16_t zero = UINT16_C(0);
    const uint16_t ones = UINT16_C(65535);
    const uint16_t even = UINT16_C(43690);
    const uint16_t odd  = UINT16_C(21845);

    uint16_t a = (uint16_t)(ones ^ s);
    uint16_t b = (uint16_t)(even ^ s);
    uint16_t c = (uint16_t)(odd ^ s);
    uint16_t z = (uint16_t)(zero ^ s);

    h = mix_u16(h, shl16_shift_u16(a, 0));
    h = mix_u16(h, shl16_shift_u16(a, 1));
    h = mix_u16(h, shl16_shift_u16(a, 15));
    h = mix_u16(h, shr16_shift_u16(a, 0));
    h = mix_u16(h, shr16_shift_u16(a, 1));
    h = mix_u16(h, shr16_shift_u16(a, 15));
    h = mix_u16(h, shl16_shift_u16(b, 1));
    h = mix_u16(h, shr16_shift_u16(b, 1));
    h = mix_u16(h, shr16_shift_u16(b, 15));
    h = mix_u16(h, shl16_shift_u16(c, 1));
    h = mix_u16(h, shr16_shift_u16(c, 1));
    h = mix_u16(h, shl16_shift_u16(z, 5));
    h = mix_u16(h, shr16_shift_u16(z, 5));

    for (uint8_t i = 0; i < 16; i = (uint8_t)(i + 1)) {
        h = mix_u16(h, shl16_shift_u16(a, i));
        h = mix_u16(h, shr16_shift_u16(a, i));
        h = mix_u16(h, shl16_shift_u16(b, i));
        h = mix_u16(h, shr16_shift_u16(c, i));
    }

    h = mix_u16(h, shl16_shift_u16(a, 15));
    h = mix_u16(h, shr16_shift_u16(a, 15));
    h = mix_u16(h, shr16_shift_u16(b, 15));
    h = mix_u16(h, shl16_shift_u16(c, 15));
    h = mix_u16(h, shl16_shift_u16(a, 16));
    h = mix_u16(h, shr16_shift_u16(a, 16));
    h = mix_u16(h, shr16_shift_u16(b, 16));
    h = mix_u16(h, shl16_shift_u16(c, 16));
    h = mix_u16(h, shl16_shift_u16(a, 17));
    h = mix_u16(h, shr16_shift_u16(a, 17));
    h = mix_u16(h, shr16_shift_u16(b, 17));
    h = mix_u16(h, shl16_shift_u16(c, 17));
    h = mix_u16(h, shl16_shift_u16(a, 32));
    h = mix_u16(h, shr16_shift_u16(a, 32));
    h = mix_u16(h, shr16_shift_u16(b, 32));
    h = mix_u16(h, shl16_shift_u16(c, 32));
    h = mix_u16(h, shl16_shift_u16(a, 39));
    h = mix_u16(h, shr16_shift_u16(a, 39));
    h = mix_u16(h, shr16_shift_u16(b, 39));
    h = mix_u16(h, shl16_shift_u16(c, 39));
    h = mix_u16(h, shl16_shift_u16(a, 255));
    h = mix_u16(h, shr16_shift_u16(a, 255));
    h = mix_u16(h, shr16_shift_u16(b, 255));
    h = mix_u16(h, shl16_shift_u16(c, 255));
    h = mix_u16(h, shl16_shift_u16(a, 33));
    h = mix_u16(h, shr16_shift_u16(a, 33));
    h = mix_u16(h, shr16_shift_u16(b, 33));
    h = mix_u16(h, shl16_shift_u16(c, 33));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 10u + j + (seed & 3u);
        h = mix_u16(h, shl16_shift_u16(a, n));
        h = mix_u16(h, shr16_shift_u16(a, n));
        h = mix_u16(h, shr16_shift_u16(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint16_t shl16_shift_i16(uint16_t v, uint64_t n) { return n >= 16u ? (uint16_t)0 : (uint16_t)(v << n); }
static inline uint16_t asr16_shift_i16(uint16_t v, uint64_t n) {
    const uint16_t fill = (uint16_t)(UINT16_C(0) - (v >> 15));
    if (n >= 16u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint16_t)((v >> n) | (uint16_t)(fill << (16u - n)));
}


static uint64_t shift_i16(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    const uint16_t zero = UINT16_C(0);
    const uint16_t neg1 = UINT16_C(65535);
    const uint16_t even = UINT16_C(43690);
    const uint16_t odd  = UINT16_C(21845);

    uint16_t a = (uint16_t)(neg1 ^ s);
    uint16_t b = (uint16_t)(even ^ s);
    uint16_t c = (uint16_t)(odd ^ s);
    uint16_t z = (uint16_t)(zero ^ s);

    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 0));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 1));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 15));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 0));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 1));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 15));
    h = mix_i16(h, (int16_t)shl16_shift_i16(b, 1));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 1));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 15));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 1));
    h = mix_i16(h, (int16_t)asr16_shift_i16(c, 1));
    h = mix_i16(h, (int16_t)shl16_shift_i16(z, 5));
    h = mix_i16(h, (int16_t)asr16_shift_i16(z, 5));

    for (uint8_t i = 0; i < 16; i = (uint8_t)(i + 1)) {
        h = mix_i16(h, (int16_t)shl16_shift_i16(a, i));
        h = mix_i16(h, (int16_t)asr16_shift_i16(a, i));
        h = mix_i16(h, (int16_t)shl16_shift_i16(b, i));
        h = mix_i16(h, (int16_t)asr16_shift_i16(c, i));
    }

    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 15));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 15));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 15));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 15));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 16));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 16));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 16));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 16));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 17));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 17));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 17));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 17));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 32));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 32));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 32));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 32));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 39));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 39));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 39));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 39));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 255));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 255));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 255));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 255));
    h = mix_i16(h, (int16_t)shl16_shift_i16(a, 33));
    h = mix_i16(h, (int16_t)asr16_shift_i16(a, 33));
    h = mix_i16(h, (int16_t)asr16_shift_i16(b, 33));
    h = mix_i16(h, (int16_t)shl16_shift_i16(c, 33));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 10u + j + (seed & 3u);
        h = mix_i16(h, (int16_t)shl16_shift_i16(a, n));
        h = mix_i16(h, (int16_t)asr16_shift_i16(a, n));
        h = mix_i16(h, (int16_t)asr16_shift_i16(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint32_t shl32_shift_u32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v << n); }
static inline uint32_t shr32_shift_u32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v >> n); }


static uint64_t shift_u32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t zero = UINT32_C(0);
    const uint32_t ones = UINT32_C(4294967295);
    const uint32_t even = UINT32_C(2863311530);
    const uint32_t odd  = UINT32_C(1431655765);

    uint32_t a = (uint32_t)(ones ^ s);
    uint32_t b = (uint32_t)(even ^ s);
    uint32_t c = (uint32_t)(odd ^ s);
    uint32_t z = (uint32_t)(zero ^ s);

    h = mix_u32(h, shl32_shift_u32(a, 0));
    h = mix_u32(h, shl32_shift_u32(a, 1));
    h = mix_u32(h, shl32_shift_u32(a, 31));
    h = mix_u32(h, shr32_shift_u32(a, 0));
    h = mix_u32(h, shr32_shift_u32(a, 1));
    h = mix_u32(h, shr32_shift_u32(a, 31));
    h = mix_u32(h, shl32_shift_u32(b, 1));
    h = mix_u32(h, shr32_shift_u32(b, 1));
    h = mix_u32(h, shr32_shift_u32(b, 31));
    h = mix_u32(h, shl32_shift_u32(c, 1));
    h = mix_u32(h, shr32_shift_u32(c, 1));
    h = mix_u32(h, shl32_shift_u32(z, 5));
    h = mix_u32(h, shr32_shift_u32(z, 5));

    for (uint8_t i = 0; i < 32; i = (uint8_t)(i + 1)) {
        h = mix_u32(h, shl32_shift_u32(a, i));
        h = mix_u32(h, shr32_shift_u32(a, i));
        h = mix_u32(h, shl32_shift_u32(b, i));
        h = mix_u32(h, shr32_shift_u32(c, i));
    }

    h = mix_u32(h, shl32_shift_u32(a, 31));
    h = mix_u32(h, shr32_shift_u32(a, 31));
    h = mix_u32(h, shr32_shift_u32(b, 31));
    h = mix_u32(h, shl32_shift_u32(c, 31));
    h = mix_u32(h, shl32_shift_u32(a, 32));
    h = mix_u32(h, shr32_shift_u32(a, 32));
    h = mix_u32(h, shr32_shift_u32(b, 32));
    h = mix_u32(h, shl32_shift_u32(c, 32));
    h = mix_u32(h, shl32_shift_u32(a, 33));
    h = mix_u32(h, shr32_shift_u32(a, 33));
    h = mix_u32(h, shr32_shift_u32(b, 33));
    h = mix_u32(h, shl32_shift_u32(c, 33));
    h = mix_u32(h, shl32_shift_u32(a, 64));
    h = mix_u32(h, shr32_shift_u32(a, 64));
    h = mix_u32(h, shr32_shift_u32(b, 64));
    h = mix_u32(h, shl32_shift_u32(c, 64));
    h = mix_u32(h, shl32_shift_u32(a, 71));
    h = mix_u32(h, shr32_shift_u32(a, 71));
    h = mix_u32(h, shr32_shift_u32(b, 71));
    h = mix_u32(h, shl32_shift_u32(c, 71));
    h = mix_u32(h, shl32_shift_u32(a, 255));
    h = mix_u32(h, shr32_shift_u32(a, 255));
    h = mix_u32(h, shr32_shift_u32(b, 255));
    h = mix_u32(h, shl32_shift_u32(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 26u + j + (seed & 3u);
        h = mix_u32(h, shl32_shift_u32(a, n));
        h = mix_u32(h, shr32_shift_u32(a, n));
        h = mix_u32(h, shr32_shift_u32(b, n));
    }

    h = mix_u32(h, shl32_shift_u32(a, 37u & 31u));
    h = mix_u32(h, shr32_shift_u32(b, 255u & 31u));

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint32_t shl32_shift_i32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v << n); }
static inline uint32_t asr32_shift_i32(uint32_t v, uint64_t n) {
    const uint32_t fill = (uint32_t)(UINT32_C(0) - (v >> 31));
    if (n >= 32u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint32_t)((v >> n) | (uint32_t)(fill << (32u - n)));
}


static uint64_t shift_i32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t zero = UINT32_C(0);
    const uint32_t neg1 = UINT32_C(4294967295);
    const uint32_t even = UINT32_C(2863311530);
    const uint32_t odd  = UINT32_C(1431655765);

    uint32_t a = (uint32_t)(neg1 ^ s);
    uint32_t b = (uint32_t)(even ^ s);
    uint32_t c = (uint32_t)(odd ^ s);
    uint32_t z = (uint32_t)(zero ^ s);

    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 0));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 1));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 31));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 0));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 1));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 31));
    h = mix_i32(h, (int32_t)shl32_shift_i32(b, 1));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 1));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 31));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 1));
    h = mix_i32(h, (int32_t)asr32_shift_i32(c, 1));
    h = mix_i32(h, (int32_t)shl32_shift_i32(z, 5));
    h = mix_i32(h, (int32_t)asr32_shift_i32(z, 5));

    for (uint8_t i = 0; i < 32; i = (uint8_t)(i + 1)) {
        h = mix_i32(h, (int32_t)shl32_shift_i32(a, i));
        h = mix_i32(h, (int32_t)asr32_shift_i32(a, i));
        h = mix_i32(h, (int32_t)shl32_shift_i32(b, i));
        h = mix_i32(h, (int32_t)asr32_shift_i32(c, i));
    }

    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 31));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 31));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 31));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 31));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 32));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 32));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 32));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 32));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 33));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 33));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 33));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 33));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 64));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 64));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 64));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 64));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 71));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 71));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 71));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 71));
    h = mix_i32(h, (int32_t)shl32_shift_i32(a, 255));
    h = mix_i32(h, (int32_t)asr32_shift_i32(a, 255));
    h = mix_i32(h, (int32_t)asr32_shift_i32(b, 255));
    h = mix_i32(h, (int32_t)shl32_shift_i32(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 26u + j + (seed & 3u);
        h = mix_i32(h, (int32_t)shl32_shift_i32(a, n));
        h = mix_i32(h, (int32_t)asr32_shift_i32(a, n));
        h = mix_i32(h, (int32_t)asr32_shift_i32(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint64_t shl64_shift_u64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v << n); }
static inline uint64_t shr64_shift_u64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v >> n); }


static uint64_t shift_u64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = (uint64_t)seed;

    const uint64_t zero = UINT64_C(0);
    const uint64_t ones = UINT64_C(18446744073709551615);
    const uint64_t even = UINT64_C(12297829382473034410);
    const uint64_t odd  = UINT64_C(6148914691236517205);

    uint64_t a = (uint64_t)(ones ^ s);
    uint64_t b = (uint64_t)(even ^ s);
    uint64_t c = (uint64_t)(odd ^ s);
    uint64_t z = (uint64_t)(zero ^ s);

    h = mix_u64(h, shl64_shift_u64(a, 0));
    h = mix_u64(h, shl64_shift_u64(a, 1));
    h = mix_u64(h, shl64_shift_u64(a, 63));
    h = mix_u64(h, shr64_shift_u64(a, 0));
    h = mix_u64(h, shr64_shift_u64(a, 1));
    h = mix_u64(h, shr64_shift_u64(a, 63));
    h = mix_u64(h, shl64_shift_u64(b, 1));
    h = mix_u64(h, shr64_shift_u64(b, 1));
    h = mix_u64(h, shr64_shift_u64(b, 63));
    h = mix_u64(h, shl64_shift_u64(c, 1));
    h = mix_u64(h, shr64_shift_u64(c, 1));
    h = mix_u64(h, shl64_shift_u64(z, 5));
    h = mix_u64(h, shr64_shift_u64(z, 5));

    for (uint8_t i = 0; i < 64; i = (uint8_t)(i + 1)) {
        h = mix_u64(h, shl64_shift_u64(a, i));
        h = mix_u64(h, shr64_shift_u64(a, i));
        h = mix_u64(h, shl64_shift_u64(b, i));
        h = mix_u64(h, shr64_shift_u64(c, i));
    }

    h = mix_u64(h, shl64_shift_u64(a, 63));
    h = mix_u64(h, shr64_shift_u64(a, 63));
    h = mix_u64(h, shr64_shift_u64(b, 63));
    h = mix_u64(h, shl64_shift_u64(c, 63));
    h = mix_u64(h, shl64_shift_u64(a, 64));
    h = mix_u64(h, shr64_shift_u64(a, 64));
    h = mix_u64(h, shr64_shift_u64(b, 64));
    h = mix_u64(h, shl64_shift_u64(c, 64));
    h = mix_u64(h, shl64_shift_u64(a, 65));
    h = mix_u64(h, shr64_shift_u64(a, 65));
    h = mix_u64(h, shr64_shift_u64(b, 65));
    h = mix_u64(h, shl64_shift_u64(c, 65));
    h = mix_u64(h, shl64_shift_u64(a, 128));
    h = mix_u64(h, shr64_shift_u64(a, 128));
    h = mix_u64(h, shr64_shift_u64(b, 128));
    h = mix_u64(h, shl64_shift_u64(c, 128));
    h = mix_u64(h, shl64_shift_u64(a, 135));
    h = mix_u64(h, shr64_shift_u64(a, 135));
    h = mix_u64(h, shr64_shift_u64(b, 135));
    h = mix_u64(h, shl64_shift_u64(c, 135));
    h = mix_u64(h, shl64_shift_u64(a, 255));
    h = mix_u64(h, shr64_shift_u64(a, 255));
    h = mix_u64(h, shr64_shift_u64(b, 255));
    h = mix_u64(h, shl64_shift_u64(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 58u + j + (seed & 3u);
        h = mix_u64(h, shl64_shift_u64(a, n));
        h = mix_u64(h, shr64_shift_u64(a, n));
        h = mix_u64(h, shr64_shift_u64(b, n));
    }

    return h;
}

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint64_t shl64_shift_i64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v << n); }
static inline uint64_t asr64_shift_i64(uint64_t v, uint64_t n) {
    const uint64_t fill = (uint64_t)(UINT64_C(0) - (v >> 63));
    if (n >= 64u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint64_t)((v >> n) | (uint64_t)(fill << (64u - n)));
}


static uint64_t shift_i64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = (uint64_t)seed;

    const uint64_t zero = UINT64_C(0);
    const uint64_t neg1 = UINT64_C(18446744073709551615);
    const uint64_t even = UINT64_C(12297829382473034410);
    const uint64_t odd  = UINT64_C(6148914691236517205);

    uint64_t a = (uint64_t)(neg1 ^ s);
    uint64_t b = (uint64_t)(even ^ s);
    uint64_t c = (uint64_t)(odd ^ s);
    uint64_t z = (uint64_t)(zero ^ s);

    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 0));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 1));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 63));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 0));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 1));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 63));
    h = mix_i64(h, (int64_t)shl64_shift_i64(b, 1));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 1));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 63));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 1));
    h = mix_i64(h, (int64_t)asr64_shift_i64(c, 1));
    h = mix_i64(h, (int64_t)shl64_shift_i64(z, 5));
    h = mix_i64(h, (int64_t)asr64_shift_i64(z, 5));

    for (uint8_t i = 0; i < 64; i = (uint8_t)(i + 1)) {
        h = mix_i64(h, (int64_t)shl64_shift_i64(a, i));
        h = mix_i64(h, (int64_t)asr64_shift_i64(a, i));
        h = mix_i64(h, (int64_t)shl64_shift_i64(b, i));
        h = mix_i64(h, (int64_t)asr64_shift_i64(c, i));
    }

    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 63));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 63));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 63));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 63));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 64));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 64));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 64));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 64));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 65));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 65));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 65));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 65));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 128));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 128));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 128));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 128));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 135));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 135));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 135));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 135));
    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 255));
    h = mix_i64(h, (int64_t)asr64_shift_i64(a, 255));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 255));
    h = mix_i64(h, (int64_t)shl64_shift_i64(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 58u + j + (seed & 3u);
        h = mix_i64(h, (int64_t)shl64_shift_i64(a, n));
        h = mix_i64(h, (int64_t)asr64_shift_i64(a, n));
        h = mix_i64(h, (int64_t)asr64_shift_i64(b, n));
    }

    h = mix_i64(h, (int64_t)shl64_shift_i64(a, 70u & 63u));
    h = mix_i64(h, (int64_t)asr64_shift_i64(b, 255u & 63u));

    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, shift_u8(seed));
    h = mix_u64(h, shift_i8(seed));
    h = mix_u64(h, shift_u16(seed));
    h = mix_u64(h, shift_i16(seed));
    h = mix_u64(h, shift_u32(seed));
    h = mix_u64(h, shift_i32(seed));
    h = mix_u64(h, shift_u64(seed));
    h = mix_u64(h, shift_i64(seed));
    return h;
}
