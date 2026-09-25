#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint64_t shl64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v << n); }
static inline uint64_t shr64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v >> n); }
static inline uint64_t asr64(uint64_t v, uint64_t n) {
    const uint64_t fill = (uint64_t)(UINT64_C(0) - (v >> 63));
    if (n >= 64u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint64_t)((v >> n) | (uint64_t)(fill << (64u - n)));
}
static inline uint32_t shl32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v << n); }
static inline uint32_t shr32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v >> n); }
static inline uint8_t shl8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v << n); }

static uint64_t below_width(uint64_t h, uint64_t a) {
    for (uint64_t i = 0; i < 64; i++) { h = mix_u64(h, shr64(a, i) ^ shl64(a, i)); }
    for (uint64_t j = 0; j <= 63; j++) { h = mix_u64(h, shl64(a, j)); }
    for (int64_t k = 0; k < 64; k++) { h = mix_i64(h, (int64_t)asr64(a, (uint64_t)k)); }
    for (uint8_t n = 0; n < 32; n++) { h = mix_u32(h, shl32((uint32_t)a, n)); }
    return h;
}

static uint64_t past_width(uint64_t h, uint64_t a, uint64_t top) {
    for (uint64_t i = 0; i < top; i++) {
        h = mix_u64(h, shr64(a, i) ^ shl64(a, i));
        h = mix_i64(h, (int64_t)asr64(a, i));
        h = mix_u32(h, shl32((uint32_t)a, i));
    }
    for (uint64_t j = 0; j <= 64; j++) { h = mix_u64(h, shl64(a, j)); }
    for (uint8_t n = 0; n < 40; n++) { h = mix_u32(h, shr32((uint32_t)a, n)); }
    return h;
}

static uint64_t guarded(uint64_t h, uint64_t a, uint64_t n) {
    if (n < 64) { h = mix_u64(h, shl64(a, n)); }
    if (n >= 64) { h = mix_u64(h, 1); }
    else         { h = mix_u64(h, shr64(a, n)); }
    h = mix_u64(h, shl64(a, n));
    return h;
}

static uint64_t differences(uint64_t h, uint64_t a) {
    for (uint64_t bit = 0; bit < 128; bit++) {
        if (bit < 64) { h = mix_u64(h, shr64(a, 63 - bit) & 1); }
        else          { h = mix_u64(h, shr64(a, 127 - bit) & 1); }
    }
    return h;
}

static uint64_t transpose(uint64_t h, uint64_t seed, uint64_t block) {
    uint64_t q[8];
    uint8_t input[16];
    for (uint64_t p = 0; p < 8; p++) { q[p] = 0; }
    for (uint64_t i = 0; i < 16; i++) { input[i] = (uint8_t)(seed * 37 + i * 11); }
    for (uint64_t i = 0; i < 16; i++) {
        const uint64_t byte = input[i];
        const uint64_t position = (i & 3) * 16 + (i >> 2) * 4 + block;
        for (uint64_t plane = 0; plane < 8; plane++) {
            q[plane] = q[plane] | shl64(shr64(byte, plane) & 1, position);
        }
    }
    for (uint64_t p = 0; p < 8; p++) { h = mix_u64(h, q[p]); }
    for (uint64_t i = 0; i < 16; i++) {
        const uint64_t position = (i & 3) * 16 + (i >> 2) * 4 + block;
        uint8_t byte = 0;
        for (uint64_t plane = 0; plane < 8; plane++) {
            byte = (uint8_t)(byte | shl8((uint8_t)(shr64(q[plane], position) & 1), plane));
        }
        h = mix_u8(h, byte);
    }
    return h;
}

static uint64_t transpose_first(uint64_t h, uint64_t seed) {
    uint64_t q[8];
    for (uint64_t p = 0; p < 8; p++) { q[p] = 0; }
    for (uint64_t i = 0; i < 16; i++) {
        const uint64_t byte = (seed * 37 + i * 11) & 255;
        const uint64_t position = (i & 3) * 16 + (i >> 2) * 4;
        for (uint64_t plane = 0; plane < 8; plane++) {
            q[plane] = q[plane] | shl64(shr64(byte, plane) & 1, position);
        }
    }
    for (uint64_t p = 0; p < 8; p++) { h = mix_u64(h, q[p]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t a = UINT64_C(0x9E3779B97F4A7C15) ^ seed;
    const uint64_t b = UINT64_C(0x8000000000000001) ^ (seed << 7);

    h = below_width(h, a);
    h = below_width(h, b);
    h = past_width(h, a, 72);
    h = past_width(h, b, 64);
    h = guarded(h, a, 5);
    h = guarded(h, a, 63);
    h = guarded(h, a, 64);
    h = guarded(h, b, 200);
    h = differences(h, a);
    h = differences(h, b);
    h = transpose_first(h, seed);
    h = transpose(h, seed, 0);
    h = transpose(h, seed, 3);
    h = transpose(h, seed, 4);
    h = transpose(h, seed, 9);
    return h;
}
