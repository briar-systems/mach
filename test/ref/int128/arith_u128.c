#include "corpus.h"

#define U128(hi, lo) (((corpus_u128)UINT64_C(hi) << 64) | (corpus_u128)UINT64_C(lo))

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const corpus_u128 s = (corpus_u128)seed;

    const corpus_u128 lo_ones = U128(0, 18446744073709551615);
    const corpus_u128 top     = (corpus_u128)1 << 127;
    const corpus_u128 big     = U128(0x0123456789ABCDEF, 0xFEDCBA9876543210);

    corpus_u128 a = lo_ones + s;
    for (corpus_u128 i = 0; i < 20; i = i + 1) {
        a = a + (lo_ones - i);
        h = mix_u128(h, a);
        a = a - (i * 3 + 1);
        h = mix_u128(h, a);
    }

    corpus_u128 b = big ^ s;
    h = mix_u128(h, b + b);
    h = mix_u128(h, b - a);
    h = mix_u128(h, a - b);
    h = mix_u128(h, (corpus_u128)0 - b);
    h = mix_u128(h, ~b);
    h = mix_u128(h, b & a);
    h = mix_u128(h, b | a);
    h = mix_u128(h, b ^ a);
    h = mix_u128(h, top + top);
    h = mix_u128(h, top - 1 + s);

    for (unsigned k = 0; k < 128; k += 9) {
        h = mix_u128(h, b << k);
        h = mix_u128(h, b >> k);
        h = mix_u128(h, (b << k) | (b >> (127 - k)));
    }
    h = mix_u128(h, b << 64);
    h = mix_u128(h, b >> 64);
    h = mix_u128(h, (b >> 64) << 64);
    h = mix_u64(h, (uint64_t)(b >> 64));
    h = mix_u64(h, (uint64_t)b);
    return h;
}
