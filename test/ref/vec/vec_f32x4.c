#include "corpus.h"

/* mach's f32x4 is 4 lanes x 4 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment, so an
 * array of it strides identically and the reference is layout-faithful. */
typedef struct { _Alignas(16) float l[4]; } f32x4;

typedef struct { uint32_t lead; f32x4 v; uint32_t trail; } Pair;

static f32x4 vadd(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x4 vsub(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f32x4 vmul(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x4 vdiv(f32x4 x, f32x4 y) {
    f32x4 r;
    for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f32x4 v) {
    h = mix_f32(h, v.l[0]);
    h = mix_f32(h, v.l[1]);
    h = mix_f32(h, v.l[2]);
    h = mix_f32(h, v.l[3]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x4 a = { { 1.5f, -2.25f, 3.75f, -0.5f } };
    f32x4 b = { { 0.5f, 4.0f, -0.125f, 8.0f } };
    const f32x4 ramp = { { 1.0f, 3.0f, 9.0f, 27.0f } };
    a.l[0] = a.l[0] + z;
    a.l[3] = a.l[3] + z;
    b.l[1] = b.l[1] + z;
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

    f32x4 acc = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const f32x4 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        a = vsub(a, b);
        h = fold_vec(h, a);
    }

    f32x4 arr[4];
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
