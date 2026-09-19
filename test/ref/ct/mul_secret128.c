#include "corpus.h"

static uint64_t hw_hi(uint64_t a, uint64_t b) { return (uint64_t)(((corpus_u128)a * (corpus_u128)b) >> 64); }
static uint64_t hw_lo(uint64_t a, uint64_t b) { return (uint64_t)((corpus_u128)a * (corpus_u128)b); }
static corpus_u128 hw_wide(uint64_t a, uint64_t b) { return (corpus_u128)a * (corpus_u128)b; }

static uint64_t ref_hi(uint64_t a, uint64_t b) {
    const uint64_t al = a & UINT64_C(0xFFFFFFFF);
    const uint64_t ah = a >> 32;
    const uint64_t bl = b & UINT64_C(0xFFFFFFFF);
    const uint64_t bh = b >> 32;
    const uint64_t ll = al * bl;
    const uint64_t lh = al * bh;
    const uint64_t hl = ah * bl;
    const uint64_t hh = ah * bh;
    const uint64_t mid = (ll >> 32) + (lh & UINT64_C(0xFFFFFFFF)) + (hl & UINT64_C(0xFFFFFFFF));
    return hh + (lh >> 32) + (hl >> 32) + (mid >> 32);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t ops[8] = {0, 1, UINT64_C(18446744073709551615), UINT64_C(9223372036854775808),
        UINT64_C(6510615555426900570) ^ seed, UINT64_C(14106333703424951235) ^ seed, UINT64_C(7) ^ seed,
        UINT64_C(18446744073709551614) ^ seed};
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            h = mix_u64(h, hw_hi(ops[i], ops[j]));
            h = mix_u64(h, ref_hi(ops[i], ops[j]));
            h = mix_u64(h, hw_lo(ops[i], ops[j]));
            h = mix_u64(h, ops[i] * ops[j]);
            h = mix_u128(h, hw_wide(ops[i], ops[j]));
        }
    }
    return h;
}
