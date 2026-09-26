/* the unsigned and signed 128-bit products: widening, high half and full product.
 * one noinline part per type. */
#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

static corpus_u128 wide_mul_u128(uint64_t a, uint64_t b) { return (corpus_u128)a * (corpus_u128)b; }
static uint64_t high_mul_u128(uint64_t a, uint64_t b) { return (uint64_t)(((corpus_u128)a * (corpus_u128)b) >> 64); }
static corpus_u128 low_mul_u128(corpus_u128 a, corpus_u128 b) { return a * b; }

static uint64_t mul_u128(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t ops[8] = {0, 1, UINT64_C(18446744073709551615), UINT64_C(9223372036854775808),
        UINT64_C(6510615555426900570) ^ seed, UINT64_C(14106333703424951235) ^ seed, UINT64_C(7) ^ seed,
        UINT64_C(18446744073709551614) ^ seed};
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            h = mix_u128(h, wide_mul_u128(ops[i], ops[j]));
            h = mix_u64(h, high_mul_u128(ops[i], ops[j]));
            h = mix_u64(h, (uint64_t)((corpus_u128)ops[i] * (corpus_u128)ops[j]));
        }
    }

    const corpus_u128 s = (corpus_u128)seed;
    corpus_u128 a = U128(0x0123456789ABCDEF, 0xFEDCBA9876543210) + s;
    for (corpus_u128 k = 0; k < 12; k = k + 1) {
        a = low_mul_u128(a, (k << 64) + 2 * k + 3);
        h = mix_u128(h, a);
    }
    const corpus_u128 b = ~(corpus_u128)0 - s;
    h = mix_u128(h, low_mul_u128(b, b));
    h = mix_u128(h, low_mul_u128(b, a));
    h = mix_u128(h, low_mul_u128(a, b));
    h = mix_u128(h, a * a);
    h = mix_u128(h, a * ~(corpus_u128)0);
    h = mix_u128(h, a * ((corpus_u128)1 << 64));
    return h;
}

static corpus_i128 wide_mul_i128(int64_t a, int64_t b) { return (corpus_i128)a * (corpus_i128)b; }
static int64_t high_mul_i128(int64_t a, int64_t b) { return (int64_t)(((corpus_i128)a * (corpus_i128)b) >> 64); }

static uint64_t mul_i128(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)(seed & 1023);

    const int64_t ops[8] = {0, 1, -1, -INT64_C(9223372036854775807) - 1, INT64_C(9223372036854775807) - s,
        -INT64_C(6510615555426900570) + s, 7 + s, -7 - s};
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            h = mix_i128(h, wide_mul_i128(ops[i], ops[j]));
            h = mix_i64(h, high_mul_i128(ops[i], ops[j]));
            h = mix_i64(h, (int64_t)((corpus_i128)ops[i] * (corpus_i128)ops[j]));
        }
    }

    corpus_i128 a = -(corpus_i128)INT64_C(0x7EDCBA9876543210) - (corpus_i128)s;
    for (corpus_i128 k = 1; k < 6; k = k + 1) {
        const corpus_i128 p = a * (k - 3);
        h = mix_i128(h, p);
        h = mix_i128(h, (k - 3) * a);
        h = mix_i128(h, p >> 64);
    }
    const corpus_i128 b = ((corpus_i128)1 << 62) + (corpus_i128)s;
    h = mix_i128(h, b * b);
    h = mix_i128(h, b * -b);
    h = mix_i128(h, -b * -b);
    h = mix_i128(h, a * 4);
    h = mix_i128(h, a * -4);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, mul_u128(seed));
    h = mix_u64(h, mul_i128(seed));
    return h;
}
