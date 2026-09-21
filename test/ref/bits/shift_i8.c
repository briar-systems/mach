#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint8_t shl8(uint8_t v, uint64_t n) { return n >= 8u ? (uint8_t)0 : (uint8_t)(v << n); }
static inline uint8_t asr8(uint8_t v, uint64_t n) {
    const uint8_t fill = (uint8_t)(UINT8_C(0) - (v >> 7));
    if (n >= 8u) { return fill; }
    if (n == 0u)   { return v; }
    return (uint8_t)((v >> n) | (uint8_t)(fill << (8u - n)));
}


uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    const uint8_t zero = UINT8_C(0);
    const uint8_t neg1 = UINT8_C(255);
    const uint8_t even = UINT8_C(170);
    const uint8_t odd  = UINT8_C(85);

    uint8_t a = (uint8_t)(neg1 ^ s);
    uint8_t b = (uint8_t)(even ^ s);
    uint8_t c = (uint8_t)(odd ^ s);
    uint8_t z = (uint8_t)(zero ^ s);

    h = mix_i8(h, (int8_t)shl8(a, 0));
    h = mix_i8(h, (int8_t)shl8(a, 1));
    h = mix_i8(h, (int8_t)shl8(a, 7));
    h = mix_i8(h, (int8_t)asr8(a, 0));
    h = mix_i8(h, (int8_t)asr8(a, 1));
    h = mix_i8(h, (int8_t)asr8(a, 7));
    h = mix_i8(h, (int8_t)shl8(b, 1));
    h = mix_i8(h, (int8_t)asr8(b, 1));
    h = mix_i8(h, (int8_t)asr8(b, 7));
    h = mix_i8(h, (int8_t)shl8(c, 1));
    h = mix_i8(h, (int8_t)asr8(c, 1));
    h = mix_i8(h, (int8_t)shl8(z, 5));
    h = mix_i8(h, (int8_t)asr8(z, 5));

    for (uint8_t i = 0; i < 8; i = (uint8_t)(i + 1)) {
        h = mix_i8(h, (int8_t)shl8(a, i));
        h = mix_i8(h, (int8_t)asr8(a, i));
        h = mix_i8(h, (int8_t)shl8(b, i));
        h = mix_i8(h, (int8_t)asr8(c, i));
    }

    h = mix_i8(h, (int8_t)shl8(a, 7));
    h = mix_i8(h, (int8_t)asr8(a, 7));
    h = mix_i8(h, (int8_t)asr8(b, 7));
    h = mix_i8(h, (int8_t)shl8(c, 7));
    h = mix_i8(h, (int8_t)shl8(a, 8));
    h = mix_i8(h, (int8_t)asr8(a, 8));
    h = mix_i8(h, (int8_t)asr8(b, 8));
    h = mix_i8(h, (int8_t)shl8(c, 8));
    h = mix_i8(h, (int8_t)shl8(a, 9));
    h = mix_i8(h, (int8_t)asr8(a, 9));
    h = mix_i8(h, (int8_t)asr8(b, 9));
    h = mix_i8(h, (int8_t)shl8(c, 9));
    h = mix_i8(h, (int8_t)shl8(a, 16));
    h = mix_i8(h, (int8_t)asr8(a, 16));
    h = mix_i8(h, (int8_t)asr8(b, 16));
    h = mix_i8(h, (int8_t)shl8(c, 16));
    h = mix_i8(h, (int8_t)shl8(a, 23));
    h = mix_i8(h, (int8_t)asr8(a, 23));
    h = mix_i8(h, (int8_t)asr8(b, 23));
    h = mix_i8(h, (int8_t)shl8(c, 23));
    h = mix_i8(h, (int8_t)shl8(a, 255));
    h = mix_i8(h, (int8_t)asr8(a, 255));
    h = mix_i8(h, (int8_t)asr8(b, 255));
    h = mix_i8(h, (int8_t)shl8(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 2u + j + (seed & 3u);
        h = mix_i8(h, (int8_t)shl8(a, n));
        h = mix_i8(h, (int8_t)asr8(a, n));
        h = mix_i8(h, (int8_t)asr8(b, n));
    }

    return h;
}
