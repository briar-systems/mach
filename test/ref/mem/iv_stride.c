#include "corpus.h"

typedef struct {
    uint32_t a;
    uint64_t b;
    uint16_t c;
} Rec;

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    Rec rs[16];
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        rs[i].a = (uint32_t)(i * UINT64_C(2654435761) + s);
        rs[i].b = (uint64_t)((uint64_t)(i * UINT64_C(1099511628211)) ^ s);
        rs[i].c = (uint16_t)(i * UINT64_C(40503) + s);
    }

    uint32_t ws[32];
    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        ws[i] = (uint32_t)(i * UINT64_C(69069) + s);
    }

    for (uint64_t i = 0; i < UINT64_C(5); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u64(h, rs[(uint64_t)(i * UINT64_C(3) + UINT64_C(1))].b);
        h = mix_u32(h, rs[(uint64_t)(i * UINT64_C(3) + UINT64_C(1))].a);
    }

    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u16(h, rs[(uint64_t)(UINT64_C(15) - i)].c);
    }

    for (uint64_t i = 0; i < UINT64_C(13); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, ws[(uint64_t)(i + UINT64_C(5) + (s & UINT64_C(1)))]);
        h = mix_u32(h, ws[(uint64_t)(i * UINT64_C(2) + UINT64_C(1))]);
    }

    for (uint64_t r = 0; r < UINT64_C(4); r = (uint64_t)(r + UINT64_C(1))) {
        for (uint64_t c = 0; c < UINT64_C(4); c = (uint64_t)(c + UINT64_C(1))) {
            h = mix_u64(h, (uint64_t)(rs[(uint64_t)(r * UINT64_C(4) + c)].b + c));
        }
    }

    const uint64_t lead = (uint64_t)(UINT64_C(3) + (s & UINT64_C(1)));
    uint64_t w = (uint64_t)(UINT64_C(0) - lead);
    for (uint64_t n = 0; n < UINT64_C(10); n = (uint64_t)(n + UINT64_C(1))) {
        h = mix_u64(h, rs[(uint64_t)(w + lead)].b);
        w = (uint64_t)(w + UINT64_C(1));
    }

    uint64_t last = 0;
    for (uint64_t i = 0; i < UINT64_C(7); i = (uint64_t)(i + UINT64_C(1))) {
        last = (uint64_t)(i * UINT64_C(40503) + s);
        h = mix_u64(h, last);
    }
    h = mix_u64(h, last);

    uint64_t m = 0;
    for (uint64_t i = 0; i < UINT64_C(100); i = (uint64_t)(i + UINT64_C(1))) {
        m = (uint64_t)(i * UINT64_C(12345));
        if (m > (uint64_t)(UINT64_C(50000) + s)) {
            break;
        }
    }
    h = mix_u64(h, m);

    for (uint32_t j = 0; j < UINT32_C(20); j = (uint32_t)(j + UINT32_C(1))) {
        h = mix_u32(h, (uint32_t)((uint32_t)(j * UINT32_C(2654435761)) + (uint32_t)s));
    }
    return h;
}
