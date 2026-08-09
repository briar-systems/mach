#include "corpus.h"

/* one C struct per mach shape, each with the shape's own size and alignment: u32x4,
 * f32x4 and i8x16 fill the vector register so they align to 16, f64x2 likewise.
 *
 * integer lanes are held unsigned and every lane operation widens before it
 * computes, so nothing here can overflow a signed type. sx8 is the same
 * two's-complement sign extension mix_i8 performs. */
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;
typedef struct { _Alignas(16) float l[4]; } f32x4;
typedef struct { _Alignas(16) double l[2]; } f64x2;
typedef struct { _Alignas(16) uint8_t l[16]; } i8x16;

static uint64_t sx8(uint8_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x80)) - UINT64_C(0x80));
}

static uint64_t fold_u32x4(uint64_t h, u32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_u32(h, v.l[i]); }
    return h;
}

static uint64_t fold_f32x4(uint64_t h, f32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

static uint64_t fold_f64x2(uint64_t h, f64x2 v) {
    for (unsigned i = 0; i < 2u; i++) { h = mix_f64(h, v.l[i]); }
    return h;
}

static uint64_t fold_i8x16(uint64_t h, i8x16 v) {
    for (unsigned i = 0; i < 16u; i++) { h = mix_u64(h, sx8(v.l[i])); }
    return h;
}

#define N8(x) ((uint8_t)-(uint32_t)(x))

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;
    const float z = (float)seed;
    const double d = (double)seed;
    const uint8_t e = (uint8_t)seed;

    u32x4 u = { { UINT32_C(286331153), UINT32_C(572662306), UINT32_C(858993459), UINT32_C(1145324612) } };
    u.l[0] = (uint32_t)(u.l[0] + s);
    u.l[3] = (uint32_t)(u.l[3] + s);
    h = fold_u32x4(h, u);

    u32x4 rev;
    rev.l[0] = u.l[3];
    rev.l[1] = u.l[2];
    rev.l[2] = u.l[1];
    rev.l[3] = u.l[0];
    h = fold_u32x4(h, rev);

    u32x4 rot;
    rot.l[0] = u.l[1];
    rot.l[1] = u.l[2];
    rot.l[2] = u.l[3];
    rot.l[3] = u.l[0];
    h = fold_u32x4(h, rot);

    u32x4 swap;
    swap.l[0] = u.l[2];
    swap.l[1] = u.l[3];
    swap.l[2] = u.l[0];
    swap.l[3] = u.l[1];
    h = fold_u32x4(h, swap);

    u32x4 bcast;
    bcast.l[0] = u.l[2];
    bcast.l[1] = u.l[2];
    bcast.l[2] = u.l[2];
    bcast.l[3] = u.l[2];
    h = fold_u32x4(h, bcast);

    /* mach default-initializes an uninitialized vector local to all-zero lanes */
    u32x4 sparse = { { 0, 0, 0, 0 } };
    sparse.l[1] = u.l[0];
    h = fold_u32x4(h, sparse);

    u32x4 over = u;
    over.l[1] = u.l[0];
    over.l[1] = u.l[3];
    h = fold_u32x4(h, over);

    u32x4 seq = { { 0, 0, 0, 0 } };
    seq.l[0] = (uint32_t)(u.l[0] + UINT32_C(1));
    h = mix_u32(h, seq.l[0]);
    seq.l[1] = (uint32_t)(seq.l[0] + u.l[1]);
    h = mix_u32(h, seq.l[1]);
    seq.l[2] = (uint32_t)(seq.l[1] ^ u.l[2]);
    h = mix_u32(h, seq.l[2]);
    seq.l[3] = (uint32_t)(seq.l[2] - u.l[3]);
    h = mix_u32(h, seq.l[3]);
    h = fold_u32x4(h, seq);

    u32x4 t;
    for (unsigned i = 0; i < 4u; i++) { t.l[i] = (uint32_t)(rev.l[i] + u.l[i]); }
    h = fold_u32x4(h, t);
    for (unsigned i = 0; i < 4u; i++) { t.l[i] = (uint32_t)(rot.l[i] - u.l[i]); }
    h = fold_u32x4(h, t);
    for (unsigned i = 0; i < 4u; i++) { t.l[i] = (uint32_t)(swap.l[i] * u.l[i]); }
    h = fold_u32x4(h, t);
    for (unsigned i = 0; i < 4u; i++) { t.l[i] = (uint32_t)(rev.l[i] ^ rot.l[i]); }
    h = fold_u32x4(h, t);

    f32x4 f = { { 1.5f, -2.25f, 3.75f, -0.5f } };
    f.l[1] = f.l[1] + z;
    h = fold_f32x4(h, f);

    f32x4 frev;
    frev.l[0] = f.l[3];
    frev.l[1] = f.l[2];
    frev.l[2] = f.l[1];
    frev.l[3] = f.l[0];
    h = fold_f32x4(h, frev);

    f32x4 mix;
    mix.l[0] = f.l[0];
    mix.l[1] = frev.l[1];
    mix.l[2] = f.l[2];
    mix.l[3] = frev.l[3];
    h = fold_f32x4(h, mix);
    f32x4 fp;
    for (unsigned i = 0; i < 4u; i++) { fp.l[i] = mix.l[i] * frev.l[i]; }
    h = fold_f32x4(h, fp);
    h = mix_f32(h, f.l[0] - frev.l[0]);
    h = mix_f32(h, f.l[3] - frev.l[3]);

    f64x2 g = { { 1.5, -2.25 } };
    g.l[0] = g.l[0] + d;
    f64x2 gswap;
    gswap.l[0] = g.l[1];
    gswap.l[1] = g.l[0];
    h = fold_f64x2(h, g);
    h = fold_f64x2(h, gswap);
    f64x2 gd;
    for (unsigned i = 0; i < 2u; i++) { gd.l[i] = gswap.l[i] - g.l[i]; }
    h = fold_f64x2(h, gd);

    i8x16 q = { { N8(9), 7, N8(5), 3, N8(1), 2, N8(4), 6, N8(8), 9, N8(3), 1, N8(2), 4, N8(6), 8 } };
    q.l[0] = (uint8_t)((uint32_t)q.l[0] + (uint32_t)e);
    q.l[15] = (uint8_t)((uint32_t)q.l[15] + (uint32_t)e);
    h = fold_i8x16(h, q);

    i8x16 qrev;
    for (unsigned i = 0; i < 16u; i++) { qrev.l[i] = q.l[15u - i]; }
    h = fold_i8x16(h, qrev);
    i8x16 qt;
    for (unsigned i = 0; i < 16u; i++) { qt.l[i] = (uint8_t)((uint32_t)qrev.l[i] + (uint32_t)q.l[i]); }
    h = fold_i8x16(h, qt);
    for (unsigned i = 0; i < 16u; i++) { qt.l[i] = (uint8_t)((uint32_t)qrev.l[i] ^ (uint32_t)q.l[i]); }
    h = fold_i8x16(h, qt);
    return h;
}
