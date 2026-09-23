#include "corpus.h"

/* the lanes are held unsigned; every shift saturates at or above the lane
 * width as mach's does, the multiply is modular, and a signed lane's
 * arithmetic shift reads its sign through the two's-complement identity. */

static uint32_t shr32(uint32_t x, uint32_t c) { return (c >= 32u) ? 0u : (x >> c); }
static uint32_t shl32(uint32_t x, uint32_t c) { return (c >= 32u) ? 0u : (uint32_t)(x << c); }
static uint64_t shr64(uint64_t x, uint64_t c) { return (c >= 64u) ? 0u : (x >> c); }
static uint64_t shl64(uint64_t x, uint64_t c) { return (c >= 64u) ? 0u : (x << c); }

static uint16_t shl16(uint16_t x, uint16_t c) { return (c >= 16u) ? (uint16_t)0 : (uint16_t)((uint32_t)x << c); }
static uint16_t sar16(uint16_t x, uint16_t c) {
    const int neg = (x >> 15u) != 0;
    if (c >= 16u) { return neg ? (uint16_t)UINT64_C(65535) : (uint16_t)0; }
    return neg ? (uint16_t)~(uint16_t)(((uint16_t)~x) >> c) : (uint16_t)(x >> c);
}
static int16_t sx16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }

static uint32_t mix_lane(uint32_t x) {
    uint32_t y = x ^ shr32(x, 16u);
    y = (uint32_t)(y * UINT32_C(0x7FEB352D));
    y = y ^ shr32(y, 15u);
    y = (uint32_t)(y * UINT32_C(0x846CA68B));
    return y ^ shr32(y, 16u);
}

static uint32_t mix_lane_by(uint32_t x, uint32_t a, uint32_t b) {
    uint32_t y = x ^ shr32(x, a);
    y = (uint32_t)(y * UINT32_C(0x7FEB352D));
    return y ^ shl32(y, b);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint32_t x[4] = { UINT32_C(0x9E3779B9), UINT32_C(0x85EBCA6B), UINT32_C(0xC2B2AE35), UINT32_C(0x27D4EB2F) };
    x[0] = (uint32_t)(x[0] + (uint32_t)seed);
    for (unsigned i = 0; i < 8u; i++) {
        for (unsigned l = 0; l < 4u; l++) { x[l] = mix_lane(x[l]); }
        for (unsigned l = 0; l < 4u; l++) { h = mix_u32(h, x[l]); }
    }
    uint32_t n = (uint32_t)((uint32_t)seed + 13u);
    for (unsigned i = 0; i < 8u; i++) {
        for (unsigned l = 0; l < 4u; l++) { x[l] = mix_lane_by(x[l], n, (uint32_t)(n + 4u)); }
        for (unsigned l = 0; l < 4u; l++) { h = mix_u32(h, x[l]); }
        n = (uint32_t)(n + 3u);
    }
    uint64_t r[2] = { UINT64_C(0x0123456789ABCDEF), UINT64_C(0xFEDCBA9876543210) };
    r[1] = r[1] + seed;
    for (uint64_t k = seed + 1u; k < 64u; k += 7u) {
        const uint64_t back = (uint64_t)(64u - k);
        for (unsigned l = 0; l < 2u; l++) { r[l] = shl64(r[l], k) | shr64(r[l], back); }
        for (unsigned l = 0; l < 2u; l++) { h = mix_u64(h, r[l]); }
    }
    uint16_t s[8] = { UINT16_C(32768), UINT16_C(32767), UINT16_C(65535), UINT16_C(1), UINT16_C(65236), UINT16_C(300), UINT16_C(65529), UINT16_C(7) };
    s[0] = (uint16_t)((uint32_t)s[0] + (uint32_t)(uint16_t)seed);
    for (int16_t j = sx16((uint16_t)seed); j < 20; j = (int16_t)(j + 3)) {
        for (unsigned l = 0; l < 8u; l++) {
            s[l] = (uint16_t)((uint32_t)sar16(s[l], (uint16_t)j) + (uint32_t)shl16(s[l], 3u));
        }
        for (unsigned l = 0; l < 8u; l++) { h = mix_i16(h, sx16(s[l])); }
    }
    return h;
}
