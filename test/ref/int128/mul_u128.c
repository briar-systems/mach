#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

static corpus_u128 wide(uint64_t a, uint64_t b) { return (corpus_u128)a * (corpus_u128)b; }
static uint64_t high(uint64_t a, uint64_t b) { return (uint64_t)(((corpus_u128)a * (corpus_u128)b) >> 64); }
static corpus_u128 low(corpus_u128 a, corpus_u128 b) { return a * b; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t ops[8] = {0, 1, UINT64_C(18446744073709551615), UINT64_C(9223372036854775808),
        UINT64_C(6510615555426900570) ^ seed, UINT64_C(14106333703424951235) ^ seed, UINT64_C(7) ^ seed,
        UINT64_C(18446744073709551614) ^ seed};
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            h = mix_u128(h, wide(ops[i], ops[j]));
            h = mix_u64(h, high(ops[i], ops[j]));
            h = mix_u64(h, (uint64_t)((corpus_u128)ops[i] * (corpus_u128)ops[j]));
        }
    }

    const corpus_u128 s = (corpus_u128)seed;
    corpus_u128 a = U128(0x0123456789ABCDEF, 0xFEDCBA9876543210) + s;
    for (corpus_u128 k = 0; k < 12; k = k + 1) {
        a = low(a, (k << 64) + 2 * k + 3);
        h = mix_u128(h, a);
    }
    const corpus_u128 b = ~(corpus_u128)0 - s;
    h = mix_u128(h, low(b, b));
    h = mix_u128(h, low(b, a));
    h = mix_u128(h, low(a, b));
    h = mix_u128(h, a * a);
    h = mix_u128(h, a * ~(corpus_u128)0);
    h = mix_u128(h, a * ((corpus_u128)1 << 64));
    return h;
}
