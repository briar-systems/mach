#include "corpus.h"

/* mach's u8x16 is sixteen u8 lanes, added lane by lane with wraparound. */
typedef struct { _Alignas(16) uint8_t l[16]; } u8x16;

static u8x16 pick(corpus_u128 x, u8x16 v) {
    if (x > 0xffff) { return v; }
    for (unsigned i = 0; i < 16u; i++) { v.l[i] = (uint8_t)(v.l[i] + v.l[i]); }
    return v;
}

static corpus_u128 widen(u8x16 v, corpus_u128 x) {
    return x + (corpus_u128)v.l[0] + ((corpus_u128)v.l[15] << 64);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    u8x16 v = {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
    v.l[0] = (uint8_t)(v.l[0] + (uint8_t)seed);
    corpus_u128 big = ((((corpus_u128)UINT64_C(0x0102030405060708)) << 64) | UINT64_C(0x090a0b0c0d0e0f10)) + (corpus_u128)seed;
    u8x16 a = pick(big, v);
    u8x16 b = pick((corpus_u128)seed, v);
    h = mix_u64(h, a.l[0]);
    h = mix_u64(h, a.l[15]);
    h = mix_u64(h, b.l[0]);
    h = mix_u64(h, b.l[15]);
    corpus_u128 w = widen(v, big);
    h = mix_u128(h, w);
    h = mix_u64(h, w == big);
    return h;
}
