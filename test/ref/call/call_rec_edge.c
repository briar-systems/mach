#include "corpus.h"

#include <stddef.h>

typedef struct { uint8_t head; uint8_t body[8]; } E9;
typedef struct { uint32_t a; uint32_t b; uint32_t c; } E12;
typedef struct { uint64_t a; uint64_t b; } E16;
typedef struct { double x; double y; } E16f;
typedef struct { uint64_t a; double x; } E16m;
typedef struct { uint8_t head; uint8_t body[16]; } E17;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold_e9(uint64_t h, E9 r) {
    h = mix_u8(h, r.head);
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u8(h, r.body[i]);
    }
    return h;
}

static uint64_t fold_e12(uint64_t h, E12 r) {
    h = mix_u32(h, r.a);
    h = mix_u32(h, r.b);
    return mix_u32(h, r.c);
}

static uint64_t fold_e16(uint64_t h, E16 r) {
    h = mix_u64(h, r.a);
    return mix_u64(h, r.b);
}

static uint64_t fold_e16f(uint64_t h, E16f r) {
    h = mix_f64(h, r.x);
    return mix_f64(h, r.y);
}

static uint64_t fold_e16m(uint64_t h, E16m r) {
    h = mix_u64(h, r.a);
    return mix_f64(h, r.x);
}

static uint64_t fold_e17(uint64_t h, E17 r) {
    h = mix_u8(h, r.head);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u8(h, r.body[i]);
    }
    return h;
}

static uint64_t layout(uint64_t h) {
    h = mix_u64(h, (uint64_t)sizeof(E9));
    h = mix_u64(h, (uint64_t)sizeof(E12));
    h = mix_u64(h, (uint64_t)sizeof(E16));
    h = mix_u64(h, (uint64_t)sizeof(E16f));
    h = mix_u64(h, (uint64_t)sizeof(E16m));
    h = mix_u64(h, (uint64_t)sizeof(E17));
    h = mix_u64(h, (uint64_t)_Alignof(E9));
    h = mix_u64(h, (uint64_t)_Alignof(E12));
    h = mix_u64(h, (uint64_t)_Alignof(E16));
    h = mix_u64(h, (uint64_t)_Alignof(E16f));
    h = mix_u64(h, (uint64_t)_Alignof(E16m));
    h = mix_u64(h, (uint64_t)_Alignof(E17));
    h = mix_u64(h, (uint64_t)offsetof(E9, body));
    h = mix_u64(h, (uint64_t)offsetof(E12, c));
    h = mix_u64(h, (uint64_t)offsetof(E16, b));
    h = mix_u64(h, (uint64_t)offsetof(E16f, y));
    h = mix_u64(h, (uint64_t)offsetof(E16m, x));
    h = mix_u64(h, (uint64_t)offsetof(E17, body));
    return h;
}

static uint64_t take_edge(uint64_t p0, E9 e9, uint32_t p1, E12 e12, double x0,
                          E16 e16, E16f e16f, E16m e16m, uint64_t p2, E17 e17,
                          E9 t9, E12 t12, E16 t16, E17 t17, float x1, uint64_t p3) {
    uint64_t h = fold_init();
    h = mix_u64(h, p0);
    h = fold_e9(h, e9);
    h = mix_u32(h, p1);
    h = fold_e12(h, e12);
    h = mix_f64(h, x0);
    h = fold_e16(h, e16);
    h = fold_e16f(h, e16f);
    h = fold_e16m(h, e16m);
    h = mix_u64(h, p2);
    h = fold_e17(h, e17);
    h = fold_e9(h, t9);
    h = fold_e12(h, t12);
    h = fold_e16(h, t16);
    h = fold_e17(h, t17);
    h = mix_f32(h, x1);
    h = mix_u64(h, p3);

    E17 c17 = e17;
    c17.head = (uint8_t)(c17.head + 1u);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        c17.body[i] = (uint8_t)((uint8_t)(c17.body[i] + (uint8_t)i) + 1u);
    }
    h = fold_e17(h, c17);
    h = fold_e17(h, e17);
    return h;
}

static E9 mk9(uint64_t v) {
    E9 r = {0, {0}};
    r.head = (uint8_t)v;
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        r.body[i] = (uint8_t)((uint8_t)(v >> (i * UINT64_C(8))) + (uint8_t)i);
    }
    return r;
}

static E12 mk12(uint64_t v) {
    E12 r = {0, 0, 0};
    r.a = (uint32_t)v;
    r.b = (uint32_t)(v >> 16);
    r.c = (uint32_t)(v >> 32);
    return r;
}

static E16 mk16(uint64_t v) {
    E16 r = {0, 0};
    r.a = v;
    r.b = (uint64_t)(v * UINT64_C(3) + UINT64_C(7));
    return r;
}

static E16f mk16f(uint64_t v) {
    E16f r = {0.0, 0.0};
    r.x = (double)(v & UINT64_C(65535)) / 8.0;
    r.y = (double)(v & UINT64_C(4095)) - 2048.0;
    return r;
}

static E16m mk16m(uint64_t v) {
    E16m r = {0, 0.0};
    r.a = (uint64_t)(v ^ UINT64_C(305419896));
    r.x = (double)(v & UINT64_C(262143)) / 64.0;
    return r;
}

static E17 mk17(uint64_t v) {
    E17 r = {0, {0}};
    r.head = (uint8_t)(v >> 7);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        r.body[i] = (uint8_t)((uint8_t)(v >> (i * UINT64_C(4))) ^ (uint8_t)i);
    }
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
        const uint64_t d = (uint64_t)(pass * UINT64_C(17) + seed);
        r = take_edge((uint64_t)(v[0] + d), mk9(v[1]), (uint32_t)v[2], mk12(v[3]),
                      (double)(v[4] & UINT64_C(1023)), mk16((uint64_t)(v[5] + d)), mk16f(v[6]), mk16m(v[7]),
                      v[8], mk17(v[9]), mk9(v[10]), mk12(v[11]),
                      mk16(v[0]), mk17(v[1]), (float)(v[2] & UINT64_C(255)), v[3]);
        h = mix_u64(h, r);
    }

    h = mix_u64(h, take_edge(
        r, mk9(r), (uint32_t)r, mk12(r), (double)(r & UINT64_C(1023)), mk16(r), mk16f(r), mk16m(r),
        v[8], mk17(r), mk9(v[10]), mk12(v[11]), mk16(v[0]), mk17(v[1]),
        (float)(v[2] & UINT64_C(255)), v[4]));

    return h;
}
