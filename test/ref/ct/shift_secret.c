#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). every op runs on
 * the unsigned identity. */
static inline uint8_t shl8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v << n); }
static inline uint8_t asr8(uint8_t v, uint64_t n) {
    const uint8_t fill = (uint8_t)(UINT8_C(0) - (v >> 7));
    if (n >= 8u) { return fill; }
    if (n == 0u) { return v; }
    return (uint8_t)((v >> n) | (uint8_t)(fill << (8u - n)));
}
static inline uint32_t shr32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v >> n); }
static inline uint32_t asr32(uint32_t v, uint64_t n) {
    const uint32_t fill = (uint32_t)(UINT32_C(0) - (v >> 31));
    if (n >= 32u) { return fill; }
    if (n == 0u) { return v; }
    return (uint32_t)((v >> n) | (uint32_t)(fill << (32u - n)));
}
static inline uint64_t shl64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v << n); }
static inline uint64_t shr64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v >> n); }
static inline uint64_t asr64(uint64_t v, uint64_t n) {
    const uint64_t fill = (uint64_t)(UINT64_C(0) - (v >> 63));
    if (n >= 64u) { return fill; }
    if (n == 0u) { return v; }
    return (uint64_t)((v >> n) | (uint64_t)(fill << (64u - n)));
}

static uint64_t fold8(uint64_t h, uint64_t seed) {
    const uint8_t counts[7] = {0, 1, 7, 8, 9, 16, 255};
    const uint8_t a = (uint8_t)(UINT8_C(165) ^ (uint8_t)seed);
    const uint8_t b = (uint8_t)(UINT8_C(165) ^ (uint8_t)seed);
    uint64_t r = h;
    for (size_t i = 0; i < 7; i++) {
        r = mix_u8(r, shl8(a, counts[i]));
        r = mix_i8(r, (int8_t)asr8(b, counts[i]));
    }
    return r;
}

static uint64_t fold32(uint64_t h, uint64_t seed) {
    const uint8_t counts[7] = {0, 1, 31, 32, 33, 64, 255};
    const uint32_t a = (uint32_t)(UINT32_C(2863311530) ^ (uint32_t)seed);
    const uint32_t b = (uint32_t)(UINT32_C(2863311530) ^ (uint32_t)seed);
    const uint32_t nb = (uint32_t)(UINT32_C(0) - b);
    uint64_t r = h;
    for (size_t i = 0; i < 7; i++) {
        r = mix_u32(r, shr32(a, counts[i]));
        r = mix_i32(r, (int32_t)asr32(b, counts[i]));
        r = mix_i32(r, (int32_t)asr32(nb, counts[i]));
    }
    return r;
}

static uint64_t fold64(uint64_t h, uint64_t seed) {
    const uint8_t counts[7] = {0, 1, 63, 64, 65, 128, 255};
    const uint64_t a = (uint64_t)(UINT64_C(12297829382473034410) ^ seed);
    const uint64_t b = (uint64_t)(UINT64_C(12297829382473034410) ^ seed);
    const uint64_t nb = (uint64_t)(UINT64_C(0) - b);
    uint64_t r = h;
    for (size_t i = 0; i < 7; i++) {
        r = mix_u64(r, shl64(a, counts[i]));
        r = mix_u64(r, shr64(a, counts[i]));
        r = mix_i64(r, (int64_t)asr64(b, counts[i]));
        r = mix_i64(r, (int64_t)asr64(nb, counts[i]));
    }
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = fold8(h, seed);
    h = fold32(h, seed);
    h = fold64(h, seed);
    return h;
}
