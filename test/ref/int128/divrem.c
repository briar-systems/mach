/* u128 and i128 quotients and remainders through the compiler's helpers.
 * one noinline part per type. */
#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

static uint64_t divrem_u128(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_u128 s = (corpus_u128)seed;

    const corpus_u128 big = U128(0x0123456789ABCDEF, 0xFEDCBA9876543210) + s;
    const corpus_u128 ds[8] = {1, 3 + s, UINT64_C(4294967311), UINT64_C(18446744073709551615) - s,
        ((corpus_u128)1 << 64) + s, ((corpus_u128)1 << 100) + 12345 + s, big >> 7, ~(corpus_u128)0 - s};
    const corpus_u128 ns[6] = {0, 7 + s, UINT64_C(18446744073709551615), big, ~(corpus_u128)0, big ^ (s << 77)};

    for (uint64_t i = 0; i < 6; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            const corpus_u128 q = ns[i] / ds[j];
            const corpus_u128 r = ns[i] % ds[j];
            h = mix_u128(h, q);
            h = mix_u128(h, r);
            h = mix_u128(h, q * ds[j] + r - ns[i]);
        }
    }

    corpus_u128 a = big;
    for (corpus_u128 k = 0; k < 10; k = k + 1) {
        a = a / (k + 2);
        h = mix_u128(h, a);
        h = mix_u128(h, big % (a + 1));
    }
    h = mix_u128(h, big / big);
    h = mix_u128(h, big % big);
    h = mix_u128(h, (big >> 64) / ((corpus_u128)(uint64_t)big + 1));
    return h;
}

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

static uint64_t divrem_i128(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_i128 s = (corpus_i128)(seed & 1023);

    const corpus_i128 imax = (corpus_i128)(((corpus_u128)1 << 127) - 1);
    const corpus_i128 imin = -imax - 1;
    const corpus_i128 big  = (corpus_i128)U128(0x0123456789ABCDEF, 0xFEDCBA9876543210) + s;

    const corpus_i128 ds[8] = {1, -1, 3 + s, -(3 + s), ((corpus_i128)1 << 64) + s, -(((corpus_i128)1 << 64) + s), big >> 9, imax - s};
    const corpus_i128 ns[7] = {0, 7 + s, -(7 + s), big, -big, imax, imin + s + 1};

    for (uint64_t i = 0; i < 7; i++) {
        for (uint64_t j = 0; j < 8; j++) {
            const corpus_i128 q = ns[i] / ds[j];
            const corpus_i128 r = ns[i] % ds[j];
            h = mix_i128(h, q);
            h = mix_i128(h, r);
            h = mix_i128(h, q * ds[j] + r - ns[i]);
        }
    }

    h = mix_i128(h, imin / 2);
    h = mix_i128(h, imin % 2);
    h = mix_i128(h, imin / imax);
    h = mix_i128(h, imin % imax);
    h = mix_i128(h, imin / (imin + 1));
    h = mix_i128(h, imax / -7);
    h = mix_i128(h, imax % -7);
    h = mix_i128(h, -7 / imax);
    h = mix_i128(h, -7 % imax);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, divrem_u128(seed));
    h = mix_u64(h, divrem_i128(seed));
    return h;
}
