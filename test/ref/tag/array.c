#include "corpus.h"

typedef struct { uint8_t d; union { int64_t value; } p; } Reply;
typedef struct { uint16_t d; union { uint8_t byte; uint8_t pair[2]; } p; } Small;
typedef struct { uint32_t count; Reply items[5]; } Bag;
typedef struct { Small items[3]; } Smalls;

static Reply reply_empty(void) { Reply r; memset(&r, 0, sizeof r); r.d = 0; return r; }
static Reply reply_value(int64_t v) { Reply r; memset(&r, 0, sizeof r); r.p.value = v; r.d = 1; return r; }
static Small small_none(void) { Small v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static Small small_byte(uint8_t b) { Small v; memset(&v, 0, sizeof v); v.p.byte = b; v.d = 1; return v; }
static Small small_pair(uint8_t a, uint8_t b) { Small v; memset(&v, 0, sizeof v); v.p.pair[0] = a; v.p.pair[1] = b; v.d = 2; return v; }

static int64_t reply_of(Reply r) {
    if (r.d == 1) { return r.p.value; }
    return -1;
}

static uint32_t small_of(Small v) {
    uint32_t out = 65535;
    if (v.d == 1) { out = (uint32_t)v.p.byte; }
    else if (v.d == 2) { out = (uint32_t)(((uint32_t)v.p.pair[0] << 8) | (uint32_t)v.p.pair[1]); }
    return out;
}

static uint64_t fold_bag(uint64_t h, Bag b) {
    h = mix_u32(h, b.count);
    uint64_t i = 0;
    while (i < 5) {
        h = mix_i64(h, reply_of(b.items[i]));
        i = i + 1;
    }
    return h;
}

static uint32_t selected(Bag b) {
    uint32_t n = 0;
    uint64_t i = 0;
    while (i < 5) {
        if (b.items[i].d == 1) { n = (uint32_t)(n + UINT32_C(1)); }
        i = i + 1;
    }
    return n;
}

static uint64_t fold_smalls(uint64_t h, Smalls a) {
    uint64_t i = 0;
    while (i < 3) {
        h = mix_u32(h, small_of(a.items[i]));
        i = i + 1;
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed;

    Bag b;
    memset(&b, 0, sizeof b);
    h = fold_bag(h, b);
    h = mix_u32(h, selected(b));

    uint64_t i = seed;
    while (i < 5) {
        b.items[i] = reply_value(s + (int64_t)i * 10);
        i = i + 2;
    }
    b.count = selected(b);
    h = fold_bag(h, b);

    if (b.items[2].d == 1) { b.items[2].p.value = b.items[2].p.value + 7; }
    b.items[4] = reply_empty();
    b.count = selected(b);
    h = fold_bag(h, b);

    Reply copy[5];
    memcpy(copy, b.items, sizeof copy);
    copy[0] = reply_value(-5);
    if (copy[2].d == 1) { copy[2].p.value = 0; }
    Bag c;
    memset(&c, 0, sizeof c);
    c.count = selected(b);
    memcpy(c.items, copy, sizeof copy);
    h = fold_bag(h, b);
    h = fold_bag(h, c);

    const Bag d = c;
    h = fold_bag(h, d);

    Smalls sm;
    memset(&sm, 0, sizeof sm);
    h = fold_smalls(h, sm);
    sm.items[0] = small_byte((uint8_t)(UINT64_C(3) + seed));
    sm.items[1] = small_pair((uint8_t)(UINT64_C(4) + seed), 5);
    sm.items[2] = small_byte(6);
    h = fold_smalls(h, sm);
    if (sm.items[1].d == 2) { sm.items[1].p.pair[1] = (uint8_t)(sm.items[1].p.pair[1] + 1); }
    sm.items[2] = small_none();
    h = fold_smalls(h, sm);
    return h;
}
