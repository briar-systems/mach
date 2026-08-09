#include "corpus.h"

/* mach's f32x5 is 5 lanes x 4 bytes, size 20, with $align_of 16 because the shape
 * is placed as several register-width pieces. size is not a multiple of alignment,
 * so no C struct reproduces both numbers at once. the reference takes the size: a
 * plain struct of five floats is 20 bytes and strides an array by 20, which is what
 * the lane-derived size says. the case observes only lanes it wrote back through
 * indices, so a mach stride that rounded up would still have to be self-consistent
 * to agree here. */
typedef struct { float l[5]; } f32x5;

typedef struct { uint32_t lead; f32x5 v; uint32_t trail; } Pair;

static f32x5 vadd(f32x5 x, f32x5 y) {
    f32x5 r;
    for (unsigned i = 0; i < 5u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x5 vsub(f32x5 x, f32x5 y) {
    f32x5 r;
    for (unsigned i = 0; i < 5u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f32x5 vmul(f32x5 x, f32x5 y) {
    f32x5 r;
    for (unsigned i = 0; i < 5u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x5 vdiv(f32x5 x, f32x5 y) {
    f32x5 r;
    for (unsigned i = 0; i < 5u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f32x5 v) {
    for (unsigned i = 0; i < 5u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x5 a = { { 1.5f, -2.25f, 3.75f, -0.5f, 6.25f } };
    f32x5 b = { { 0.5f, 4.0f, -0.125f, 8.0f, -2.0f } };
    const f32x5 ramp = { { 1.0f, 3.0f, 9.0f, 27.0f, 81.0f } };
    a.l[0] = a.l[0] + z;
    a.l[4] = a.l[4] + z;
    b.l[2] = b.l[2] + z;

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

    f32x5 acc = { { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } };
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const f32x5 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        a = vsub(a, b);
        h = fold_vec(h, a);
    }

    f32x5 arr[6];
    arr[0] = a;
    arr[1] = vadd(a, b);
    arr[2] = vmul(a, ramp);
    arr[3] = acc;
    arr[4] = vsub(b, a);
    arr[5] = b;
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
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
