#include "corpus.h"

static uint64_t seq;
static uint64_t hits[4];
static uint64_t stamps[4];

static uint8_t probe(uint64_t id, uint8_t r) {
    seq = (uint64_t)(seq + UINT64_C(1));
    hits[id] = (uint64_t)(hits[id] + UINT64_C(1));
    stamps[id] = seq;
    return r;
}

static void reset_state(void) {
    seq = 0;
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        hits[i] = 0;
        stamps[i] = 0;
    }
}

static uint64_t fold_state(uint64_t h) {
    uint64_t a = mix_u64(h, seq);
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        a = mix_u64(a, hits[i]);
        a = mix_u64(a, stamps[i]);
    }
    return a;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    reset_state();
    const uint8_t r1 = (uint8_t)(probe(0, 0) && probe(1, (uint8_t)(1 ^ s)));
    h = mix_u8(h, r1);
    h = fold_state(h);

    reset_state();
    const uint8_t r2 = (uint8_t)(probe(0, 1) && probe(1, (uint8_t)(1 ^ s)));
    h = mix_u8(h, r2);
    h = fold_state(h);

    reset_state();
    const uint8_t r3 = (uint8_t)(probe(0, 1) || probe(1, (uint8_t)(0 ^ s)));
    h = mix_u8(h, r3);
    h = fold_state(h);

    reset_state();
    const uint8_t r4 = (uint8_t)(probe(0, 0) || probe(1, (uint8_t)(1 ^ s)));
    h = mix_u8(h, r4);
    h = fold_state(h);

    reset_state();
    const uint8_t r5 = (uint8_t)((probe(0, 0) || probe(1, 1)) && (probe(2, (uint8_t)(0 ^ s)) || probe(3, 1)));
    h = mix_u8(h, r5);
    h = fold_state(h);

    reset_state();
    const uint8_t r6 = (uint8_t)((probe(0, 1) || probe(1, 1)) && (probe(2, 0) || probe(3, (uint8_t)(1 ^ s))));
    h = mix_u8(h, r6);
    h = fold_state(h);

    reset_state();
    const uint8_t r7 = (uint8_t)((probe(0, 0) || probe(1, (uint8_t)(0 ^ s))) && (probe(2, 1) || probe(3, 1)));
    h = mix_u8(h, r7);
    h = fold_state(h);

    return h;
}
