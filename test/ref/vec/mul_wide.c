#include "corpus.h"

/* each lane widens to 64 bits by its signedness before it multiplies, and the wide
 * lane is the low half of that unsigned product, read back through the
 * two's-complement identity for a signed result. */

static uint64_t widen(char s, unsigned bits, uint64_t v) {
    uint64_t m = (UINT64_C(1) << bits) - 1u;
    uint64_t x = v & m;
    if (s == 'i' && (x >> (bits - 1u)) != 0) { x |= ~m; }
    return x;
}

static uint64_t fold_lane(uint64_t h, char s, unsigned w, uint64_t v) {
    if (s == 'u') {
        if (w == 16u) { return mix_u16(h, (uint16_t)v); }
        if (w == 32u) { return mix_u32(h, (uint32_t)v); }
        return mix_u64(h, v);
    }
    if (w == 16u) { uint16_t b = (uint16_t)v; int16_t r; memcpy(&r, &b, sizeof r); return mix_i16(h, r); }
    if (w == 32u) { uint32_t b = (uint32_t)v; int32_t r; memcpy(&r, &b, sizeof r); return mix_i32(h, r); }
    { int64_t r; memcpy(&r, &v, sizeof r); return mix_i64(h, r); }
}

static uint64_t probe(uint64_t h, char sa, char sb, char rs, unsigned bits, unsigned lanes,
    const uint64_t *a, const uint64_t *b) {
    for (unsigned i = 0; i < lanes; i++) {
        uint64_t p = (uint64_t)(widen(sa, bits, a[i]) * widen(sb, bits, b[i]));
        h = fold_lane(h, rs, bits * 2u, p);
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    {
        uint64_t a[8] = { UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(0), UINT64_C(1), UINT64_C(129), UINT64_C(126), UINT64_C(7) };
        uint64_t b[8] = { UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(5), UINT64_C(127), UINT64_C(127), UINT64_C(128), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'i', 'i', 'i', 8u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(127), UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(2), UINT64_C(127), UINT64_C(128) };
        uint64_t b[8] = { UINT64_C(127), UINT64_C(127), UINT64_C(128), UINT64_C(129), UINT64_C(11), UINT64_C(2), UINT64_C(128), UINT64_C(124) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'i', 'i', 'i', 8u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(5), UINT64_C(127), UINT64_C(127), UINT64_C(128), UINT64_C(9) };
        uint64_t b[8] = { UINT64_C(127), UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(2), UINT64_C(127), UINT64_C(128) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'i', 'i', 'i', 8u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(0), UINT64_C(255), UINT64_C(254), UINT64_C(0), UINT64_C(1), UINT64_C(1), UINT64_C(254), UINT64_C(7) };
        uint64_t b[8] = { UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(5), UINT64_C(255), UINT64_C(255), UINT64_C(0), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(255), UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(254), UINT64_C(2), UINT64_C(255), UINT64_C(0) };
        uint64_t b[8] = { UINT64_C(255), UINT64_C(255), UINT64_C(0), UINT64_C(1), UINT64_C(11), UINT64_C(2), UINT64_C(0), UINT64_C(252) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(5), UINT64_C(255), UINT64_C(255), UINT64_C(0), UINT64_C(9) };
        uint64_t b[8] = { UINT64_C(255), UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(254), UINT64_C(2), UINT64_C(255), UINT64_C(0) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 8u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(0) };
        uint64_t b[4] = { UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(32767), UINT64_C(32768), UINT64_C(32768), UINT64_C(32767) };
        uint64_t b[4] = { UINT64_C(32767), UINT64_C(32767), UINT64_C(32768), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(5) };
        uint64_t b[4] = { UINT64_C(32767), UINT64_C(32768), UINT64_C(32768), UINT64_C(32767) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(0), UINT64_C(65535), UINT64_C(65534), UINT64_C(0) };
        uint64_t b[4] = { UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(65535), UINT64_C(0), UINT64_C(0), UINT64_C(65535) };
        uint64_t b[4] = { UINT64_C(65535), UINT64_C(65535), UINT64_C(0), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(5) };
        uint64_t b[4] = { UINT64_C(65535), UINT64_C(0), UINT64_C(0), UINT64_C(65535) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 4u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(2147483648), UINT64_C(2147483647) };
        uint64_t b[2] = { UINT64_C(2147483648), UINT64_C(2147483648) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'i', 'i', 'i', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(2147483647), UINT64_C(2147483648) };
        uint64_t b[2] = { UINT64_C(2147483647), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'i', 'i', 'i', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(2147483648), UINT64_C(2147483648) };
        uint64_t b[2] = { UINT64_C(2147483647), UINT64_C(2147483648) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'i', 'i', 'i', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(0), UINT64_C(4294967295) };
        uint64_t b[2] = { UINT64_C(0), UINT64_C(0) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'u', 'u', 'u', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(4294967295), UINT64_C(0) };
        uint64_t b[2] = { UINT64_C(4294967295), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'u', 'u', 'u', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(0), UINT64_C(0) };
        uint64_t b[2] = { UINT64_C(4294967295), UINT64_C(0) };
        a[1] = (a[1] ^ seed) & UINT64_C(4294967295);
        h = probe(h, 'u', 'u', 'u', 32u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(32768), UINT64_C(32767) };
        uint64_t b[2] = { UINT64_C(32768), UINT64_C(32768) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(32767), UINT64_C(32768) };
        uint64_t b[2] = { UINT64_C(32767), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 2u, a, b);
    }
    {
        uint64_t a[2] = { UINT64_C(32768), UINT64_C(32768) };
        uint64_t b[2] = { UINT64_C(32767), UINT64_C(32768) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 2u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(0), UINT64_C(255), UINT64_C(254), UINT64_C(0) };
        uint64_t b[4] = { UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(255), UINT64_C(0), UINT64_C(0), UINT64_C(255) };
        uint64_t b[4] = { UINT64_C(255), UINT64_C(255), UINT64_C(0), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(0), UINT64_C(0), UINT64_C(255), UINT64_C(5) };
        uint64_t b[4] = { UINT64_C(255), UINT64_C(0), UINT64_C(0), UINT64_C(255) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'u', 'u', 'u', 8u, 4u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(0), UINT64_C(1), UINT64_C(32769), UINT64_C(32766), UINT64_C(7) };
        uint64_t b[8] = { UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(5), UINT64_C(32767), UINT64_C(32767), UINT64_C(32768), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(32767), UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(2), UINT64_C(32767), UINT64_C(32768) };
        uint64_t b[8] = { UINT64_C(32767), UINT64_C(32767), UINT64_C(32768), UINT64_C(32769), UINT64_C(11), UINT64_C(2), UINT64_C(32768), UINT64_C(32764) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(5), UINT64_C(32767), UINT64_C(32767), UINT64_C(32768), UINT64_C(9) };
        uint64_t b[8] = { UINT64_C(32767), UINT64_C(32768), UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(2), UINT64_C(32767), UINT64_C(32768) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'i', 'i', 16u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(0), UINT64_C(65535), UINT64_C(65534), UINT64_C(0), UINT64_C(1), UINT64_C(1), UINT64_C(65534), UINT64_C(7) };
        uint64_t b[8] = { UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(5), UINT64_C(65535), UINT64_C(65535), UINT64_C(0), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(65535), UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(65534), UINT64_C(2), UINT64_C(65535), UINT64_C(0) };
        uint64_t b[8] = { UINT64_C(65535), UINT64_C(65535), UINT64_C(0), UINT64_C(1), UINT64_C(11), UINT64_C(2), UINT64_C(0), UINT64_C(65532) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 8u, a, b);
    }
    {
        uint64_t a[8] = { UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(5), UINT64_C(65535), UINT64_C(65535), UINT64_C(0), UINT64_C(9) };
        uint64_t b[8] = { UINT64_C(65535), UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(65534), UINT64_C(2), UINT64_C(65535), UINT64_C(0) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'u', 'u', 'u', 16u, 8u, a, b);
    }
    {
        uint64_t a[16] = { UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(0), UINT64_C(1), UINT64_C(129), UINT64_C(126), UINT64_C(7), UINT64_C(128), UINT64_C(127), UINT64_C(3), UINT64_C(128), UINT64_C(100), UINT64_C(127), UINT64_C(130), UINT64_C(42) };
        uint64_t b[16] = { UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(5), UINT64_C(127), UINT64_C(127), UINT64_C(128), UINT64_C(9), UINT64_C(127), UINT64_C(127), UINT64_C(128), UINT64_C(129), UINT64_C(11), UINT64_C(2), UINT64_C(128), UINT64_C(124) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'i', 'i', 'i', 8u, 16u, a, b);
    }
    {
        uint64_t a[16] = { UINT64_C(127), UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(2), UINT64_C(127), UINT64_C(128), UINT64_C(0), UINT64_C(127), UINT64_C(128), UINT64_C(13), UINT64_C(127), UINT64_C(128), UINT64_C(1), UINT64_C(128) };
        uint64_t b[16] = { UINT64_C(127), UINT64_C(128), UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(2), UINT64_C(127), UINT64_C(128), UINT64_C(0), UINT64_C(127), UINT64_C(128), UINT64_C(13), UINT64_C(127), UINT64_C(128), UINT64_C(1), UINT64_C(128) };
        a[1] = (a[1] ^ seed) & UINT64_C(255);
        h = probe(h, 'i', 'i', 'i', 8u, 16u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(0) };
        uint64_t b[4] = { UINT64_C(0), UINT64_C(0), UINT64_C(65535), UINT64_C(5) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'u', 'i', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(32767), UINT64_C(32768), UINT64_C(32768), UINT64_C(32767) };
        uint64_t b[4] = { UINT64_C(65535), UINT64_C(0), UINT64_C(0), UINT64_C(65535) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'u', 'i', 16u, 4u, a, b);
    }
    {
        uint64_t a[4] = { UINT64_C(1), UINT64_C(32769), UINT64_C(32766), UINT64_C(7) };
        uint64_t b[4] = { UINT64_C(65535), UINT64_C(65535), UINT64_C(0), UINT64_C(9) };
        a[1] = (a[1] ^ seed) & UINT64_C(65535);
        h = probe(h, 'i', 'u', 'i', 16u, 4u, a, b);
    }
    return h;
}
