#include "corpus.h"

static uint64_t read_run(const uint8_t *p, uint64_t n) {
    uint64_t acc = 0;
    for (uint64_t k = 0; k < n; k = (uint64_t)(k + UINT64_C(1))) {
        acc = (uint64_t)((acc << 8) | (uint64_t)p[k]);
    }
    return acc;
}

static uint64_t from(const uint8_t *s, uint64_t i, uint64_t n) {
    return read_run(&s[i], (uint64_t)(n - i));
}

static uint64_t from_view(const uint8_t *s, uint64_t i, uint64_t n) {
    const uint8_t *const p = &s[i];
    return read_run(p, (uint64_t)(n - i));
}

uint64_t checksum(uint64_t seed) {
    uint8_t four[4] = {0xE4, 0xBD, 0xA0, 0x21};
    const uint64_t a = from(&four[0], UINT64_C(0), UINT64_C(3));
    uint8_t five[4] = {0xE4, 0xBD, (uint8_t)(UINT64_C(0xA0) + seed), 0x21};
    const uint64_t b = from(&five[0], UINT64_C(0), UINT64_C(4));
    uint8_t six[8] = {1, 2, 3, (uint8_t)(UINT64_C(4) + seed), 5, 6, 7, 8};
    const uint64_t c = from_view(&six[0], UINT64_C(0), UINT64_C(8));
    uint64_t h = fold_init();
    h = mix_u64(h, a);
    h = mix_u64(h, b);
    h = mix_u64(h, c);
    return h;
}
