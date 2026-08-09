#include "corpus.h"

/* mach's f32x8 is 8 lanes x 4 bytes, size 32, with $align_of 16: the register
 * width, not the whole shape, because there is no 32-byte vector load for a larger
 * alignment to serve. _Alignas(16) on the C member gives the same size and the same
 * alignment, so an array of it strides by 32 the same way. */
typedef struct { _Alignas(16) float l[8]; } f32x8;

typedef struct { uint32_t lead; f32x8 v; uint32_t trail; } Pair;

static f32x8 vadd(f32x8 x, f32x8 y) {
    f32x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x8 vsub(f32x8 x, f32x8 y) {
    f32x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f32x8 vmul(f32x8 x, f32x8 y) {
    f32x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x8 vdiv(f32x8 x, f32x8 y) {
    f32x8 r;
    for (unsigned i = 0; i < 8u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f32x8 v) {
    for (unsigned i = 0; i < 8u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x8 a = { { 1.5f, -2.25f, 3.75f, -0.5f, 6.25f, -8.125f, 0.75f, -16.0f } };
    f32x8 b = { { 0.5f, 4.0f, -0.125f, 8.0f, -2.0f, 1.25f, -32.0f, 0.0625f } };
    const f32x8 ramp = { { 1.0f, 3.0f, 9.0f, 27.0f, 81.0f, 243.0f, 729.0f, 2187.0f } };
    a.l[0] = a.l[0] + z;
    a.l[7] = a.l[7] + z;
    b.l[3] = b.l[3] + z;
    b.l[4] = b.l[4] + z;

    h = fold_vec(h, a);
    h = fold_vec(h, b);
    h = fold_vec(h, vadd(a, b));
    h = fold_vec(h, vsub(a, b));
    h = fold_vec(h, vsub(b, a));
    h = fold_vec(h, vmul(a, b));
    h = fold_vec(h, vdiv(a, b));
    h = fold_vec(h, vdiv(b, a));
    h = fold_vec(h, vmul(a, ramp));
    h = fold_vec(h, vsub(ramp, b));
    h = fold_vec(h, vdiv(ramp, a));

    f32x8 acc = { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } };
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const f32x8 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        a = vsub(a, b);
        h = fold_vec(h, a);
    }

    f32x8 arr[4];
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
