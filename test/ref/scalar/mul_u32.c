#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(2654435761) + s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a * (uint32_t)(UINT32_C(2) * i + UINT32_C(3)));
        h = mix_u32(h, a);
    }

    uint32_t b = (uint32_t)(UINT32_C(4294967231) + s);
    h = mix_u32(h, (uint32_t)(b * b));
    h = mix_u32(h, (uint32_t)(b * UINT32_C(4294967295)));
    h = mix_u32(h, (uint32_t)(a * b));
    h = mix_u32(h, (uint32_t)(b * a));

    uint32_t c = (uint32_t)(UINT32_C(65537) + s);
    h = mix_u32(h, (uint32_t)(c * c));
    h = mix_u32(h, (uint32_t)(c * UINT32_C(65535)));
    return h;
}
