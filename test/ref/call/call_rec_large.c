#include "corpus.h"

#include <stddef.h>

typedef struct { uint64_t a; uint64_t b; } N16;
typedef struct { uint64_t a; uint64_t b; uint64_t c; } L24;
typedef struct { double x; double y; double z; } L24f;
typedef struct { uint64_t a; double x; uint32_t b; uint32_t c; } L24m;
typedef struct { uint64_t a; uint64_t b; uint64_t c; uint64_t d; } L32;
typedef struct { N16 lo; N16 hi; } L32n;
typedef struct { uint64_t a; uint64_t b; uint64_t c; uint64_t d; uint64_t e; uint64_t f; } L48;
typedef struct { uint64_t a; double x; uint32_t b; uint32_t c; double y; uint64_t d; uint64_t e; } L48m;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold_l24(uint64_t h, L24 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    return mix_u64(h, r.c);
}

static uint64_t fold_l24f(uint64_t h, L24f r) {
    h = mix_f64(h, r.x);
    h = mix_f64(h, r.y);
    return mix_f64(h, r.z);
}

static uint64_t fold_l24m(uint64_t h, L24m r) {
    h = mix_u64(h, r.a);
    h = mix_f64(h, r.x);
    h = mix_u32(h, r.b);
    return mix_u32(h, r.c);
}

static uint64_t fold_l32(uint64_t h, L32 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    return mix_u64(h, r.d);
}

static uint64_t fold_l32n(uint64_t h, L32n r) {
    h = mix_u64(h, r.lo.a);
    h = mix_u64(h, r.lo.b);
    h = mix_u64(h, r.hi.a);
    return mix_u64(h, r.hi.b);
}

static uint64_t fold_l48(uint64_t h, L48 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    h = mix_u64(h, r.d);
    h = mix_u64(h, r.e);
    return mix_u64(h, r.f);
}

static uint64_t fold_l48m(uint64_t h, L48m r) {
    h = mix_u64(h, r.a);
    h = mix_f64(h, r.x);
    h = mix_u32(h, r.b);
    h = mix_u32(h, r.c);
    h = mix_f64(h, r.y);
    h = mix_u64(h, r.d);
    return mix_u64(h, r.e);
}

static uint64_t layout(uint64_t h) {
    h = mix_u64(h, (uint64_t)sizeof(L24));
    h = mix_u64(h, (uint64_t)sizeof(L24f));
    h = mix_u64(h, (uint64_t)sizeof(L24m));
    h = mix_u64(h, (uint64_t)sizeof(L32));
    h = mix_u64(h, (uint64_t)sizeof(L32n));
    h = mix_u64(h, (uint64_t)sizeof(L48));
    h = mix_u64(h, (uint64_t)sizeof(L48m));
    h = mix_u64(h, (uint64_t)_Alignof(L24));
    h = mix_u64(h, (uint64_t)_Alignof(L24f));
    h = mix_u64(h, (uint64_t)_Alignof(L48m));
    h = mix_u64(h, (uint64_t)offsetof(L24, c));
    h = mix_u64(h, (uint64_t)offsetof(L24m, b));
    h = mix_u64(h, (uint64_t)offsetof(L24m, c));
    h = mix_u64(h, (uint64_t)offsetof(L32n, hi));
    h = mix_u64(h, (uint64_t)offsetof(L48, f));
    h = mix_u64(h, (uint64_t)offsetof(L48m, y));
    h = mix_u64(h, (uint64_t)offsetof(L48m, e));
    return h;
}

static uint64_t take_large(uint64_t p0, L24 l24, L24f l24f, L24m l24m,
                           uint32_t p1, L32 l32, L32n l32n, double x0,
                           L48 l48, L48m l48m, L24 t24, L32 t32, L48 t48,
                           float x1, uint64_t p2) {
    uint64_t h = fold_init();
    h = mix_u64(h, p0);
    h = fold_l24(h, l24);
    h = fold_l24f(h, l24f);
    h = fold_l24m(h, l24m);
    h = mix_u32(h, p1);
    h = fold_l32(h, l32);
    h = fold_l32n(h, l32n);
    h = mix_f64(h, x0);
    h = fold_l48(h, l48);
    h = fold_l48m(h, l48m);
    h = fold_l24(h, t24);
    h = fold_l32(h, t32);
    h = fold_l48(h, t48);
    h = mix_f32(h, x1);
    h = mix_u64(h, p2);

    L48 c48 = l48;
    c48.a = (uint64_t)(c48.a + UINT64_C(1));
    c48.f = (uint64_t)(c48.f ^ UINT64_C(4294967295));
    h = fold_l48(h, c48);
    h = fold_l48(h, l48);
    return h;
}

