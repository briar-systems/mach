#include "corpus.h"

/* mach's u64x2 is 2 lanes x 8 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment. every
 * lane operation is unsigned, so every wrap here is defined. */
typedef struct { _Alignas(16) uint64_t l[2]; } u64x2;

typedef struct { uint32_t lead; u64x2 v; uint32_t trail; } Pair;

static u64x2 vadd(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] + y.l[i]); }
    return r;
}

static u64x2 vsub(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] - y.l[i]); }
    return r;
}

static u64x2 vmul(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] * y.l[i]); }
    return r;
}

static u64x2 vand(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] & y.l[i]); }
    return r;
}

static u64x2 vor(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] | y.l[i]); }
    return r;
}

static u64x2 vxor(u64x2 x, u64x2 y) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(x.l[i] ^ y.l[i]); }
    return r;
}

static u64x2 vnot(u64x2 x) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(~x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, u64x2 v) {
    h = mix_u64(h, v.l[0]);
    h = mix_u64(h, v.l[1]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    u64x2 a = { { UINT64_C(18446744073709551600), UINT64_C(305419896) } };
    u64x2 b = { { UINT64_C(17), UINT64_C(18446744073709551615) } };
    const u64x2 ramp = { { UINT64_C(1), UINT64_C(27) } };
    const u64x2 zero = { { 0, 0 } };
    a.l[0] = (uint64_t)(a.l[0] + seed);
    b.l[1] = (uint64_t)(b.l[1] + seed);

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

    u64x2 acc = zero;
    u64x2 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const u64x2 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    u64x2 arr[4];
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
