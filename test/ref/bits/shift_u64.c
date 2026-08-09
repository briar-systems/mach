#include "corpus.h"

/* mach masks a u64 shift count to the low 6 bits (count mod 64) rather than
 * treating an out-of-range count as undefined, so the reference masks before
 * shifting: a plain C `<<`/`>>` by a count >= the operand width is UB. */
static inline uint64_t shl64(uint64_t v, uint64_t n) { return v << (n & 63u); }
static inline uint64_t shr64(uint64_t v, uint64_t n) { return v >> (n & 63u); }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t zero = UINT64_C(0);
    const uint64_t ones = UINT64_C(0xFFFFFFFFFFFFFFFF);
    const uint64_t even = UINT64_C(0xAAAAAAAAAAAAAAAA);
    const uint64_t odd  = UINT64_C(0x5555555555555555);

    uint64_t a = ones ^ seed;
    uint64_t b = even ^ seed;
    uint64_t c = odd ^ seed;
    uint64_t z = zero ^ seed;

    h = mix_u64(h, shl64(a, 0));
    h = mix_u64(h, shl64(a, 1));
    h = mix_u64(h, shl64(a, 63));
    h = mix_u64(h, shr64(a, 0));
    h = mix_u64(h, shr64(a, 1));
    h = mix_u64(h, shr64(a, 63));
    h = mix_u64(h, shl64(b, 1));
    h = mix_u64(h, shr64(b, 1));
    h = mix_u64(h, shl64(c, 1));
    h = mix_u64(h, shr64(c, 1));
    h = mix_u64(h, shl64(z, 5));
    h = mix_u64(h, shr64(z, 5));

    h = mix_u64(h, shl64(a, 64));
    h = mix_u64(h, shr64(a, 64));
    h = mix_u64(h, shl64(a, 65));
    h = mix_u64(h, shr64(a, 65));
    h = mix_u64(h, shl64(a, 127));
    h = mix_u64(h, shr64(a, 127));
    h = mix_u64(h, shl64(b, 64));
    h = mix_u64(h, shr64(b, 127));

    for (uint64_t i = 0; i < UINT64_C(64); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u64(h, shl64(a, i));
        h = mix_u64(h, shr64(a, i));
        h = mix_u64(h, shl64(b, (uint64_t)(i + seed)));
        h = mix_u64(h, shr64(c, (uint64_t)(i + seed)));
    }

    for (uint64_t j = UINT64_C(60); j < UINT64_C(72); j = (uint64_t)(j + UINT64_C(1))) {
        h = mix_u64(h, shl64(a, (uint64_t)(j + seed)));
        h = mix_u64(h, shr64(a, (uint64_t)(j + seed)));
    }

    return h;
}
