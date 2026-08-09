#include "corpus.h"

/* mach's u32x4 is 4 lanes x 4 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment. every
 * lane operation is unsigned, so every wrap here is defined. */
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;

typedef struct { uint32_t lead; u32x4 v; uint32_t trail; } Pair;

static u32x4 vadd(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] + y.l[i]); }
    return r;
}

static u32x4 vsub(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] - y.l[i]); }
    return r;
}

static u32x4 vmul(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] * y.l[i]); }
    return r;
}

static u32x4 vand(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] & y.l[i]); }
    return r;
}

static u32x4 vor(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] | y.l[i]); }
    return r;
}

static u32x4 vxor(u32x4 x, u32x4 y) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] ^ y.l[i]); }
    return r;
}

static u32x4 vnot(u32x4 x) {
    u32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(~x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, u32x4 v) {
    h = mix_u32(h, v.l[0]);
    h = mix_u32(h, v.l[1]);
    h = mix_u32(h, v.l[2]);
    h = mix_u32(h, v.l[3]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    u32x4 a = { { UINT32_C(4294967280), UINT32_C(1), UINT32_C(2147483648), UINT32_C(305419896) } };
    u32x4 b = { { UINT32_C(17), UINT32_C(4294967295), UINT32_C(2147483649), UINT32_C(1) } };
    const u32x4 ramp = { { UINT32_C(1), UINT32_C(3), UINT32_C(9), UINT32_C(27) } };
    const u32x4 zero = { { 0, 0, 0, 0 } };
    a.l[0] = (uint32_t)(a.l[0] + s);
    a.l[2] = (uint32_t)(a.l[2] + s);
    b.l[1] = (uint32_t)(b.l[1] + s);
    b.l[3] = (uint32_t)(b.l[3] + s);

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

    u32x4 acc = zero;
    u32x4 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const u32x4 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    u32x4 arr[4];
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
