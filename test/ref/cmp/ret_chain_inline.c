#include "corpus.h"

static uint32_t pick(uint32_t v) {
    if (v == 1) { return 7; }
    else if (v == 2) { return 9; }
    return 65535;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;
    h = mix_u32(h, pick(s));
    h = mix_u32(h, pick((uint32_t)(s + UINT32_C(1))));
    h = mix_u32(h, pick((uint32_t)(s + UINT32_C(2))));
    uint32_t i = 0;
    while (i < 4) {
        h = mix_u32(h, pick((uint32_t)(i + s)));
        i = i + 1;
    }
    return h;
}
