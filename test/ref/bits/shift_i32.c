#include "corpus.h"

/* every op here runs on the unsigned two's-complement identity so nothing is
 * implementation-defined or UB: a plain C `<<` on a negative signed value, or
 * `>>` on one, is not guaranteed to be an arithmetic shift, and a count >= the
 * operand width is UB either way. mach masks the count to the low 5 bits
 * (count mod 32) and its `>>` on a signed operand sign-extends. */
static inline uint32_t shl32(uint32_t v, uint32_t n) { return (uint32_t)(v << (n & 31u)); }

static inline uint32_t asr32(uint32_t v, uint32_t n) {
    const uint32_t sh = n & 31u;
    if (sh == 0u) { return v; }
    const uint32_t lo   = (uint32_t)(v >> sh);
    const uint32_t fill = (uint32_t)(UINT32_C(0) - (v >> 31));
    const uint32_t hi   = (uint32_t)(fill << (32u - sh));
    return (uint32_t)(lo | hi);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s  = (uint32_t)seed;
    const uint32_t su = (uint32_t)seed;

    const uint32_t zero = UINT32_C(0);
    const uint32_t neg1 = UINT32_C(0xFFFFFFFF);
    const uint32_t even = UINT32_C(0xAAAAAAAA);
    const uint32_t odd  = UINT32_C(0x55555555);

    uint32_t a = (uint32_t)(neg1 ^ s);
    uint32_t b = (uint32_t)(even ^ s);
    uint32_t c = (uint32_t)(odd ^ s);
    uint32_t z = (uint32_t)(zero ^ s);

    h = mix_i32(h, (int32_t)shl32(a, 0));
    h = mix_i32(h, (int32_t)shl32(a, 1));
    h = mix_i32(h, (int32_t)shl32(a, 31));
    h = mix_i32(h, (int32_t)asr32(a, 0));
    h = mix_i32(h, (int32_t)asr32(a, 1));
    h = mix_i32(h, (int32_t)asr32(a, 31));
    h = mix_i32(h, (int32_t)shl32(b, 1));
    h = mix_i32(h, (int32_t)asr32(b, 1));
    h = mix_i32(h, (int32_t)asr32(b, 31));
    h = mix_i32(h, (int32_t)shl32(c, 1));
    h = mix_i32(h, (int32_t)asr32(c, 1));
    h = mix_i32(h, (int32_t)shl32(z, 5));
    h = mix_i32(h, (int32_t)asr32(z, 5));

    h = mix_i32(h, (int32_t)shl32(a, 32));
    h = mix_i32(h, (int32_t)asr32(a, 32));
    h = mix_i32(h, (int32_t)shl32(a, 33));
    h = mix_i32(h, (int32_t)asr32(a, 33));
    h = mix_i32(h, (int32_t)shl32(a, 63));
    h = mix_i32(h, (int32_t)asr32(a, 63));
    h = mix_i32(h, (int32_t)shl32(b, 32));
    h = mix_i32(h, (int32_t)asr32(b, 63));

    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        h = mix_i32(h, (int32_t)shl32(a, i));
        h = mix_i32(h, (int32_t)asr32(a, i));
        h = mix_i32(h, (int32_t)shl32(b, (uint32_t)(i + su)));
        h = mix_i32(h, (int32_t)asr32(c, (uint32_t)(i + su)));
    }

    for (uint32_t j = UINT32_C(30); j < UINT32_C(40); j = (uint32_t)(j + UINT32_C(1))) {
        h = mix_i32(h, (int32_t)shl32(a, (uint32_t)(j + su)));
        h = mix_i32(h, (int32_t)asr32(a, (uint32_t)(j + su)));
    }

    return h;
}
