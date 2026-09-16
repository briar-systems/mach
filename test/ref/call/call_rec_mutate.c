#include "corpus.h"

typedef struct { uint32_t a; uint32_t b; } M8;
typedef struct { uint64_t a; uint64_t b; } M16;
typedef struct { uint64_t a; uint64_t b; uint64_t c; } M24;
typedef struct {
    uint64_t a; uint64_t b; uint64_t c; uint64_t d;
    uint64_t e; uint64_t f; uint64_t g; uint64_t h;
} M64;
typedef struct __attribute__((aligned(16))) { uint8_t a; uint8_t b; } MA16;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t fold8(uint64_t h, M8 r) {
    h = mix_u32(h, r.a);
    return mix_u32(h, r.b);
}

static uint64_t fold16(uint64_t h, M16 r) {
    h = mix_u64(h, r.a);
    return mix_u64(h, r.b);
}

static uint64_t fold24(uint64_t h, M24 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    return mix_u64(h, r.c);
}

static uint64_t fold64(uint64_t h, M64 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    h = mix_u64(h, r.d);
    h = mix_u64(h, r.e);
    h = mix_u64(h, r.f);
    h = mix_u64(h, r.g);
    return mix_u64(h, r.h);
}

static uint64_t folda(uint64_t h, MA16 r) {
    h = mix_u8(h, r.a);
    return mix_u8(h, r.b);
}

static uint64_t clobber8(M8 v, uint64_t k) {
    M8 r = v;
    r.a = (uint32_t)k;
    r.b = (uint32_t)(k >> 32);
    return fold8(fold8(fold_init(), v), r);
}

static uint64_t clobber16(M16 v, uint64_t k) {
    v.a = k;
    v.b = (uint64_t)~k;
    return fold16(fold_init(), v);
}

static uint64_t clobber24(M24 v, uint64_t k) {
    v.a = k;
    v.b = (uint64_t)~k;
    v.c = (uint64_t)(k ^ UINT64_C(0x5555555555555555));
    return fold24(fold_init(), v);
}

static uint64_t clobber64(M64 v, uint64_t k) {
    v.a = k;
    v.b = (uint64_t)~k;
    v.c = (uint64_t)(k + UINT64_C(3));
    v.d = (uint64_t)(k * UINT64_C(7));
    v.e = (uint64_t)(k ^ UINT64_C(0x0F0F0F0F0F0F0F0F));
    v.f = (uint64_t)(k - UINT64_C(11));
    v.g = (uint64_t)(k >> 3);
    v.h = (uint64_t)(k << 5);
    return fold64(fold_init(), v);
}

static uint64_t clobbera(MA16 v, uint64_t k) {
    v.a = (uint8_t)k;
    v.b = (uint8_t)(k >> 8);
    return folda(fold_init(), v);
}

static uint64_t clobber16_indirect(M16 v, uint64_t k) {
    v.a = (uint64_t)(k + UINT64_C(1));
    v.b = (uint64_t)(k + UINT64_C(2));
    return fold16(fold_init(), v);
}

static uint64_t twice64(M64 x, M64 y) {
    return fold64(fold64(fold_init(), x), y);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, (uint64_t)sizeof(M8));
    h = mix_u64(h, (uint64_t)sizeof(M16));
    h = mix_u64(h, (uint64_t)sizeof(M24));
    h = mix_u64(h, (uint64_t)sizeof(M64));
    h = mix_u64(h, (uint64_t)sizeof(MA16));
    h = mix_u64(h, (uint64_t)_Alignof(MA16));

    uint64_t v[8];
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    M8 r8 = { (uint32_t)v[0], (uint32_t)v[1] };
    M16 r16 = { v[2], v[3] };
    M24 r24 = { v[4], v[5], v[6] };
    M64 r64 = { v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7] };
    MA16 ra = { (uint8_t)v[7], (uint8_t)(v[7] >> 8) };

    for (uint64_t pass = 0; pass < UINT64_C(3); pass = (uint64_t)(pass + UINT64_C(1))) {
        const uint64_t k = gen(seed, (uint64_t)(pass + UINT64_C(17)));

        h = mix_u64(h, clobber8(r8, k));
        h = fold8(h, r8);

        h = mix_u64(h, clobber16(r16, k));
        h = fold16(h, r16);

        h = mix_u64(h, clobber24(r24, k));
        h = fold24(h, r24);

        h = mix_u64(h, clobber64(r64, k));
        h = fold64(h, r64);

        h = mix_u64(h, clobbera(ra, k));
        h = folda(h, ra);

        uint64_t (*f)(M16, uint64_t) = clobber16_indirect;
        h = mix_u64(h, f(r16, k));
        h = fold16(h, r16);

        h = mix_u64(h, twice64(r64, r64));
        h = fold64(h, r64);
    }

    return h;
}
