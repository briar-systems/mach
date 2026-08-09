#include "corpus.h"

#include <stddef.h>

typedef struct { uint64_t a; uint64_t b; } N16;
typedef struct { uint32_t a; uint32_t b; uint32_t c; uint32_t d; uint32_t e; } B20;
typedef struct { uint64_t a; uint64_t b; uint64_t c; } B24;
typedef struct { double x; double y; double z; } B24f;
typedef struct { uint64_t a; uint64_t b; uint64_t c; uint64_t d; } B32;
typedef struct { uint64_t a; double x; uint32_t b; uint32_t c; double y; uint64_t d; } B40m;
typedef struct { uint64_t a; uint64_t b; uint64_t c; uint64_t d; uint64_t e; uint64_t f; } B48;
typedef struct { uint32_t lead; B24 big; N16 tail; } Nest;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold20(uint64_t h, B20 r) {
    h = mix_u32(h, r.a);
    h = mix_u32(h, r.b);
    h = mix_u32(h, r.c);
    h = mix_u32(h, r.d);
    return mix_u32(h, r.e);
}

static uint64_t fold24(uint64_t h, B24 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    return mix_u64(h, r.c);
}

static uint64_t fold24f(uint64_t h, B24f r) {
    h = mix_f64(h, r.x);
    h = mix_f64(h, r.y);
    return mix_f64(h, r.z);
}

static uint64_t fold32(uint64_t h, B32 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    return mix_u64(h, r.d);
}

static uint64_t fold40m(uint64_t h, B40m r) {
    h = mix_u64(h, r.a);
    h = mix_f64(h, r.x);
    h = mix_u32(h, r.b);
    h = mix_u32(h, r.c);
    h = mix_f64(h, r.y);
    return mix_u64(h, r.d);
}

static uint64_t fold48(uint64_t h, B48 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    h = mix_u64(h, r.d);
    h = mix_u64(h, r.e);
    return mix_u64(h, r.f);
}

static uint64_t fold_nest(uint64_t h, Nest r) {
    h = mix_u32(h, r.lead);
    h = fold24(h, r.big);
    h = mix_u64(h, r.tail.a);
    return mix_u64(h, r.tail.b);
}

static uint64_t layout(uint64_t h) {
    h = mix_u64(h, (uint64_t)sizeof(B20));
    h = mix_u64(h, (uint64_t)sizeof(B24));
    h = mix_u64(h, (uint64_t)sizeof(B24f));
    h = mix_u64(h, (uint64_t)sizeof(B32));
    h = mix_u64(h, (uint64_t)sizeof(B40m));
    h = mix_u64(h, (uint64_t)sizeof(B48));
    h = mix_u64(h, (uint64_t)sizeof(Nest));
    h = mix_u64(h, (uint64_t)offsetof(Nest, big));
    h = mix_u64(h, (uint64_t)offsetof(Nest, tail));
    return h;
}

static B20 mk20(uint64_t v) {
    B20 r = {0, 0, 0, 0, 0};
    r.a = (uint32_t)v;
    r.b = (uint32_t)(v >> 8);
    r.c = (uint32_t)(v >> 16);
    r.d = (uint32_t)(v >> 24);
    r.e = (uint32_t)(v >> 32);
    return r;
}

