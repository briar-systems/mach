#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a C shift by
 * 128 or more is UB, so the reference tests the count first and runs on the
 * unsigned identity. */
static inline corpus_u128 shl128(corpus_u128 v, uint64_t n) { return n >= 128u ? (corpus_u128)0 : (corpus_u128)(v << n); }
static inline corpus_u128 asr128(corpus_u128 v, uint64_t n) {
    const corpus_u128 fill = (corpus_u128)0 - (v >> 127);
    if (n >= 128u) { return fill; }
    if (n == 0u)   { return v; }
    return (v >> n) | (corpus_u128)(fill << (128u - n));
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_u128 s = (corpus_u128)seed;

    const corpus_u128 big = U128(0xFEDCBA9876543210, 0x0123456789ABCDEF);
    const corpus_u128 pos = U128(0x7EDCBA9876543210, 0x0123456789ABCDEF);
    corpus_u128 a = big ^ s;
    corpus_u128 b = pos ^ s;

    for (uint8_t k = 0; k < 128; k = (uint8_t)(k + 1)) {
        h = mix_i128(h, (corpus_i128)shl128(a, k));
        h = mix_i128(h, (corpus_i128)asr128(a, k));
        h = mix_i128(h, (corpus_i128)asr128(b, k));
    }

    h = mix_i128(h, (corpus_i128)shl128(a, 127));
    h = mix_i128(h, (corpus_i128)asr128(a, 127));
    h = mix_i128(h, (corpus_i128)asr128(b, 127));
    h = mix_i128(h, (corpus_i128)shl128(a, 128));
    h = mix_i128(h, (corpus_i128)asr128(a, 128));
    h = mix_i128(h, (corpus_i128)asr128(b, 128));
    h = mix_i128(h, (corpus_i128)shl128(a, 129));
    h = mix_i128(h, (corpus_i128)asr128(a, 129));
    h = mix_i128(h, (corpus_i128)asr128(b, 129));
    h = mix_i128(h, (corpus_i128)shl128(a, 200));
    h = mix_i128(h, (corpus_i128)asr128(a, 200));
    h = mix_i128(h, (corpus_i128)asr128(b, 200));
    h = mix_i128(h, (corpus_i128)shl128(a, 255));
    h = mix_i128(h, (corpus_i128)asr128(a, 255));
    h = mix_i128(h, (corpus_i128)asr128(b, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 122u + j + (seed & 3u);
        h = mix_i128(h, (corpus_i128)shl128(a, n));
        h = mix_i128(h, (corpus_i128)asr128(a, n));
        h = mix_i128(h, (corpus_i128)asr128(b, n));
    }

    return h;
}
