#include "corpus.h"

static corpus_i128 wide(int64_t a, int64_t b) { return (corpus_i128)a * (corpus_i128)b; }
static int64_t high(int64_t a, int64_t b) { return (int64_t)(((corpus_i128)a * (corpus_i128)b) >> 64); }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)(seed & 1023);

    const int64_t ops[8] = {0, 1, -1, -INT64_C(9223372036854775807) - 1, INT64_C(9223372036854775807) - s,
        -INT64_C(6510615555426900570) + s, 7 + s, -7 - s};
    for (uint64_t i = 0; i < 8; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            h = mix_i128(h, wide(ops[i], ops[j]));
            h = mix_i64(h, high(ops[i], ops[j]));
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
