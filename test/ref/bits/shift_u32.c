#include "corpus.h"

/* mach masks a u32 shift count to the low 5 bits (count mod 32) rather than
 * treating an out-of-range count as undefined, so the reference masks before
 * shifting: a plain C `<<`/`>>` by a count >= the operand width is UB. */
static inline uint32_t shl32(uint32_t v, uint32_t n) { return (uint32_t)(v << (n & 31u)); }
static inline uint32_t shr32(uint32_t v, uint32_t n) { return (uint32_t)(v >> (n & 31u)); }

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
    h = mix_u32(h, shl32(c, 1));
    h = mix_u32(h, shr32(c, 1));
    h = mix_u32(h, shl32(z, 5));
    h = mix_u32(h, shr32(z, 5));

    h = mix_u32(h, shl32(a, 32));
    h = mix_u32(h, shr32(a, 32));
    h = mix_u32(h, shl32(a, 33));
    h = mix_u32(h, shr32(a, 33));
    h = mix_u32(h, shl32(a, 63));
    h = mix_u32(h, shr32(a, 63));
    h = mix_u32(h, shl32(b, 32));
    h = mix_u32(h, shr32(b, 63));

    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        h = mix_u32(h, shl32(a, i));
        h = mix_u32(h, shr32(a, i));
        h = mix_u32(h, shl32(b, (uint32_t)(i + s)));
        h = mix_u32(h, shr32(c, (uint32_t)(i + s)));
    }

    for (uint32_t j = UINT32_C(30); j < UINT32_C(40); j = (uint32_t)(j + UINT32_C(1))) {
        h = mix_u32(h, shl32(a, (uint32_t)(j + s)));
        h = mix_u32(h, shr32(a, (uint32_t)(j + s)));
    }

    return h;
}
