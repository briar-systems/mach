#include "corpus.h"

/* mach's i16x8 is 8 lanes x 2 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment.
 *
 * lanes are held as uint16_t and every lane operation widens to uint32_t before it
 * computes, so nothing here can overflow a signed type. the signed value reaches
 * the fold through the two's-complement sign-extension identity in sx16, which is
 * the same u64 mix_i16 produces. */
typedef struct { _Alignas(16) uint16_t l[8]; } i16x8;

typedef struct { uint32_t lead; i16x8 v; uint32_t trail; } Pair;

static uint64_t sx16(uint16_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x8000)) - UINT64_C(0x8000));
}

static i16x8 vadd(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] + (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vsub(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] - (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vmul(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] * (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vand(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] & (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vor(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] | (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vxor(i16x8 x, i16x8 y) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] ^ (uint32_t)y.l[i]); }
    return r;
}

static i16x8 vnot(i16x8 x) {
    i16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)(~(uint32_t)x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, i16x8 v) {
    for (unsigned i = 0; i < 8u; i++) { h = mix_u64(h, sx16(v.l[i])); }
    return h;
}

#define N16(x) ((uint16_t)-(uint32_t)(x))

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    i16x8 a = { { 1001, N16(1103), 1207, N16(1301), 1409, N16(1511), 1601, N16(1709) } };
    i16x8 b = { { 13, N16(11), 9, N16(7), 5, N16(3), 2, N16(1) } };
    const i16x8 ramp = { { 1, 2, 3, 4, 5, 6, 7, 8 } };
    const i16x8 zero = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
    a.l[0] = (uint16_t)((uint32_t)a.l[0] + (uint32_t)s);
    a.l[6] = (uint16_t)((uint32_t)a.l[6] + (uint32_t)s);
    b.l[2] = (uint16_t)((uint32_t)b.l[2] + (uint32_t)s);
    b.l[7] = (uint16_t)((uint32_t)b.l[7] + (uint32_t)s);

    h = fold_vec(h, a);
    h = fold_vec(h, b);
    h = fold_vec(h, vadd(a, b));
    h = fold_vec(h, vsub(a, b));
    h = fold_vec(h, vsub(b, a));
    h = fold_vec(h, vmul(a, b));
    h = fold_vec(h, vmul(a, ramp));
    h = fold_vec(h, vmul(b, ramp));
    h = fold_vec(h, vmul(vadd(a, b), ramp));
    h = fold_vec(h, vsub(zero, a));
    h = fold_vec(h, vsub(zero, b));
    h = fold_vec(h, vand(a, b));
    h = fold_vec(h, vor(a, b));
    h = fold_vec(h, vxor(a, b));
    h = fold_vec(h, vnot(a));
    h = fold_vec(h, vnot(b));
    h = fold_vec(h, vxor(vand(a, b), vor(a, b)));

    i16x8 acc = zero;
    i16x8 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const i16x8 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    i16x8 arr[4];
    arr[0] = a;
    arr[1] = vadd(a, b);
    arr[2] = vmul(a, ramp);
    arr[3] = acc;
    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_vec(h, arr[k]);
    }

    Pair r;
    r.lead = UINT32_C(4294967295);
    r.v = arr[2];
    r.trail = UINT32_C(305419896);
    h = mix_u32(h, r.lead);
    h = fold_vec(h, r.v);
    h = mix_u32(h, r.trail);
    return h;
}
