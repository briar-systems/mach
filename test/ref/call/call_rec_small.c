#include "corpus.h"

#include <stddef.h>

typedef struct { uint8_t a; } R1;
typedef struct { uint8_t a; uint8_t b; } R2;
typedef struct { uint8_t a; uint8_t b; uint16_t c; } R4;
typedef struct { uint32_t a; uint16_t b; uint8_t c; } R8;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold_r1(uint64_t h, R1 r) { return mix_u8(h, r.a); }

static uint64_t fold_r2(uint64_t h, R2 r) {
    h = mix_u8(h, r.a);
    return mix_u8(h, r.b);
}

static uint64_t fold_r4(uint64_t h, R4 r) {
    h = mix_u8(h, r.a);
    h = mix_u8(h, r.b);
    return mix_u16(h, r.c);
}

static uint64_t fold_r8(uint64_t h, R8 r) {
    h = mix_u32(h, r.a);
    h = mix_u16(h, r.b);
    return mix_u8(h, r.c);
}

static uint64_t layout(uint64_t h) {
    h = mix_u64(h, (uint64_t)sizeof(R1));
    h = mix_u64(h, (uint64_t)sizeof(R2));
    h = mix_u64(h, (uint64_t)sizeof(R4));
    h = mix_u64(h, (uint64_t)sizeof(R8));
    h = mix_u64(h, (uint64_t)_Alignof(R1));
    h = mix_u64(h, (uint64_t)_Alignof(R2));
    h = mix_u64(h, (uint64_t)_Alignof(R4));
    h = mix_u64(h, (uint64_t)_Alignof(R8));
    h = mix_u64(h, (uint64_t)offsetof(R2, b));
    h = mix_u64(h, (uint64_t)offsetof(R4, b));
    h = mix_u64(h, (uint64_t)offsetof(R4, c));
    h = mix_u64(h, (uint64_t)offsetof(R8, b));
    h = mix_u64(h, (uint64_t)offsetof(R8, c));
    return h;
}

static uint64_t take_small(uint64_t p0, R1 r1, uint32_t p1, R2 r2, double x0,
                           R4 r4, uint64_t p2, R8 r8, R1 s1, R2 s2, R4 s4,
                           R8 s8, float x1, uint64_t p3) {
    uint64_t h = fold_init();
    h = mix_u64(h, p0);
    h = fold_r1(h, r1);
    h = mix_u32(h, p1);
    h = fold_r2(h, r2);
    h = mix_f64(h, x0);
    h = fold_r4(h, r4);
    h = mix_u64(h, p2);
    h = fold_r8(h, r8);
    h = fold_r1(h, s1);
    h = fold_r2(h, s2);
    h = fold_r4(h, s4);
    h = fold_r8(h, s8);
    h = mix_f32(h, x1);
    h = mix_u64(h, p3);

    R8 c8 = r8;
    c8.a = (uint32_t)(c8.a + UINT32_C(1));
    c8.b = (uint16_t)(c8.b + 2u);
    c8.c = (uint8_t)(c8.c + 3u);
    h = fold_r8(h, c8);
    h = fold_r8(h, r8);
    return h;
}

static R1 mk1(uint64_t v) { R1 r = {0}; r.a = (uint8_t)v; return r; }

static R2 mk2(uint64_t v) {
    R2 r = {0};
    r.a = (uint8_t)v;
    r.b = (uint8_t)(v >> 8);
    return r;
}

static R4 mk4(uint64_t v) {
    R4 r = {0};
    r.a = (uint8_t)v;
    r.b = (uint8_t)(v >> 8);
    r.c = (uint16_t)(v >> 16);
    return r;
}

static R8 mk8(uint64_t v) {
    R8 r = {0};
    r.a = (uint32_t)v;
    r.b = (uint16_t)(v >> 32);
    r.c = (uint8_t)(v >> 48);
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
        const uint64_t d = (uint64_t)(pass * UINT64_C(13) + seed);
        r = take_small((uint64_t)(v[0] + d), mk1(v[1]), (uint32_t)v[2], mk2(v[3]),
                       (double)(v[4] & UINT64_C(1023)), mk4(v[5]), v[6], mk8((uint64_t)(v[7] + d)),
                       mk1(v[8]), mk2(v[9]), mk4(v[10]), mk8(v[11]),
                       (float)(v[0] & UINT64_C(255)), v[1]);
        h = mix_u64(h, r);
    }

    h = mix_u64(h, take_small(
        r, mk1(r), (uint32_t)r, mk2(r), (double)(r & UINT64_C(1023)), mk4(r), v[6],
        mk8(r), mk1(v[8]), mk2(v[9]), mk4(v[10]), mk8(v[11]),
        (float)(v[0] & UINT64_C(255)), v[2]));

    return h;
}
