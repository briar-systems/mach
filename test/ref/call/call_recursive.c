#include "corpus.h"

typedef struct { uint64_t lo; uint64_t mid; uint64_t hi; } Acc;

static uint64_t pong(uint64_t depth, uint64_t v, uint64_t h);

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t descend(uint64_t depth, uint64_t v, uint64_t h) {
    uint64_t local[8];
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        local[i] = (uint64_t)(v * (i + UINT64_C(1)) + depth);
    }
    const uint64_t guard = (uint64_t)(v ^ depth);

    uint64_t a = h;
    if (depth > 0) {
        a = descend((uint64_t)(depth - UINT64_C(1)), (uint64_t)(v * UINT64_C(3) + UINT64_C(1)), a);
    }

    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        a = mix_u64(a, local[i]);
    }
    a = mix_u64(a, guard);
    return mix_u64(a, depth);
}

static uint64_t ping(uint64_t depth, uint64_t v, uint64_t h) {
    uint64_t quad[4];
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        quad[i] = (uint64_t)(v + i * UINT64_C(7) + depth);
    }

    uint64_t a = h;
    if (depth > 0) {
        a = pong((uint64_t)(depth - UINT64_C(1)), (uint64_t)(v ^ UINT64_C(305419896)), a);
    }

    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        a = mix_u64(a, quad[i]);
    }
    return mix_u64(a, (uint64_t)(depth * UINT64_C(2)));
}

static uint64_t pong(uint64_t depth, uint64_t v, uint64_t h) {
    uint64_t six[6];
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        six[i] = (uint64_t)(v * (i + UINT64_C(3)) - depth);
    }
    const uint64_t mark = (uint64_t)(~v);

    uint64_t a = h;
    if (depth > 0) {
        a = ping((uint64_t)(depth - UINT64_C(1)), (uint64_t)(v + UINT64_C(11)), a);
    }

    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        a = mix_u64(a, six[i]);
    }
    a = mix_u64(a, mark);
    return mix_u64(a, (uint64_t)(depth * UINT64_C(3) + UINT64_C(1)));
}

static uint64_t tree(uint64_t depth, uint64_t v, uint64_t h) {
    const uint64_t here = (uint64_t)(v * UINT64_C(2654435761) + depth);
    uint64_t a = mix_u64(h, here);
    if (depth > 0) {
        a = tree((uint64_t)(depth - UINT64_C(1)), (uint64_t)(here + UINT64_C(1)), a);
        a = mix_u64(a, here);
        a = tree((uint64_t)(depth - UINT64_C(1)), (uint64_t)(here ^ UINT64_C(2863311530)), a);
    }
    return mix_u64(a, here);
}

static uint64_t tail(uint64_t depth, uint64_t v, uint64_t h) {
    uint64_t buf[4];
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        buf[i] = (uint64_t)(v ^ (uint64_t)(i * UINT64_C(1099511628211) + depth));
    }
    uint64_t a = h;
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        a = mix_u64(a, buf[i]);
    }
    if (depth == 0) {
        return a;
    }
    return tail((uint64_t)(depth - UINT64_C(1)), (uint64_t)(v * UINT64_C(5) + UINT64_C(3)), a);
}

static Acc gather(uint64_t depth, uint64_t v) {
    Acc here = {0, 0, 0};
    here.lo = (uint64_t)(v + depth);
    here.mid = (uint64_t)(v * UINT64_C(3) + depth);
    here.hi = (uint64_t)((~v) ^ depth);
    if (depth == 0) {
        return here;
    }
    const Acc below = gather((uint64_t)(depth - UINT64_C(1)), (uint64_t)(v * UINT64_C(7) + UINT64_C(1)));
    Acc out = {0, 0, 0};
    out.lo = (uint64_t)(here.lo ^ below.hi);
    out.mid = (uint64_t)(here.mid + below.mid);
    out.hi = (uint64_t)(here.hi ^ below.lo);
    return out;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[6];
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    h = descend((uint64_t)(UINT64_C(40) + seed), v[0], h);
    h = ping((uint64_t)(UINT64_C(33) + seed), v[1], h);
    h = pong((uint64_t)(UINT64_C(33) + seed), v[2], h);
    h = tree(UINT64_C(8), (uint64_t)(v[3] + seed), h);
    h = tail((uint64_t)(UINT64_C(48) + seed), v[4], h);

    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        const Acc g = gather((uint64_t)(UINT64_C(24) + seed), (uint64_t)(v[k] + seed));
        h = mix_u64(h, g.lo);
        h = mix_u64(h, g.mid);
        h = mix_u64(h, g.hi);
    }

    return h;
}
