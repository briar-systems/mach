#include "corpus.h"

/* mach's f32x2 is 2 lanes x 4 bytes, size 8, and it is narrower than the vector
 * register so $align_of is a single lane, 4. a plain C struct of two floats has
 * exactly that size and alignment, so an array of it strides by 8 and Pair places
 * the vector at offset 4 the same way. */
typedef struct { float l[2]; } f32x2;

typedef struct { uint32_t lead; f32x2 v; uint32_t trail; } Pair;

static f32x2 vadd(f32x2 x, f32x2 y) {
    f32x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x2 vsub(f32x2 x, f32x2 y) {
    f32x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f32x2 vmul(f32x2 x, f32x2 y) {
    f32x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x2 vdiv(f32x2 x, f32x2 y) {
    f32x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f32x2 v) {
    h = mix_f32(h, v.l[0]);
    h = mix_f32(h, v.l[1]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x2 a = { { 1.5f, -2.25f } };
    f32x2 b = { { 0.5f, 4.0f } };
    const f32x2 ramp = { { 1.0f, 27.0f } };
    a.l[0] = a.l[0] + z;
    b.l[1] = b.l[1] + z;

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

    f32x2 acc = { { 0.0f, 0.0f } };
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const f32x2 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        a = vsub(a, b);
        h = fold_vec(h, a);
    }

    f32x2 arr[6];
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
