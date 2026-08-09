#include "corpus.h"

/* mach's i8x16 is 16 lanes x 1 byte, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment.
 *
 * lanes are held as uint8_t and every lane operation widens to uint32_t before it
 * computes, so nothing here can overflow a signed type. the signed value reaches
 * the fold through the two's-complement sign-extension identity in sx8, which is
 * the same u64 mix_i8 produces. */
typedef struct { _Alignas(16) uint8_t l[16]; } i8x16;

typedef struct { uint32_t lead; i8x16 v; uint32_t trail; } Pair;

static uint64_t sx8(uint8_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x80)) - UINT64_C(0x80));
}

static i8x16 vadd(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] + (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vsub(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] - (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vmul(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] * (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vand(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] & (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vor(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] | (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vxor(i8x16 x, i8x16 y) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] ^ (uint32_t)y.l[i]); }
    return r;
}

static i8x16 vnot(i8x16 x) {
    i8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)(~(uint32_t)x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, i8x16 v) {
    for (unsigned i = 0; i < 16u; i++) { h = mix_u64(h, sx8(v.l[i])); }
    return h;
}

#define N8(x) ((uint8_t)-(uint32_t)(x))

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    i8x16 a = { { N8(9), 7, N8(5), 3, N8(1), 2, N8(4), 6, N8(8), 9, N8(3), 1, N8(2), 4, N8(6), 8 } };
    i8x16 b = { { 2, N8(3), 4, N8(5), 6, N8(7), 8, N8(9), 1, N8(2), 3, N8(4), 5, N8(6), 7, N8(1) } };
    const i8x16 ramp = { { 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2, 1, 2, 3, 2 } };
    const i8x16 zero = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
    a.l[0] = (uint8_t)((uint32_t)a.l[0] + (uint32_t)s);
    a.l[7] = (uint8_t)((uint32_t)a.l[7] + (uint32_t)s);
    b.l[3] = (uint8_t)((uint32_t)b.l[3] + (uint32_t)s);
    b.l[12] = (uint8_t)((uint32_t)b.l[12] + (uint32_t)s);

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

    i8x16 acc = zero;
    i8x16 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(3); i = (uint64_t)(i + UINT64_C(1))) {
        const i8x16 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    i8x16 arr[4];
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
