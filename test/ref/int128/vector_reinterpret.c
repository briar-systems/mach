#include "corpus.h"

/* mach's u8x16 and u32x4 are 16 bytes, reinterpreted as a little-endian u128
 * byte for byte. a comparison mask lane is all-ones for true and all-zeros for
 * false, built explicitly here. */
typedef struct { _Alignas(16) uint8_t l[16]; } u8x16;
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;

static corpus_u128 as_u128(const void *p) {
    corpus_u128 r;
    memcpy(&r, p, 16);
    return r;
}

static uint64_t hits(u8x16 a, u8x16 b) {
    for (unsigned i = 0; i < 16u; i++) { if (a.l[i] == b.l[i]) { return 1; } }
    return 0;
}

static corpus_u128 mask_of(u8x16 a, u8x16 b) {
    u8x16 m;
    for (unsigned i = 0; i < 16u; i++) { m.l[i] = (uint8_t)(a.l[i] == b.l[i] ? 0xFF : 0); }
    return as_u128(&m);
}

static u8x16 spread(corpus_u128 x) {
    u8x16 v;
    memcpy(&v, &x, 16);
    return v;
}

static u32x4 add_shifted(u32x4 v, corpus_u128 x) {
    corpus_u128 y = x >> 32;
    u32x4 t;
    memcpy(&t, &y, 16);
    for (unsigned i = 0; i < 4u; i++) { v.l[i] = (uint32_t)(v.l[i] + t.l[i]); }
    return v;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    u8x16 a = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
    u8x16 b[3] = {
        {{0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16}},
    };
    a.l[0] = (uint8_t)(a.l[0] + (uint8_t)seed);
    for (unsigned k = 0; k < 3u; k++) {
        h = mix_u64(h, hits(a, b[k]));
        corpus_u128 m = mask_of(a, b[k]);
        h = mix_u128(h, m);
        h = mix_u64(h, m == 0);
        h = mix_u64(h, m > 0xffff);
    }
    corpus_u128 big = ((((corpus_u128)UINT64_C(0x0102030405060708)) << 64) | UINT64_C(0x090a0b0c0d0e0f10)) + (corpus_u128)seed;
    u8x16 v = spread(big);
    h = mix_u64(h, v.l[0]);
    h = mix_u64(h, v.l[7]);
    h = mix_u64(h, v.l[8]);
    h = mix_u64(h, v.l[15]);
    h = mix_u128(h, as_u128(&v));
    h = mix_u64(h, as_u128(&v) == big);
    u32x4 one = {{1, 2, 3, 4}};
    u32x4 w = add_shifted(one, big);
    for (unsigned j = 0; j < 4u; j++) { h = mix_u64(h, w.l[j]); }
    return h;
}
