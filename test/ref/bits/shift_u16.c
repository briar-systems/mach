#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint16_t shl16(uint16_t v, uint64_t n) { return n >= 16u ? (uint16_t)0 : (uint16_t)(v << n); }
static inline uint16_t shr16(uint16_t v, uint64_t n) { return n >= 16u ? (uint16_t)0 : (uint16_t)(v >> n); }


uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    const uint16_t zero = UINT16_C(0);
    const uint16_t ones = UINT16_C(65535);
    const uint16_t even = UINT16_C(43690);
    const uint16_t odd  = UINT16_C(21845);

    uint16_t a = (uint16_t)(ones ^ s);
    uint16_t b = (uint16_t)(even ^ s);
    uint16_t c = (uint16_t)(odd ^ s);
    uint16_t z = (uint16_t)(zero ^ s);

    h = mix_u16(h, shl16(a, 0));
    h = mix_u16(h, shl16(a, 1));
    h = mix_u16(h, shl16(a, 15));
    h = mix_u16(h, shr16(a, 0));
    h = mix_u16(h, shr16(a, 1));
    h = mix_u16(h, shr16(a, 15));
    h = mix_u16(h, shl16(b, 1));
    h = mix_u16(h, shr16(b, 1));
    h = mix_u16(h, shr16(b, 15));
    h = mix_u16(h, shl16(c, 1));
    h = mix_u16(h, shr16(c, 1));
    h = mix_u16(h, shl16(z, 5));
    h = mix_u16(h, shr16(z, 5));

    for (uint8_t i = 0; i < 16; i = (uint8_t)(i + 1)) {
        h = mix_u16(h, shl16(a, i));
        h = mix_u16(h, shr16(a, i));
        h = mix_u16(h, shl16(b, i));
        h = mix_u16(h, shr16(c, i));
    }

    h = mix_u16(h, shl16(a, 15));
    h = mix_u16(h, shr16(a, 15));
    h = mix_u16(h, shr16(b, 15));
    h = mix_u16(h, shl16(c, 15));
    h = mix_u16(h, shl16(a, 16));
    h = mix_u16(h, shr16(a, 16));
    h = mix_u16(h, shr16(b, 16));
    h = mix_u16(h, shl16(c, 16));
    h = mix_u16(h, shl16(a, 17));
    h = mix_u16(h, shr16(a, 17));
    h = mix_u16(h, shr16(b, 17));
    h = mix_u16(h, shl16(c, 17));
    h = mix_u16(h, shl16(a, 32));
    h = mix_u16(h, shr16(a, 32));
    h = mix_u16(h, shr16(b, 32));
    h = mix_u16(h, shl16(c, 32));
    h = mix_u16(h, shl16(a, 39));
    h = mix_u16(h, shr16(a, 39));
    h = mix_u16(h, shr16(b, 39));
    h = mix_u16(h, shl16(c, 39));
    h = mix_u16(h, shl16(a, 255));
    h = mix_u16(h, shr16(a, 255));
    h = mix_u16(h, shr16(b, 255));
    h = mix_u16(h, shl16(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 10u + j + (seed & 3u);
        h = mix_u16(h, shl16(a, n));
        h = mix_u16(h, shr16(a, n));
        h = mix_u16(h, shr16(b, n));
    }

    return h;
}
