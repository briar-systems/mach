#include "corpus.h"

/* lanes are held as uint64_t; every lane operation widens to uint64_t before it
 * computes and truncates on the way back, so no intermediate overflows a
 * signed type. signed lanes reach the fold through the two's-complement
 * identity, and a signed compare or divide reads the lane through memcpy. */
typedef struct { _Alignas(16) double l[2]; } vec;
typedef struct { _Alignas(16) uint64_t l[2]; } mask;

static vec vadd(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] + y.l[i]; } return r; }
static vec vsub(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] - y.l[i]; } return r; }
static vec vmul(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] * y.l[i]; } return r; }
static vec vdiv(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = x.l[i] / y.l[i]; } return r; }
static mask vcmp_lt(vec x, vec y) { mask r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (x.l[i] < y.l[i]) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }
static mask vcmp_eq(vec x, vec y) { mask r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (x.l[i] == y.l[i]) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }

static uint64_t fold_vec(uint64_t h, vec v) { for (unsigned i = 0; i < 2u; i++) { h = mix_f64(h, v.l[i]); } return h; }
static uint64_t fold_mask(uint64_t h, mask v) { for (unsigned i = 0; i < 2u; i++) { h = mix_u64(h, v.l[i]); } return h; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const double s = (double)seed;
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vadd(a, b));
    }
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vsub(a, b));
    }
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vmul(a, b));
    }
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vdiv(a, b));
    }
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_mask(h, vcmp_lt(a, b));
    }
    {
        vec a = {{ 1.5, -2.25 }};
        vec b = {{ 0.5, 4.0 }};
        a.l[0] = a.l[0] + s;
        h = fold_mask(h, vcmp_eq(a, b));
    }
    return h;
}