static L24 mk24(uint64_t v) {
    L24 r = {0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v * UINT64_C(3) + UINT64_C(1));
    r.c = (uint64_t)(v ^ UINT64_C(305419896));
    return r;
}

static L24f mk24f(uint64_t v) {
    L24f r = {0.0, 0.0, 0.0};
    r.x = (double)(v & UINT64_C(65535)) / 8.0;
    r.y = (double)(v & UINT64_C(4095)) - 2048.0;
    r.z = (double)(v & UINT64_C(262143)) / 64.0;
    return r;
}

static L24m mk24m(uint64_t v) {
    L24m r = {0, 0.0, 0, 0};
    r.a = v;
    r.x = (double)(v & UINT64_C(32767)) / 4.0;
    r.b = (uint32_t)v;
    r.c = (uint32_t)(v >> 32);
    return r;
}

static L32 mk32(uint64_t v) {
    L32 r = {0, 0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(~v);
    r.c = (uint64_t)(v * UINT64_C(5));
    r.d = (uint64_t)(v >> 3);
    return r;
}

static L32n mk32n(uint64_t v) {
    L32n r = { {0, 0}, {0, 0} };
    r.lo.a = v;
    r.lo.b = (uint64_t)(v + UINT64_C(11));
    r.hi.a = (uint64_t)(v * UINT64_C(7));
    r.hi.b = (uint64_t)(~v);
    return r;
}

static L48 mk48(uint64_t v) {
    L48 r = {0, 0, 0, 0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v + UINT64_C(1));
    r.c = (uint64_t)(v + UINT64_C(2));
    r.d = (uint64_t)(v * UINT64_C(9));
    r.e = (uint64_t)(~v);
    r.f = (uint64_t)(v ^ UINT64_C(2863311530));
    return r;
}

static L48m mk48m(uint64_t v) {
    L48m r = {0, 0.0, 0, 0, 0.0, 0, 0};
    r.a = v;
    r.x = (double)(v & UINT64_C(8191)) / 2.0;
    r.b = (uint32_t)v;
    r.c = (uint32_t)(v >> 16);
    r.y = (double)(v & UINT64_C(131071)) / 32.0;
    r.d = (uint64_t)(v * UINT64_C(3));
    r.e = (uint64_t)(~v);
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = layout(fold_init());

    uint64_t v[12];
    for (uint64_t i = 0; i < UINT64_C(12); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    uint64_t r = 0;
    for (uint64_t pass = 0; pass < UINT64_C(3); pass = (uint64_t)(pass + UINT64_C(1))) {
        const uint64_t d = (uint64_t)(pass * UINT64_C(19) + seed);
        r = take_large((uint64_t)(v[0] + d), mk24(v[1]), mk24f(v[2]), mk24m(v[3]), (uint32_t)v[4],
                       mk32((uint64_t)(v[5] + d)), mk32n(v[6]), (double)(v[7] & UINT64_C(1023)),
                       mk48(v[8]), mk48m(v[9]), mk24(v[10]), mk32(v[11]),
                       mk48(v[0]), (float)(v[1] & UINT64_C(255)), v[2]);
        h = mix_u64(h, r);
    }

    h = mix_u64(h, take_large(
        r, mk24(r), mk24f(r), mk24m(r), (uint32_t)r, mk32(r), mk32n(r),
        (double)(r & UINT64_C(1023)), mk48(r), mk48m(r), mk24(v[10]), mk32(v[11]),
        mk48(v[0]), (float)(v[1] & UINT64_C(255)), v[3]));

    return h;
}