static B24 mk24(uint64_t v) {
    B24 r = {0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(~v);
    r.c = (uint64_t)(v * UINT64_C(3) + UINT64_C(1));
    return r;
}

static B24f mk24f(uint64_t v) {
    B24f r = {0.0, 0.0, 0.0};
    r.x = (double)(v & UINT64_C(65535)) / 8.0;
    r.y = (double)(v & UINT64_C(4095)) - 2048.0;
    r.z = (double)(v & UINT64_C(262143)) / 64.0;
    return r;
}

static B32 mk32(uint64_t v) {
    B32 r = {0, 0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v + UINT64_C(1));
    r.c = (uint64_t)(v * UINT64_C(5));
    r.d = (uint64_t)(~v);
    return r;
}

static B40m mk40m(uint64_t v) {
    B40m r = {0, 0.0, 0, 0, 0.0, 0};
    r.a = v;
    r.x = (double)(v & UINT64_C(8191)) / 2.0;
    r.b = (uint32_t)v;
    r.c = (uint32_t)(v >> 16);
    r.y = (double)(v & UINT64_C(131071)) / 32.0;
    r.d = (uint64_t)(v * UINT64_C(7));
    return r;
}

static B48 mk48(uint64_t v) {
    B48 r = {0, 0, 0, 0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v + UINT64_C(1));
    r.c = (uint64_t)(v + UINT64_C(2));
    r.d = (uint64_t)(v * UINT64_C(9));
    r.e = (uint64_t)(~v);
    r.f = (uint64_t)(v ^ UINT64_C(2863311530));
    return r;
}

static B24 bump24(B24 r, uint64_t d) {
    B24 o = {0, 0, 0};
    o.a = (uint64_t)(r.a + d);
    o.b = (uint64_t)(r.b ^ d);
    o.c = (uint64_t)(r.c + r.a);
    return o;
}

static B48 bump48(B48 r, uint64_t d) {
    B48 o = {0, 0, 0, 0, 0, 0};
    o.a = (uint64_t)(r.a + d);
    o.b = (uint64_t)(r.b ^ d);
    o.c = (uint64_t)(r.c + r.a);
    o.d = (uint64_t)(r.d + r.b);
    o.e = (uint64_t)(r.e ^ r.c);
    o.f = (uint64_t)(r.f + d);
    return o;
}

static B48 widen(B24 r) {
    B48 o = {0, 0, 0, 0, 0, 0};
    o.a = r.a;
    o.b = r.b;
    o.c = r.c;
    o.d = (uint64_t)(r.a ^ r.b);
    o.e = (uint64_t)(r.b ^ r.c);
    o.f = (uint64_t)(r.c ^ r.a);
    return o;
}

static B24 narrow(B48 r) {
    B24 o = {0, 0, 0};
    o.a = (uint64_t)(r.a ^ r.d);
    o.b = (uint64_t)(r.b ^ r.e);
    o.c = (uint64_t)(r.c ^ r.f);
    return o;
}

static Nest nest_of(uint64_t v) {
    Nest r = { 0, {0, 0, 0}, {0, 0} };
    r.lead = (uint32_t)v;
    r.big = mk24(v);
    r.tail.a = (uint64_t)(v * UINT64_C(11));
    r.tail.b = (uint64_t)(~v);
    return r;
}

static uint64_t take_two(B48 p, B24 q, uint64_t x) {
    uint64_t h = fold_init();
    h = fold48(h, p);
    h = fold24(h, q);
    return mix_u64(h, x);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = layout(fold_init());

    uint64_t v[8];
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t w = (uint64_t)(v[k] + seed);
        h = fold20(h, mk20(w));
        h = fold24(h, mk24(w));
        h = fold24f(h, mk24f(w));
        h = fold32(h, mk32(w));
        h = fold40m(h, mk40m(w));
        h = fold48(h, mk48(w));
        h = fold_nest(h, nest_of(w));
    }

    B24 acc = mk24((uint64_t)(seed + UINT64_C(1)));
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        acc = bump24(acc, v[k]);
        h = fold24(h, acc);
    }

    B48 big = mk48((uint64_t)(seed + UINT64_C(2)));
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        big = bump48(big, v[k]);
        h = fold48(h, big);
    }

    B32 arr[6];
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        arr[k] = mk32((uint64_t)(v[k] + seed));
    }
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold32(h, arr[k]);
    }

    Nest n = { 0, {0, 0, 0}, {0, 0} };
    n.lead = UINT32_C(305419896);
    n.big = bump24(mk24((uint64_t)(seed + UINT64_C(3))), v[0]);
    n.tail.a = v[1];
    n.tail.b = v[2];
    h = fold_nest(h, n);

    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t w = (uint64_t)(v[k] + seed);
        h = mix_u64(h, take_two(widen(mk24(w)), narrow(mk48(w)), w));
        h = fold48(h, widen(narrow(mk48(w))));
        h = fold24(h, narrow(widen(mk24(w))));
        h = fold24(h, bump24(narrow(bump48(mk48(w), w)), w));
    }

    return h;
}
