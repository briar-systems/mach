#include "corpus.h"

/* mach's i32x4 is 4 lanes x 4 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment.
 *
 * lanes are held as uint32_t and every operation is unsigned, so nothing here can
 * overflow a signed type. the signed value reaches the fold through the
 * two's-complement sign-extension identity in sx32, which is the same u64 mix_i32
 * produces. */
typedef struct { _Alignas(16) uint32_t l[4]; } i32x4;

typedef struct { uint32_t lead; i32x4 v; uint32_t trail; } Pair;

static uint64_t sx32(uint32_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x80000000)) - UINT64_C(0x80000000));
}

static i32x4 vadd(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] + y.l[i]); }
    return r;
}

static i32x4 vsub(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] - y.l[i]); }
    return r;
}

static i32x4 vmul(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] * y.l[i]); }
    return r;
}

static i32x4 vand(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] & y.l[i]); }
    return r;
}

static i32x4 vor(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] | y.l[i]); }
    return r;
}

static i32x4 vxor(i32x4 x, i32x4 y) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(x.l[i] ^ y.l[i]); }
    return r;
}

static i32x4 vnot(i32x4 x) {
    i32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(~x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, i32x4 v) {
    h = mix_u64(h, sx32(v.l[0]));
    h = mix_u64(h, sx32(v.l[1]));
    h = mix_u64(h, sx32(v.l[2]));
    h = mix_u64(h, sx32(v.l[3]));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    i32x4 a = { { UINT32_C(10007), (uint32_t)-UINT32_C(20011), UINT32_C(30013), (uint32_t)-UINT32_C(32768) } };
    i32x4 b = { { (uint32_t)-UINT32_C(7), UINT32_C(131), (uint32_t)-UINT32_C(1009), UINT32_C(3) } };
    const i32x4 ramp = { { UINT32_C(1), UINT32_C(3), UINT32_C(9), UINT32_C(27) } };
    const i32x4 zero = { { 0, 0, 0, 0 } };
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

    i32x4 acc = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const i32x4 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        acc = vxor(acc, b);
        h = fold_vec(h, acc);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    i32x4 arr[4];
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
