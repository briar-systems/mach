#include "corpus.h"

/* mach saturates a shift count at or above the operand width: << and a
 * logical >> answer 0, an arithmetic >> the sign fill (#3756). a plain C shift
 * by such a count is UB, and a C >> of a negative signed value is
 * implementation-defined, so every op runs on the unsigned identity. */
static inline uint32_t shl32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v << n); }
static inline uint32_t shr32(uint32_t v, uint64_t n) { return n >= 32u ? (uint32_t)0 : (uint32_t)(v >> n); }


uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t zero = UINT32_C(0);
    const uint32_t ones = UINT32_C(4294967295);
    const uint32_t even = UINT32_C(2863311530);
    const uint32_t odd  = UINT32_C(1431655765);

    uint32_t a = (uint32_t)(ones ^ s);
    uint32_t b = (uint32_t)(even ^ s);
    uint32_t c = (uint32_t)(odd ^ s);
    uint32_t z = (uint32_t)(zero ^ s);

    h = mix_u32(h, shl32(a, 0));
    h = mix_u32(h, shl32(a, 1));
    h = mix_u32(h, shl32(a, 31));
    h = mix_u32(h, shr32(a, 0));
    h = mix_u32(h, shr32(a, 1));
    h = mix_u32(h, shr32(a, 31));
    h = mix_u32(h, shl32(b, 1));
    h = mix_u32(h, shr32(b, 1));
    h = mix_u32(h, shr32(b, 31));
    h = mix_u32(h, shl32(c, 1));
    h = mix_u32(h, shr32(c, 1));
    h = mix_u32(h, shl32(z, 5));
    h = mix_u32(h, shr32(z, 5));

    for (uint8_t i = 0; i < 32; i = (uint8_t)(i + 1)) {
        h = mix_u32(h, shl32(a, i));
        h = mix_u32(h, shr32(a, i));
        h = mix_u32(h, shl32(b, i));
        h = mix_u32(h, shr32(c, i));
    }

    h = mix_u32(h, shl32(a, 31));
    h = mix_u32(h, shr32(a, 31));
    h = mix_u32(h, shr32(b, 31));
    h = mix_u32(h, shl32(c, 31));
    h = mix_u32(h, shl32(a, 32));
    h = mix_u32(h, shr32(a, 32));
    h = mix_u32(h, shr32(b, 32));
    h = mix_u32(h, shl32(c, 32));
    h = mix_u32(h, shl32(a, 33));
    h = mix_u32(h, shr32(a, 33));
    h = mix_u32(h, shr32(b, 33));
    h = mix_u32(h, shl32(c, 33));
    h = mix_u32(h, shl32(a, 64));
    h = mix_u32(h, shr32(a, 64));
    h = mix_u32(h, shr32(b, 64));
    h = mix_u32(h, shl32(c, 64));
    h = mix_u32(h, shl32(a, 71));
    h = mix_u32(h, shr32(a, 71));
    h = mix_u32(h, shr32(b, 71));
    h = mix_u32(h, shl32(c, 71));
    h = mix_u32(h, shl32(a, 255));
    h = mix_u32(h, shr32(a, 255));
    h = mix_u32(h, shr32(b, 255));
    h = mix_u32(h, shl32(c, 255));

    for (uint64_t j = 0; j < 12; j = j + 1) {
        const uint64_t n = 26u + j + (seed & 3u);
        h = mix_u32(h, shl32(a, n));
        h = mix_u32(h, shr32(a, n));
        h = mix_u32(h, shr32(b, n));
    }

    return h;
}
