#include "corpus.h"

/* every op here runs on the unsigned two's-complement identity so nothing is
 * implementation-defined or UB: a plain C `<<` on a negative signed value, or
 * `>>` on one, is not guaranteed to be an arithmetic shift, and a count >= the
 * operand width is UB either way. mach masks the count to the low 6 bits
 * (count mod 64) and its `>>` on a signed operand sign-extends. */
static inline uint64_t shl64(uint64_t v, uint64_t n) { return v << (n & 63u); }

static inline uint64_t asr64(uint64_t v, uint64_t n) {
    const uint64_t sh = n & 63u;
    if (sh == 0u) { return v; }
    const uint64_t lo   = v >> sh;
    const uint64_t fill = UINT64_C(0) - (v >> 63);
    const uint64_t hi   = fill << (64u - sh);
    return lo | hi;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t zero = UINT64_C(0);
    const uint64_t neg1 = UINT64_C(0xFFFFFFFFFFFFFFFF);
    const uint64_t even = UINT64_C(0xAAAAAAAAAAAAAAAA);
    const uint64_t odd  = UINT64_C(0x5555555555555555);

    uint64_t a = neg1 ^ seed;
    uint64_t b = even ^ seed;
    uint64_t c = odd ^ seed;
    uint64_t z = zero ^ seed;

    h = mix_i64(h, (int64_t)shl64(a, 0));
    h = mix_i64(h, (int64_t)shl64(a, 1));
    h = mix_i64(h, (int64_t)shl64(a, 63));
    h = mix_i64(h, (int64_t)asr64(a, 0));
    h = mix_i64(h, (int64_t)asr64(a, 1));
    h = mix_i64(h, (int64_t)asr64(a, 63));
    h = mix_i64(h, (int64_t)shl64(b, 1));
    h = mix_i64(h, (int64_t)asr64(b, 1));
    h = mix_i64(h, (int64_t)asr64(b, 63));
    h = mix_i64(h, (int64_t)shl64(c, 1));
    h = mix_i64(h, (int64_t)asr64(c, 1));
    h = mix_i64(h, (int64_t)shl64(z, 5));
    h = mix_i64(h, (int64_t)asr64(z, 5));

    h = mix_i64(h, (int64_t)shl64(a, 64));
    h = mix_i64(h, (int64_t)asr64(a, 64));
    h = mix_i64(h, (int64_t)shl64(a, 65));
    h = mix_i64(h, (int64_t)asr64(a, 65));
    h = mix_i64(h, (int64_t)shl64(a, 127));
    h = mix_i64(h, (int64_t)asr64(a, 127));
    h = mix_i64(h, (int64_t)shl64(b, 64));
    h = mix_i64(h, (int64_t)asr64(b, 127));

    for (uint64_t i = 0; i < UINT64_C(64); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_i64(h, (int64_t)shl64(a, i));
        h = mix_i64(h, (int64_t)asr64(a, i));
        h = mix_i64(h, (int64_t)shl64(b, (uint64_t)(i + seed)));
        h = mix_i64(h, (int64_t)asr64(c, (uint64_t)(i + seed)));
    }

    for (uint64_t j = UINT64_C(60); j < UINT64_C(72); j = (uint64_t)(j + UINT64_C(1))) {
        h = mix_i64(h, (int64_t)shl64(a, (uint64_t)(j + seed)));
        h = mix_i64(h, (int64_t)asr64(a, (uint64_t)(j + seed)));
    }

    return h;
}
