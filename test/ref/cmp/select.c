#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t x = seed ^ UINT64_C(0x9E3779B97F4A7C15);

    int32_t mn = INT32_C(0x7FFFFFFF);
    uint16_t mx = 0;
    int64_t sa = 0;
    int8_t sc = 0;
    uint64_t ac = 0;
    uint32_t bits = 0;

    for (uint32_t k = 0; k < UINT32_C(200); k++) {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        const int32_t w = (int32_t)(x & UINT64_C(0x7FFF)) - INT32_C(0x4000);
        const uint16_t u = (uint16_t)((x >> 16) & UINT64_C(0xFFFF));
        const int8_t b = (int8_t)((int32_t)((x >> 32) & UINT64_C(0x7F)) - INT32_C(64));

        if (w < mn) { mn = w; }
        if (u > mx) { mx = u; }
        int64_t a = (int64_t)w;
        if (a < 0) { a = 0 - a; }
        sa = sa + a;
        int8_t c = b;
        if (c < -20) { c = -20; }
        else if (c > 20) { c = 20; }
        sc = (int8_t)(sc ^ c);
        if (u >= UINT16_C(0x8000)) { ac = ac + (uint64_t)u; }
        uint32_t one = 0;
        if (w == 7 || u < UINT16_C(0x4000)) { one = 1; }
        uint32_t zero = 1;
        if (100 < w) { zero = 0; }
        bits = bits + one + zero;

        h = mix_i32(h, mn);
        h = mix_u16(h, mx);
        h = mix_i64(h, sa);
        h = mix_i8(h, sc);
        h = mix_u64(h, ac);
        h = mix_u32(h, bits);
    }

    return h;
}
