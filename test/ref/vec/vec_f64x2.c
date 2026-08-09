#include "corpus.h"

/* mach's f64x2 is 2 lanes x 8 bytes, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment. */
typedef struct { _Alignas(16) double l[2]; } f64x2;

typedef struct { uint32_t lead; f64x2 v; uint32_t trail; } Pair;

static f64x2 vadd(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f64x2 vsub(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] - y.l[i]; }
    return r;
}

static f64x2 vmul(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f64x2 vdiv(f64x2 x, f64x2 y) {
    f64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] / y.l[i]; }
    return r;
}

static uint64_t fold_vec(uint64_t h, f64x2 v) {
    h = mix_f64(h, v.l[0]);
    h = mix_f64(h, v.l[1]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const double z = (double)seed;

    f64x2 a = { { 1.5, -2.25 } };
    f64x2 b = { { 0.5, 4.0 } };
    const f64x2 ramp = { { 1.0, 27.0 } };
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

    f64x2 acc = { { 0.0, 0.0 } };
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const f64x2 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vadd(acc, t);
        h = fold_vec(h, acc);
        a = vsub(a, b);
        h = fold_vec(h, a);
    }

    f64x2 arr[4];
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
