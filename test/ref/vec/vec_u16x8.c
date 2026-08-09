#include "corpus.h"

/* mach's u16x8 is 8 lanes x 2 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment.
 *
 * every lane operation widens to uint32_t before it computes and casts back. that
 * is load-bearing for the multiply: a bare uint16_t product would first promote to
 * signed int, where 65535 * 65535 overflows. */
typedef struct { _Alignas(16) uint16_t l[8]; } u16x8;

typedef struct { uint32_t lead; u16x8 v; uint32_t trail; } Pair;

static u16x8 vadd(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] + (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vsub(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] - (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vmul(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] * (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vand(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] & (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vor(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] | (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vxor(u16x8 x, u16x8 y) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)((uint32_t)x.l[i] ^ (uint32_t)y.l[i]); }
    return r;
}

static u16x8 vnot(u16x8 x) {
    u16x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = (uint16_t)(~(uint32_t)x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, u16x8 v) {
    for (unsigned i = 0; i < 8u; i++) { h = mix_u16(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    u16x8 a = { { 65520, 1, 32768, 65535, 4097, 2, 40000, 12345 } };
    u16x8 b = { { 17, 65535, 32769, 1, 60000, 30000, 25537, 54321 } };
    const u16x8 ramp = { { 1, 3, 9, 27, 81, 243, 729, 2187 } };
    const u16x8 zero = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
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

    u16x8 acc = zero;
    u16x8 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const u16x8 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    u16x8 arr[4];
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
