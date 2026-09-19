#include "corpus.h"

/* mach compares mathematical values across signedness; C's usual arithmetic
 * conversions do not, so the mixed comparisons are spelled out: a negative
 * signed value is below every unsigned one, and otherwise both are compared
 * as the unsigned type */
static int lt_us(corpus_u128 a, corpus_i128 b) { return b < 0 ? 0 : a < (corpus_u128)b; }
static int eq_us(corpus_u128 a, corpus_i128 b) { return b < 0 ? 0 : a == (corpus_u128)b; }
static int lt_su(corpus_i128 a, corpus_u128 b) { return a < 0 ? 1 : (corpus_u128)a < b; }

static uint64_t fold_pair_u(uint64_t h, corpus_u128 a, corpus_u128 b) {
    h = mix_u64(h, a == b);
    h = mix_u64(h, a != b);
    h = mix_u64(h, a < b);
    h = mix_u64(h, a <= b);
    h = mix_u64(h, a > b);
    h = mix_u64(h, a >= b);
    return h;
}

static uint64_t fold_pair_s(uint64_t h, corpus_i128 a, corpus_i128 b) {
    h = mix_u64(h, a == b);
    h = mix_u64(h, a != b);
    h = mix_u64(h, a < b);
    h = mix_u64(h, a <= b);
    h = mix_u64(h, a > b);
    h = mix_u64(h, a >= b);
    return h;
}

static uint64_t fold_pair_us(uint64_t h, corpus_u128 a, corpus_i128 b) {
    h = mix_u64(h, eq_us(a, b));
    h = mix_u64(h, !eq_us(a, b));
    h = mix_u64(h, lt_us(a, b));
    h = mix_u64(h, lt_us(a, b) || eq_us(a, b));
    h = mix_u64(h, lt_su(b, a));
    h = mix_u64(h, lt_su(b, a) || eq_us(a, b));
    h = mix_u64(h, lt_su(b, a));
    h = mix_u64(h, !lt_su(b, a));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_u128 s = (corpus_u128)seed;
    const corpus_i128 t = (corpus_i128)seed;

    const corpus_u128 u[6] = {0, s, ((corpus_u128)1 << 64) + s, ((corpus_u128)1 << 64) - 1 - s,
        ((corpus_u128)1 << 127) + s, ~(corpus_u128)0 - s};
    const corpus_i128 imax = (corpus_i128)(((corpus_u128)1 << 127) - 1);
    const corpus_i128 v[6] = {0, t, ((corpus_i128)1 << 64) + t, -1 - t, -((corpus_i128)1 << 64) - t,
        -imax - 1 + t};

    for (uint64_t i = 0; i < 6; i++) {
        for (uint64_t j = 0; j < 6; j++) {
            h = fold_pair_u(h, u[i], u[j]);
            h = fold_pair_s(h, v[i], v[j]);
            h = fold_pair_us(h, u[i], v[j]);
        }
    }

    const uint64_t w64 = UINT64_C(18446744073709551615) - seed;
    const int64_t n64 = -1 - (int64_t)seed;
    for (uint64_t i = 0; i < 6; i++) {
        h = mix_u64(h, u[i] < (corpus_u128)w64);
        h = mix_u64(h, u[i] == (corpus_u128)w64);
        h = mix_u64(h, (corpus_u128)w64 <= u[i]);
        h = mix_u64(h, v[i] < (corpus_i128)n64);
        h = mix_u64(h, v[i] == (corpus_i128)n64);
        h = mix_u64(h, (corpus_i128)n64 >= v[i]);
        h = mix_u64(h, lt_su((corpus_i128)n64, u[i]));
        h = mix_u64(h, lt_su(v[i], (corpus_u128)w64));
    }
    return h;
}
