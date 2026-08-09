#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t zero = UINT32_C(0);
    const uint32_t ones = UINT32_C(4294967295);
    const uint32_t even = UINT32_C(2863311530);
    const uint32_t odd  = UINT32_C(1431655765);

    uint32_t a = (uint32_t)(zero ^ s);
    uint32_t b = (uint32_t)(ones ^ s);
    uint32_t c = (uint32_t)(even ^ s);
    uint32_t d = (uint32_t)(odd ^ s);

    h = mix_u32(h, (uint32_t)(a & b));
    h = mix_u32(h, (uint32_t)(a | b));
    h = mix_u32(h, (uint32_t)(a ^ b));
    h = mix_u32(h, (uint32_t)(~a));
    h = mix_u32(h, (uint32_t)(~b));
    h = mix_u32(h, (uint32_t)(c & d));
    h = mix_u32(h, (uint32_t)(c | d));
    h = mix_u32(h, (uint32_t)(c ^ d));
    h = mix_u32(h, (uint32_t)(~c));
    h = mix_u32(h, (uint32_t)(~d));
    h = mix_u32(h, (uint32_t)(a & c));
    h = mix_u32(h, (uint32_t)(b | d));
    h = mix_u32(h, (uint32_t)(a & d));
    h = mix_u32(h, (uint32_t)(b | c));
    h = mix_u32(h, (uint32_t)((a & b) | (c ^ d)));
    h = mix_u32(h, (uint32_t)(~(a | b)));
    h = mix_u32(h, (uint32_t)(~(a & b)));
    h = mix_u32(h, (uint32_t)(~(c ^ d)));

    for (uint32_t i = 0; i < UINT32_C(8); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t p = (uint32_t)(a ^ i);
        const uint32_t q = (uint32_t)(b | i);
        const uint32_t r = (uint32_t)(c & (d ^ i));
        h = mix_u32(h, (uint32_t)(p & q));
        h = mix_u32(h, (uint32_t)(p | r));
        h = mix_u32(h, (uint32_t)(~(p ^ r)));
    }

    return h;
}
