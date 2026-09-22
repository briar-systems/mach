#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint64_t shl64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v << n); }
static inline uint64_t shr64(uint64_t v, uint64_t n) { return n >= 64u ? (uint64_t)0 : (uint64_t)(v >> n); }


uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = (uint64_t)seed;

    const uint64_t zero = UINT64_C(0);
    const uint64_t ones = UINT64_C(18446744073709551615);
    const uint64_t even = UINT64_C(12297829382473034410);
    const uint64_t odd  = UINT64_C(6148914691236517205);

    uint64_t a = (uint64_t)(ones ^ s);
    uint64_t b = (uint64_t)(even ^ s);
    uint64_t c = (uint64_t)(odd ^ s);
    uint64_t z = (uint64_t)(zero ^ s);

    h = mix_u64(h, shl64(a, 0));
    h = mix_u64(h, shl64(a, 1));
    h = mix_u64(h, shl64(a, 63));
    h = mix_u64(h, shr64(a, 0));
    h = mix_u64(h, shr64(a, 1));
    h = mix_u64(h, shr64(a, 63));
    h = mix_u64(h, shl64(b, 1));
    h = mix_u64(h, shr64(b, 1));
    h = mix_u64(h, shr64(b, 63));
    h = mix_u64(h, shl64(c, 1));
    h = mix_u64(h, shr64(c, 1));
    h = mix_u64(h, shl64(z, 5));
    h = mix_u64(h, shr64(z, 5));

    for (uint8_t i = 0; i < 64; i = (uint8_t)(i + 1)) {
        h = mix_u64(h, shl64(a, i));
        h = mix_u64(h, shr64(a, i));
        h = mix_u64(h, shl64(b, i));
        h = mix_u64(h, shr64(c, i));
    }

    h = mix_u64(h, shl64(a, 63));
    h = mix_u64(h, shr64(a, 63));
    h = mix_u64(h, shr64(b, 63));
    h = mix_u64(h, shl64(c, 63));
    h = mix_u64(h, shl64(a, 64));
    h = mix_u64(h, shr64(a, 64));
    h = mix_u64(h, shr64(b, 64));
    h = mix_u64(h, shl64(c, 64));
    h = mix_u64(h, shl64(a, 65));
    h = mix_u64(h, shr64(a, 65));
    h = mix_u64(h, shr64(b, 65));
    h = mix_u64(h, shl64(c, 65));
    h = mix_u64(h, shl64(a, 128));
    h = mix_u64(h, shr64(a, 128));
    h = mix_u64(h, shr64(b, 128));
    h = mix_u64(h, shl64(c, 128));
    h = mix_u64(h, shl64(a, 135));
    h = mix_u64(h, shr64(a, 135));
    h = mix_u64(h, shr64(b, 135));
    h = mix_u64(h, shl64(c, 135));
    h = mix_u64(h, shl64(a, 255));
    h = mix_u64(h, shr64(a, 255));
    h = mix_u64(h, shr64(b, 255));
    h = mix_u64(h, shl64(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 58u + j + (seed & 3u);
        h = mix_u64(h, shl64(a, n));
        h = mix_u64(h, shr64(a, n));
        h = mix_u64(h, shr64(b, n));
    }

    return h;
}
