#include "corpus.h"
#include <stddef.h>

typedef struct {
    uint8_t a;
    uint32_t b;
    uint8_t c;
} Tiny;

typedef struct {
    uint16_t lead;
    Tiny t;
    uint8_t tail;
} Mid;

typedef struct {
    Mid m;
    uint64_t d;
    uint16_t arr[3];
    uint8_t e;
} Wide;

typedef struct {
    Wide w;
    Mid ms[2];
    uint32_t z;
} Nest;

static uint64_t fold_tiny(uint64_t h, Tiny t) {
    h = mix_u8(h, t.a);
    h = mix_u32(h, t.b);
    h = mix_u8(h, t.c);
    return h;
}

static uint64_t fold_mid(uint64_t h, Mid m) {
    h = mix_u16(h, m.lead);
    h = fold_tiny(h, m.t);
    h = mix_u8(h, m.tail);
    return h;
}

static uint64_t fold_wide(uint64_t h, Wide w) {
    h = fold_mid(h, w.m);
    h = mix_u64(h, w.d);
    h = mix_u16(h, w.arr[0]);
    h = mix_u16(h, w.arr[1]);
    h = mix_u16(h, w.arr[2]);
    h = mix_u8(h, w.e);
    return h;
}

static uint64_t fold_nest(uint64_t h, Nest n) {
    h = fold_wide(h, n.w);
    h = fold_mid(h, n.ms[0]);
    h = fold_mid(h, n.ms[1]);
    h = mix_u32(h, n.z);
    return h;
}

static void fill_mid(Mid *m, uint32_t base) {
    m->lead = (uint16_t)base;
    m->t.a = (uint8_t)(base + UINT32_C(1));
    m->t.b = (uint32_t)(base + UINT32_C(2));
    m->t.c = (uint8_t)(base + UINT32_C(3));
    m->tail = (uint8_t)(base + UINT32_C(4));
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    h = mix_u64(h, (uint64_t)sizeof(Tiny));
    h = mix_u64(h, (uint64_t)sizeof(Mid));
    h = mix_u64(h, (uint64_t)sizeof(Wide));
    h = mix_u64(h, (uint64_t)sizeof(Nest));
    h = mix_u64(h, (uint64_t)_Alignof(Tiny));
    h = mix_u64(h, (uint64_t)_Alignof(Mid));
    h = mix_u64(h, (uint64_t)_Alignof(Wide));
    h = mix_u64(h, (uint64_t)_Alignof(Nest));
    h = mix_u64(h, (uint64_t)offsetof(Tiny, b));
    h = mix_u64(h, (uint64_t)offsetof(Tiny, c));
    h = mix_u64(h, (uint64_t)offsetof(Mid, t));
    h = mix_u64(h, (uint64_t)offsetof(Mid, tail));
    h = mix_u64(h, (uint64_t)offsetof(Wide, d));
    h = mix_u64(h, (uint64_t)offsetof(Wide, arr));
    h = mix_u64(h, (uint64_t)offsetof(Wide, e));
    h = mix_u64(h, (uint64_t)offsetof(Nest, ms));
    h = mix_u64(h, (uint64_t)offsetof(Nest, z));

    Nest n;
    memset(&n, 0, sizeof n);
    h = fold_nest(h, n);

    fill_mid(&n.w.m, (uint32_t)(UINT32_C(10) + s));
    n.w.d = (uint64_t)(UINT64_C(18364758544493064720) + seed);
    n.w.arr[0] = (uint16_t)(UINT32_C(20) + s);
    n.w.arr[1] = (uint16_t)(UINT32_C(21) + s);
    n.w.arr[2] = (uint16_t)(UINT32_C(22) + s);
    n.w.e = (uint8_t)(UINT32_C(23) + s);
    fill_mid(&n.ms[0], (uint32_t)(UINT32_C(30) + s));
    fill_mid(&n.ms[1], (uint32_t)(UINT32_C(40) + s));
    n.z = (uint32_t)(UINT32_C(50) + s);
    h = fold_nest(h, n);

    uint32_t *pb = &n.w.m.t.b;
    *pb = (uint32_t)(*pb ^ UINT32_C(4042322160));
    h = fold_nest(h, n);

    uint8_t *pc = &n.ms[1].t.c;
    *pc = (uint8_t)(*pc + UINT8_C(7));
    h = fold_nest(h, n);

    uint64_t *pd = &n.w.d;
    *pd = (uint64_t)(*pd >> 3);
    h = fold_nest(h, n);

    uint32_t *pz = &n.z;
    *pz = (uint32_t)(*pz * UINT32_C(3));
    h = fold_nest(h, n);

    Mid m2 = n.ms[0];
    m2.t.b = (uint32_t)(m2.t.b + UINT32_C(1));
    h = fold_mid(h, m2);
    h = fold_mid(h, n.ms[0]);
    n.ms[0] = m2;
    h = fold_nest(h, n);
    return h;
}
