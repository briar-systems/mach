#include "corpus.h"

/* the loops are scalar here; every shift saturates at or above the lane width
 * as mach's does, a signed count is read as its unsigned bits, which puts a
 * negative one past the width, and a signed lane's arithmetic shift reads its
 * sign through the two's-complement identity. */

static uint32_t shl32(uint32_t x, uint32_t c) { return (c >= 32u) ? 0u : (uint32_t)(x << c); }
static uint32_t shr32(uint32_t x, uint32_t c) { return (c >= 32u) ? 0u : (x >> c); }
static uint64_t shl64(uint64_t x, uint64_t c) { return (c >= 64u) ? 0u : (x << c); }
static uint64_t shr64(uint64_t x, uint64_t c) { return (c >= 64u) ? 0u : (x >> c); }
static uint8_t shl8(uint8_t x, uint8_t c) { return (c >= 8u) ? (uint8_t)0 : (uint8_t)((uint32_t)x << c); }

static uint32_t sar32(uint32_t x, uint32_t c) {
    const int neg = (x >> 31u) != 0;
    if (c >= 32u) { return neg ? UINT32_C(0xFFFFFFFF) : 0u; }
    return neg ? (uint32_t)~(((uint32_t)~x) >> c) : (x >> c);
}
static uint16_t sar16(uint16_t x, uint32_t c) {
    const int neg = (x >> 15u) != 0;
    if (c >= 16u) { return neg ? (uint16_t)UINT32_C(0xFFFF) : (uint16_t)0; }
    return neg ? (uint16_t)~(uint16_t)(((uint16_t)~x) >> c) : (uint16_t)(x >> c);
}
static int32_t sx32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static int16_t sx16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t n = (uint64_t)(UINT64_C(67) - (seed & UINT64_C(1)));
    const uint32_t r = (uint32_t)(seed >> 40u);
    uint64_t i;
    uint64_t j;

    const uint32_t c32[12] = { 0u, 1u, 5u, 31u, 32u, 33u, 63u, 64u, 255u, 256u, 257u, UINT32_C(4294967295) };
    const uint64_t c64[8] = { 0u, 1u, 63u, 64u, 65u, 257u, UINT64_C(4294967297), UINT64_C(18446744073709551615) };
    const uint8_t c8[8] = { 0u, 1u, 7u, 8u, 9u, 15u, 16u, 255u };

    uint32_t a[67];
    uint32_t b[67];
    uint32_t s[67];
    uint32_t sb[67];
    uint64_t q[67];
    uint64_t t[67];
    uint16_t w[67];
    uint8_t y[67];
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        a[i] = (uint32_t)((uint32_t)((uint32_t)((uint32_t)i * UINT32_C(2654435761)) + UINT32_C(12345)) + (uint32_t)seed);
        b[i] = (uint32_t)((uint32_t)((uint32_t)i * UINT32_C(2246822519)) + UINT32_C(3266489917));
        s[i] = (uint32_t)(c32[i % 12u] + r);
        sb[i] = (uint32_t)(c32[(i + 5u) % 12u] + r);
        q[i] = (uint64_t)((uint64_t)(i * UINT64_C(11400714819323198485)) + UINT64_C(1442695040888963407));
        t[i] = (uint64_t)(c64[i % 8u] + (uint64_t)r);
        w[i] = (uint16_t)((uint32_t)((uint32_t)i * UINT32_C(40503)) + 7u);
        y[i] = (uint8_t)((uint32_t)((uint32_t)i * 97u) + 3u);
    }

    for (j = 0; j < 12u; j++) {
        const uint32_t k = (uint32_t)(c32[j] + r);
        for (i = 0; i < n; i++) { h = mix_u32(h, shl32(a[i], k)); }
        for (i = 0; i < n; i++) { h = mix_u32(h, shr32(a[i], k)); }
        for (i = 0; i < n; i++) { h = mix_i32(h, sx32(sar32(b[i], k))); }
    }
    for (j = 0; j < 8u; j++) {
        const uint64_t k = (uint64_t)(c64[j] + (uint64_t)r);
        for (i = 0; i < n; i++) { h = mix_u64(h, shl64(q[i], k)); }
        for (i = 0; i < n; i++) { h = mix_u64(h, shr64(q[i], k)); }
        const uint8_t kb = (uint8_t)(c8[j] + (uint8_t)r);
        for (i = 0; i < n; i++) { h = mix_i16(h, sx16(sar16(w[i], kb))); }
        for (i = 0; i < n; i++) { h = mix_u8(h, shl8(y[i], kb)); }
    }

    for (i = 0; i < n; i++) { h = mix_u32(h, shl32(a[i], s[i])); }
    for (i = 0; i < n; i++) { h = mix_u32(h, shr32(a[i], s[i])); }
    for (i = 0; i < n; i++) { h = mix_i32(h, sx32(sar32(b[i], sb[i]))); }
    for (i = 0; i < n; i++) { h = mix_u64(h, shr64(q[i], t[i])); }
    return h;
}
