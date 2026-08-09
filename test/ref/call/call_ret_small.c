#include "corpus.h"

typedef struct { uint8_t a; } S1;
typedef struct { uint16_t a; } S2;
typedef struct { uint32_t a; } S4;
typedef struct { uint32_t a; uint32_t b; } S8;
typedef struct { float x; float y; } S8f;
typedef struct { uint32_t a; uint32_t b; uint32_t c; } S12;
typedef struct { uint64_t a; uint64_t b; } S16;
typedef struct { double x; double y; } S16f;
typedef struct { uint64_t a; double x; } S16m;
typedef struct { double x; uint64_t a; } S16n;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static S1 ret1(uint64_t v) { S1 r = {0}; r.a = (uint8_t)v; return r; }
static S2 ret2(uint64_t v) { S2 r = {0}; r.a = (uint16_t)v; return r; }
static S4 ret4(uint64_t v) { S4 r = {0}; r.a = (uint32_t)v; return r; }

static S8 ret8(uint64_t v) {
    S8 r = {0, 0};
    r.a = (uint32_t)v;
    r.b = (uint32_t)(v >> 32);
    return r;
}

static S8f ret8f(uint64_t v) {
    S8f r = {0.0f, 0.0f};
    r.x = (float)(v & UINT64_C(4095)) / 8.0f;
    r.y = (float)(v & UINT64_C(255)) - 128.0f;
    return r;
}

static S12 ret12(uint64_t v) {
    S12 r = {0, 0, 0};
    r.a = (uint32_t)v;
    r.b = (uint32_t)(v >> 16);
    r.c = (uint32_t)(v >> 32);
    return r;
}

static S16 ret16(uint64_t v) {
    S16 r = {0, 0};
    r.a = v;
    r.b = (uint64_t)(~v);
    return r;
}

static S16f ret16f(uint64_t v) {
    S16f r = {0.0, 0.0};
    r.x = (double)(v & UINT64_C(65535)) / 8.0;
    r.y = (double)(v & UINT64_C(262143)) / 64.0;
    return r;
}

static S16m ret16m(uint64_t v) {
    S16m r = {0, 0.0};
    r.a = (uint64_t)(v * UINT64_C(3) + UINT64_C(1));
    r.x = (double)(v & UINT64_C(32767)) / 4.0;
    return r;
}

static S16n ret16n(uint64_t v) {
    S16n r = {0.0, 0};
    r.x = (double)(v & UINT64_C(8191)) / 2.0;
    r.a = (uint64_t)(v ^ UINT64_C(305419896));
    return r;
}

static uint64_t fold8(uint64_t h, S8 r) {
    h = mix_u32(h, r.a);
    return mix_u32(h, r.b);
}

static uint64_t fold8f(uint64_t h, S8f r) {
    h = mix_f32(h, r.x);
    return mix_f32(h, r.y);
}

static uint64_t fold12(uint64_t h, S12 r) {
    h = mix_u32(h, r.a);
    h = mix_u32(h, r.b);
    return mix_u32(h, r.c);
}

static uint64_t fold16(uint64_t h, S16 r) {
    h = mix_u64(h, r.a);
    return mix_u64(h, r.b);
}

static uint64_t fold16f(uint64_t h, S16f r) {
    h = mix_f64(h, r.x);
    return mix_f64(h, r.y);
}

static uint64_t fold16m(uint64_t h, S16m r) {
    h = mix_u64(h, r.a);
    return mix_f64(h, r.x);
}

static uint64_t fold16n(uint64_t h, S16n r) {
    h = mix_f64(h, r.x);
    return mix_u64(h, r.a);
}

static uint64_t layout(uint64_t h) {
    h = mix_u64(h, (uint64_t)sizeof(S1));
    h = mix_u64(h, (uint64_t)sizeof(S2));
    h = mix_u64(h, (uint64_t)sizeof(S4));
    h = mix_u64(h, (uint64_t)sizeof(S8));
    h = mix_u64(h, (uint64_t)sizeof(S8f));
    h = mix_u64(h, (uint64_t)sizeof(S12));
    h = mix_u64(h, (uint64_t)sizeof(S16));
    h = mix_u64(h, (uint64_t)sizeof(S16f));
    h = mix_u64(h, (uint64_t)sizeof(S16m));
    h = mix_u64(h, (uint64_t)sizeof(S16n));
    return h;
}

static uint64_t regain(S16 r16, S16f r16f, S16m r16m, S12 r12, S8 r8, S8f r8f) {
    uint64_t h = fold_init();
    h = fold16(h, r16);
    h = fold16f(h, r16f);
    h = fold16m(h, r16m);
    h = fold12(h, r12);
    h = fold8(h, r8);
    h = fold8f(h, r8f);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = layout(fold_init());

    uint64_t v[8];
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t w = (uint64_t)(v[k] + seed);
        h = mix_u8(h, ret1(w).a);
        h = mix_u16(h, ret2(w).a);
        h = mix_u32(h, ret4(w).a);
        h = fold8(h, ret8(w));
        h = fold8f(h, ret8f(w));
        h = fold12(h, ret12(w));
        h = fold16(h, ret16(w));
        h = fold16f(h, ret16f(w));
        h = fold16m(h, ret16m(w));
        h = fold16n(h, ret16n(w));
    }

    S12 a12[6];
    S16m a16[6];
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        a12[k] = ret12((uint64_t)(v[k] + seed));
        a16[k] = ret16m((uint64_t)(v[k] * UINT64_C(3) + seed));
    }
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold12(h, a12[k]);
        h = fold16m(h, a16[k]);
    }

    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t w = (uint64_t)(v[k] + seed);
        h = mix_u64(h, regain(ret16(w), ret16f(w), ret16m(w),
                              ret12(w), ret8(w), ret8f(w)));
    }

    return h;
}
