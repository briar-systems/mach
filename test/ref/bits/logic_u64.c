#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t zero = UINT64_C(0);
    const uint64_t ones = UINT64_C(0xFFFFFFFFFFFFFFFF);
    const uint64_t even = UINT64_C(0xAAAAAAAAAAAAAAAA);
    const uint64_t odd  = UINT64_C(0x5555555555555555);

    uint64_t a = zero ^ seed;
    uint64_t b = ones ^ seed;
    uint64_t c = even ^ seed;
    uint64_t d = odd ^ seed;

    h = mix_u64(h, a & b);
    h = mix_u64(h, a | b);
    h = mix_u64(h, a ^ b);
    h = mix_u64(h, ~a);
    h = mix_u64(h, ~b);
    h = mix_u64(h, c & d);
    h = mix_u64(h, c | d);
    h = mix_u64(h, c ^ d);
    h = mix_u64(h, ~c);
    h = mix_u64(h, ~d);
    h = mix_u64(h, a & c);
    h = mix_u64(h, b | d);
    h = mix_u64(h, a & d);
    h = mix_u64(h, b | c);
    h = mix_u64(h, (a & b) | (c ^ d));
    h = mix_u64(h, ~(a | b));
    h = mix_u64(h, ~(a & b));
    h = mix_u64(h, ~(c ^ d));

    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t p = a ^ i;
        const uint64_t q = b | i;
        const uint64_t r = c & (d ^ i);
        h = mix_u64(h, p & q);
        h = mix_u64(h, p | r);
        h = mix_u64(h, ~(p ^ r));
    }

    return h;
}
