#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)(uint64_t)(hi) << 64) | (corpus_u128)(uint64_t)(lo))

static corpus_u128 through_u128(corpus_u128 x) { return x; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_u128 s = (corpus_u128)seed;

    const corpus_u128 p2_64 = U128(1, 0);
    const corpus_u128 e19 = U128(0, UINT64_C(10000000000000000000));
    const corpus_u128 neg_2p64 = U128(UINT64_C(0xFFFFFFFFFFFFFFFF), 0);
    const corpus_u128 u128_max = U128(UINT64_C(0xFFFFFFFFFFFFFFFF), UINT64_C(0xFFFFFFFFFFFFFFFF));
    const corpus_u128 i128_max = U128(UINT64_C(0x7FFFFFFFFFFFFFFF), UINT64_C(0xFFFFFFFFFFFFFFFF));
    const corpus_u128 i128_min = U128(UINT64_C(0x8000000000000000), 0);
    const corpus_u128 p2_100 = U128(UINT64_C(1) << 36, 0);

    h = mix_u8(h, 1);

    /* the signed values fold by their two's-complement pattern, so unsigned
     * arithmetic on the pattern is the same fold without signed overflow */
    for (int pass = 0; pass < 2; pass++) {
        h = mix_u128(h, (corpus_u128)(through_u128(p2_64) + s));
        h = mix_u128(h, (corpus_u128)(through_u128(e19) + s));
        h = mix_u128(h, (corpus_u128)(through_u128(neg_2p64) + s));
        h = mix_u128(h, (corpus_u128)(through_u128(u128_max) - s));
        h = mix_u128(h, (corpus_u128)(through_u128(i128_max) - s));
        h = mix_u128(h, (corpus_u128)(through_u128(i128_min) + s));
        if (pass == 0) {
            for (int k = 0; k < 6; k++) { h = mix_u8(h, 1); }
        }
    }

    h = mix_u128(h, (corpus_u128)(p2_100 + s));
    h = mix_u128(h, (corpus_u128)(through_u128(p2_100) + s));
    h = mix_u128(h, (corpus_u128)(through_u128(p2_100) + s));
    h = mix_u8(h, 1);
    return h;
}
