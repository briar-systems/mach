#include "corpus.h"

typedef struct __attribute__((packed)) {
    uint8_t tag;
    uint64_t wide;
    uint32_t mid;
    uint16_t half;
} Skewed;

static uint64_t mix(uint64_t h, uint64_t v) {
    return (uint64_t)((h ^ v) * UINT64_C(1099511628211));
}

static uint64_t deep_slots(uint64_t seed) {
    uint64_t pad[96];
    uint64_t a = (uint64_t)(seed + 1u);
    uint32_t b = (uint32_t)(seed + 2u);
    uint16_t c = (uint16_t)(seed + 3u);
    uint8_t d = (uint8_t)(seed + 4u);
    double e = 1.5;
    float g = 2.25f;
    Skewed s;
    uint8_t tail[3];
    pad[0] = a;
    pad[95] = seed;
    s.tag = d;
    s.wide = (uint64_t)(a * 3u);
    s.mid = (uint32_t)(b + 7u);
    s.half = (uint16_t)(c + 9u);
    tail[0] = (uint8_t)(d + 1u);
    tail[2] = (uint8_t)(d + 2u);

    for (uint64_t i = 1; i < UINT64_C(95); i = (uint64_t)(i + 1u)) {
        pad[i] = (uint64_t)(pad[i - 1u] + i);
    }
    a = (uint64_t)(a + pad[94]);
    b = (uint32_t)(b + s.mid);
    c = (uint16_t)(c + s.half);
    e = e + (double)g;

    uint64_t h = UINT64_C(14695981039346656037);
    h = mix(h, a);
    h = mix(h, (uint64_t)b);
    h = mix(h, (uint64_t)c);
    h = mix(h, (uint64_t)d);
    h = mix(h, (uint64_t)e);
    h = mix(h, (uint64_t)s.tag);
    h = mix(h, s.wide);
    h = mix(h, (uint64_t)s.mid);
    h = mix(h, (uint64_t)s.half);
    h = mix(h, (uint64_t)tail[0] + (uint64_t)tail[2]);
    h = mix(h, pad[95]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + 1u)) {
        h = mix_u64(h, deep_slots((uint64_t)((seed + k) * UINT64_C(1000003))));
    }
    return h;
}
