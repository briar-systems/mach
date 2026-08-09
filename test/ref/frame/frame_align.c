#include "corpus.h"

/* mach's whole-register vector shapes are size 16, align 16, so a C struct over
 * the lanes carrying _Alignas(16) has the same size, the same alignment and the
 * same array stride. `Wide` asks for 64 the same way mach's #[align(64)] does,
 * which rounds its size up to 64 on both sides. */
typedef struct { _Alignas(16) float l[4]; } f32x4;
typedef struct { _Alignas(16) double l[2]; } f64x2;
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;
typedef struct { _Alignas(64) uint64_t a; uint64_t b; } Wide;

static f32x4 f4add(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x4 f4mul(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f64x2 d2mul(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f64x2 d2sub(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f64x2 d2div(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static u32x4 u4mul(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] * y.l[i]); }
    return r;
}

static u32x4 u4add(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] + y.l[i]); }
    return r;
}

static u32x4 u4xor(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] ^ y.l[i]); }
    return r;
}

static uint64_t fold_f32x4(uint64_t h, f32x4 v) {
    h = mix_f32(h, v.l[0]);
    h = mix_f32(h, v.l[1]);
    h = mix_f32(h, v.l[2]);
    h = mix_f32(h, v.l[3]);
    return h;
}

static uint64_t fold_f64x2(uint64_t h, f64x2 v) {
    h = mix_f64(h, v.l[0]);
    h = mix_f64(h, v.l[1]);
    return h;
}

static uint64_t fold_u32x4(uint64_t h, u32x4 v) {
    h = mix_u32(h, v.l[0]);
    h = mix_u32(h, v.l[1]);
    h = mix_u32(h, v.l[2]);
    h = mix_u32(h, v.l[3]);
    return h;
}

static f32x4 spin(f32x4 v) {
    uint8_t odd[7] = { 0 };
    f32x4 acc = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    const f32x4 m = { { 1.5f, 0.5f, 1.25f, 0.75f } };
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        odd[i] = (uint8_t)(i * UINT64_C(17));
        acc = f4add(acc, v);
        acc = f4mul(acc, m);
    }
    acc.l[0] = acc.l[0] + (float)odd[3];
    return acc;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;
    const float z = (float)seed;
    const double w = (double)seed;

    uint8_t pad0[3] = { 0 };
    f32x4 va = { { 1.5f, -2.25f, 3.75f, -0.5f } };
    uint8_t pad1[5] = { 0 };
    f64x2 da = { { 1.25, -3.5 } };
    uint16_t pad2 = 0;
    u32x4 ua = { { UINT32_C(4294967295), UINT32_C(1), UINT32_C(2654435761), UINT32_C(305419896) } };
    uint8_t pad3[1] = { 0 };
    f64x2 db = { { 0.5, 6.25 } };
    u32x4 ub = { { UINT32_C(3266489917), UINT32_C(2246822519), UINT32_C(7), UINT32_C(4294967291) } };
    Wide big;
    Wide arr[3];
    f32x4 vs[4];

    pad0[0] = (uint8_t)(s + UINT64_C(1));
    pad0[2] = (uint8_t)(s + UINT64_C(3));
    pad1[1] = (uint8_t)(s + UINT64_C(5));
    pad1[4] = (uint8_t)(s + UINT64_C(7));
    pad2 = (uint16_t)(s + UINT64_C(65521));
    pad3[0] = (uint8_t)(s + UINT64_C(251));

    va.l[0] = va.l[0] + z;
    da.l[1] = da.l[1] + w;
    ua.l[2] = (uint32_t)(ua.l[2] + (uint32_t)s);

    h = fold_f32x4(h, va);
    h = fold_f64x2(h, da);
    h = fold_u32x4(h, ua);
    h = fold_f32x4(h, f4add(va, va));
    h = fold_f32x4(h, f4mul(va, va));
    h = fold_f64x2(h, db);
    h = fold_f64x2(h, d2mul(da, db));
    h = fold_f64x2(h, d2sub(da, db));
    h = fold_f64x2(h, d2div(da, db));
    h = fold_u32x4(h, ub);
    h = fold_u32x4(h, u4mul(ua, ub));
    h = fold_u32x4(h, u4add(ua, ub));
    h = fold_u32x4(h, u4xor(ua, ub));

    h = fold_f32x4(h, spin(va));
    h = fold_f32x4(h, spin(f4mul(va, va)));

    vs[0] = va;
    vs[1] = f4add(va, va);
    vs[2] = spin(va);
    vs[3] = f4mul(va, va);
    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_f32x4(h, vs[k]);
    }

    big.a = (uint64_t)(UINT64_C(1229782938247303441) + s);
    big.b = (uint64_t)(UINT64_C(8608480567731124087) ^ s);
    h = mix_u64(h, big.a);
    h = mix_u64(h, big.b);

    for (uint64_t k = 0; k < UINT64_C(3); k = (uint64_t)(k + UINT64_C(1))) {
        arr[k].a = (uint64_t)((uint64_t)(k * UINT64_C(2654435761)) + s);
        arr[k].b = (uint64_t)((uint64_t)(k << 40) ^ big.a);
    }
    for (uint64_t k = 0; k < UINT64_C(3); k = (uint64_t)(k + UINT64_C(1))) {
        h = mix_u64(h, arr[k].a);
        h = mix_u64(h, arr[k].b);
    }

    h = mix_u8(h, pad0[0]);
    h = mix_u8(h, pad0[1]);
    h = mix_u8(h, pad0[2]);
    h = mix_u8(h, pad1[1]);
    h = mix_u8(h, pad1[4]);
    h = mix_u16(h, pad2);
    h = mix_u8(h, pad3[0]);
    return h;
}
