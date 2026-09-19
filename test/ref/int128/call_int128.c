#include "corpus.h"

static corpus_u128 ret_wide(uint64_t lo, uint64_t hi) { return ((corpus_u128)hi << 64) | (corpus_u128)lo; }
static corpus_i128 ret_signed(int64_t v) { return (corpus_i128)v * 3; }
static corpus_u128 odd_then_wide(uint64_t a, corpus_u128 b, uint64_t c) { return b + (corpus_u128)a - (corpus_u128)c; }
static corpus_u128 three_wide(corpus_u128 a, corpus_u128 b, corpus_u128 c, uint64_t d) {
    return a ^ (b << 1) ^ (c >> 1) ^ (corpus_u128)d;
}
static corpus_u128 six_then_wide(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f,
    corpus_u128 g, uint64_t h) {
    return g + (corpus_u128)(a + b + c + d + e + f + h);
}
static corpus_u128 seven_then_wide(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f,
    uint64_t g, corpus_u128 w, uint64_t z) {
    return w - (corpus_u128)(a + b + c + d + e + f + g) + (corpus_u128)z;
}
static corpus_i128 eight_then_wide(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f,
    uint64_t g, uint64_t h, corpus_u128 w, corpus_i128 v) {
    corpus_i128 ws; memcpy(&ws, &w, 16);
    return v - ws + (corpus_i128)(a + b + c + d + e + f + g + h);
}
static corpus_i128 floats_between(double a, corpus_u128 b, float c, corpus_i128 d, double e) {
    corpus_i128 bs; memcpy(&bs, &b, 16);
    return d + bs + (corpus_i128)(int64_t)a + (corpus_i128)(int64_t)c - (corpus_i128)(int64_t)e;
}
static corpus_u128 many_wide(corpus_u128 a, corpus_u128 b, corpus_u128 c, corpus_u128 d, corpus_u128 e,
    corpus_u128 f, corpus_u128 g, corpus_u128 h, corpus_u128 i) {
    return a + (b << 1) + (c << 2) + (d << 3) + (e << 4) + (f << 5) + (g << 6) + (h << 7) + (i << 8);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;
    const corpus_u128 w = ret_wide(s ^ UINT64_C(0x0F0F0F0F0F0F0F0F), UINT64_C(0xFEDCBA9876543210) + s);
    const corpus_i128 n = ret_signed(-1 - (int64_t)s);
    h = mix_u128(h, w);
    h = mix_i128(h, n);
    h = mix_u128(h, odd_then_wide(s + 1, w, s + 2));
    h = mix_u128(h, three_wide(w, w + 1, w + 2, s));
    h = mix_u128(h, six_then_wide(s, s + 1, s + 2, s + 3, s + 4, s + 5, w, s + 6));
    h = mix_u128(h, seven_then_wide(s, s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, w, s + 7));
    h = mix_i128(h, eight_then_wide(s, s + 1, s + 2, s + 3, s + 4, s + 5, s + 6, s + 7, w, n));
    h = mix_i128(h, floats_between(1.5 + (double)s, w, 2.5f, n, -3.5));
    h = mix_u128(h, many_wide(w, w + 1, w + 2, w + 3, w + 4, w + 5, w + 6, w + 7, w + 8));
    return h;
}
