#include "corpus.h"
#include <stddef.h>

typedef struct { uint8_t d; union { int64_t value; } p; } Reply;
typedef struct { uint16_t d; union { uint8_t small; uint64_t big; } p; } Wide;
typedef struct { uint32_t d; union { uint16_t pair[2]; uint32_t word; } p; } Quad;
typedef struct __attribute__((packed)) { uint8_t d; union { uint64_t value; } p; } Packed;
typedef struct __attribute__((aligned(16))) { uint8_t d; union { uint8_t value; } p; } Aligned;
typedef struct { uint8_t d; union { float half; double full; } p; } Float;

static uint64_t wide_of(Wide w) {
    uint64_t out = 0;
    if (w.d == 1) { out = (uint64_t)w.p.small; }
    else if (w.d == 2) { out = w.p.big; }
    return out;
}

static uint32_t quad_of(Quad q) {
    uint32_t out = 0;
    if (q.d == 1) { out = (uint32_t)(((uint32_t)q.p.pair[0] << 16) | (uint32_t)q.p.pair[1]); }
    else if (q.d == 2) { out = q.p.word; }
    return out;
}

static uint64_t packed_of(Packed p) {
    if (p.d == 1) { return p.p.value; }
    return 0;
}

static uint8_t aligned_of(Aligned a) {
    if (a.d == 1) { return a.p.value; }
    return 0;
}

static uint64_t float_of(uint64_t h, Float f) {
    uint64_t out = mix_u8(h, 0);
    if (f.d == 1) { out = mix_f32(h, f.p.half); }
    else if (f.d == 2) { out = mix_f64(h, f.p.full); }
    return out;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    h = mix_u64(h, (uint64_t)sizeof(Reply));
    h = mix_u64(h, (uint64_t)_Alignof(Reply));
    h = mix_u64(h, (uint64_t)offsetof(Reply, p));
    h = mix_u64(h, (uint64_t)sizeof(uint8_t));
    h = mix_u64(h, (uint64_t)sizeof(Wide));
    h = mix_u64(h, (uint64_t)_Alignof(Wide));
    h = mix_u64(h, (uint64_t)offsetof(Wide, p));
    h = mix_u64(h, (uint64_t)offsetof(Wide, p));
    h = mix_u64(h, (uint64_t)sizeof(uint16_t));
    h = mix_u64(h, (uint64_t)sizeof(Quad));
    h = mix_u64(h, (uint64_t)_Alignof(Quad));
    h = mix_u64(h, (uint64_t)offsetof(Quad, p));
    h = mix_u64(h, (uint64_t)offsetof(Quad, p));
    h = mix_u64(h, (uint64_t)sizeof(uint32_t));
    h = mix_u64(h, (uint64_t)sizeof(Packed));
    h = mix_u64(h, (uint64_t)_Alignof(Packed));
    h = mix_u64(h, (uint64_t)offsetof(Packed, p));
    h = mix_u64(h, (uint64_t)sizeof(Aligned));
    h = mix_u64(h, (uint64_t)_Alignof(Aligned));
    h = mix_u64(h, (uint64_t)offsetof(Aligned, p));
    h = mix_u64(h, (uint64_t)sizeof(Float));
    h = mix_u64(h, (uint64_t)_Alignof(Float));
    h = mix_u64(h, (uint64_t)offsetof(Float, p));
    h = mix_u64(h, (uint64_t)offsetof(Float, p));

    Wide w;
    memset(&w, 0, sizeof w);
    h = mix_u64(h, wide_of(w));
    memset(&w, 0, sizeof w); w.p.small = (uint8_t)(UINT32_C(3) + s); w.d = 1;
    h = mix_u64(h, wide_of(w));
    memset(&w, 0, sizeof w); w.p.big = (uint64_t)(UINT64_C(4041206221) + seed); w.d = 2;
    h = mix_u64(h, wide_of(w));
    if (w.d == 2) { w.p.big = (uint64_t)(w.p.big ^ UINT64_C(65535)); }
    h = mix_u64(h, wide_of(w));

    Quad q;
    memset(&q, 0, sizeof q);
    h = mix_u32(h, quad_of(q));
    memset(&q, 0, sizeof q); q.p.pair[0] = (uint16_t)(UINT32_C(5) + s); q.p.pair[1] = (uint16_t)(UINT32_C(6) + s); q.d = 1;
    h = mix_u32(h, quad_of(q));
    if (q.d == 1) { q.p.pair[1] = (uint16_t)(q.p.pair[1] + UINT16_C(256)); }
    h = mix_u32(h, quad_of(q));
    memset(&q, 0, sizeof q); q.p.word = (uint32_t)(UINT32_C(3735928559) + s); q.d = 2;
    h = mix_u32(h, quad_of(q));

    Packed p;
    memset(&p, 0, sizeof p);
    h = mix_u64(h, packed_of(p));
    memset(&p, 0, sizeof p); p.p.value = (uint64_t)(UINT64_C(81985529216486895) + seed); p.d = 1;
    h = mix_u64(h, packed_of(p));
    if (p.d == 1) { p.p.value = (uint64_t)(p.p.value + UINT64_C(1)); }
    h = mix_u64(h, packed_of(p));
    const Packed pc = p;
    h = mix_u64(h, packed_of(pc));

    Aligned al;
    memset(&al, 0, sizeof al);
    h = mix_u8(h, aligned_of(al));
    memset(&al, 0, sizeof al); al.p.value = (uint8_t)(UINT32_C(9) + s); al.d = 1;
    h = mix_u8(h, aligned_of(al));
    const Aligned ac = al;
    h = mix_u8(h, aligned_of(ac));

    Float f;
    memset(&f, 0, sizeof f);
    h = float_of(h, f);
    memset(&f, 0, sizeof f); f.p.half = 1.5f + (float)s; f.d = 1;
    h = float_of(h, f);
    memset(&f, 0, sizeof f); f.p.full = -2.25 + (double)s; f.d = 2;
    h = float_of(h, f);
    return h;
}
