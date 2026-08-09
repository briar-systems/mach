#include "corpus.h"

/* mach's i16x4 is 4 lanes x 2 bytes, size 8, and it is narrower than the vector
 * register so $align_of is a single lane, 2. a plain C struct of four uint16_t has
 * exactly that size and alignment, so an array of it strides by 8 and Pair places
 * the vector at offset 2 the same way.
 *
 * lanes are held as uint16_t and every lane operation widens to uint32_t before it
 * computes, so nothing here can overflow a signed type. sx16 is the same
 * two's-complement sign extension mix_i16 performs. */
typedef struct { uint16_t l[4]; } i16x4;

typedef struct { uint8_t lead; i16x4 v; uint8_t trail; } Pair;

static uint64_t sx16(uint16_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x8000)) - UINT64_C(0x8000));
}

static i16x4 vadd(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] + (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vsub(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] - (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vmul(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] * (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vand(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] & (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vor(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] | (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vxor(i16x4 x, i16x4 y) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] ^ (uint32_t)y.l[i]); }
    return r;
}

static i16x4 vnot(i16x4 x) {
    i16x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint16_t)(~(uint32_t)x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, i16x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_u64(h, sx16(v.l[i])); }
    return h;
}

#define N16(x) ((uint16_t)-(uint32_t)(x))

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    i16x4 a = { { 1001, N16(1103), 1207, N16(1301) } };
    i16x4 b = { { 13, N16(11), 9, N16(7) } };
    const i16x4 ramp = { { 1, 2, 3, 4 } };
    const i16x4 zero = { { 0, 0, 0, 0 } };
    a.l[0] = (uint16_t)((uint32_t)a.l[0] + (uint32_t)s);
    a.l[3] = (uint16_t)((uint32_t)a.l[3] + (uint32_t)s);
    b.l[1] = (uint16_t)((uint32_t)b.l[1] + (uint32_t)s);
    b.l[2] = (uint16_t)((uint32_t)b.l[2] + (uint32_t)s);

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

    i16x4 acc = zero;
    i16x4 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const i16x4 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    i16x4 arr[6];
    arr[0] = a;
    arr[1] = vadd(a, b);
    arr[2] = vmul(a, ramp);
    arr[3] = acc;
    arr[4] = sum;
    arr[5] = vxor(a, b);
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_vec(h, arr[k]);
    }

    Pair r;
    r.lead = 219;
    r.v = arr[2];
    r.trail = 173;
    h = mix_u8(h, r.lead);
    h = fold_vec(h, r.v);
    h = mix_u8(h, r.trail);
    return h;
}
