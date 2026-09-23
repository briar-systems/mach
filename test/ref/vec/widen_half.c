#include "corpus.h"

/* each lane of the source is widened to twice its width by its signedness; the
 * low half takes lanes 0..n/2 and the high half the rest, in order. */

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

/* the lanes `first..first+count` of `v`, widened and folded in order */
static uint64_t half(uint64_t h, char s, unsigned bits, unsigned first, unsigned count, const uint64_t *v) {
    for (unsigned i = 0; i < count; i++) {
        h = fold_lane(h, s, bits * 2u, widen(s, bits, v[first + i]));
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    {
        uint64_t v[16] = { UINT64_C(128), UINT64_C(127), UINT64_C(255), UINT64_C(0), UINT64_C(1), UINT64_C(129), UINT64_C(126), UINT64_C(7),
            UINT64_C(128), UINT64_C(254), UINT64_C(3), UINT64_C(127), UINT64_C(129), UINT64_C(64), UINT64_C(192), UINT64_C(100) };
        v[1] = (v[1] ^ seed) & UINT64_C(255);
        h = half(h, 'i', 8u, 0u, 8u, v);
        h = half(h, 'i', 8u, 8u, 8u, v);
    }
    {
        uint64_t v[16] = { UINT64_C(0), UINT64_C(255), UINT64_C(254), UINT64_C(0), UINT64_C(1), UINT64_C(1), UINT64_C(254), UINT64_C(7),
            UINT64_C(128), UINT64_C(129), UINT64_C(3), UINT64_C(255), UINT64_C(2), UINT64_C(64), UINT64_C(192), UINT64_C(100) };
        v[1] = (v[1] ^ seed) & UINT64_C(255);
        h = half(h, 'u', 8u, 0u, 8u, v);
        h = half(h, 'u', 8u, 8u, 8u, v);
    }
    {
        uint64_t v[8] = { UINT64_C(32768), UINT64_C(32767), UINT64_C(65535), UINT64_C(0), UINT64_C(1), UINT64_C(32769), UINT64_C(32766), UINT64_C(65529) };
        v[1] = (v[1] ^ seed) & UINT64_C(65535);
        h = half(h, 'i', 16u, 0u, 4u, v);
        h = half(h, 'i', 16u, 4u, 4u, v);
        /* the permuted literal: lanes 0, 2, 1, 3 */
        uint64_t p[4] = { v[0], v[2], v[1], v[3] };
        h = half(h, 'i', 16u, 0u, 4u, p);
    }
    {
        uint64_t v[8] = { UINT64_C(0), UINT64_C(65535), UINT64_C(65534), UINT64_C(0), UINT64_C(1), UINT64_C(32768), UINT64_C(32767), UINT64_C(7) };
        v[1] = (v[1] ^ seed) & UINT64_C(65535);
        h = half(h, 'u', 16u, 0u, 4u, v);
        h = half(h, 'u', 16u, 4u, 4u, v);
    }
    {
        uint64_t v[4] = { UINT64_C(2147483648), UINT64_C(2147483647), UINT64_C(4294967295), UINT64_C(2147483649) };
        v[1] = (v[1] ^ seed) & UINT64_C(4294967295);
        h = half(h, 'i', 32u, 0u, 2u, v);
        h = half(h, 'i', 32u, 2u, 2u, v);
    }
    {
        uint64_t v[4] = { UINT64_C(0), UINT64_C(4294967295), UINT64_C(2147483648), UINT64_C(7) };
        v[1] = (v[1] ^ seed) & UINT64_C(4294967295);
        h = half(h, 'u', 32u, 0u, 2u, v);
        h = half(h, 'u', 32u, 2u, 2u, v);
    }
    return h;
}
