#include "corpus.h"

/* mach's f32x3 is lane-derived: 3 lanes x 4 bytes packed, size 12, align 4. a C
 * struct of three floats has exactly that size and alignment, so an array of it
 * strides by 12 the same way and the reference is layout-faithful, not just
 * value-faithful. */
typedef struct { float l[3]; } f32x3;

typedef struct { uint32_t lead; f32x3 v; uint32_t trail; } Pair;

static f32x3 vadd(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x3 vsub(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f32x3 vmul(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x3 vdiv(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f32x3 v) {
    h = mix_f32(h, v.l[0]);
    h = mix_f32(h, v.l[1]);
    h = mix_f32(h, v.l[2]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x3 a = { { 1.5f, -2.25f, 3.75f } };
    f32x3 b = { { 0.5f, 4.0f, -0.125f } };
    a.l[0] = a.l[0] + z;
    b.l[2] = b.l[2] + z;

    h = fold_vec(h, a);
    h = fold_vec(h, b);
    h = fold_vec(h, vadd(a, b));
    h = fold_vec(h, vsub(a, b));
    h = fold_vec(h, vsub(b, a));
    h = fold_vec(h, vmul(a, b));
    h = fold_vec(h, vdiv(a, b));

    f32x3 arr[4];
    arr[0] = a;
    arr[1] = vadd(a, b);
    arr[2] = vmul(a, b);
    arr[3] = b;
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
