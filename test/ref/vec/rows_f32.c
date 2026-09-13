#include "corpus.h"

/* lanes are held as uint32_t; every lane operation widens to uint64_t before it
 * computes and truncates on the way back, so no intermediate overflows a
 * signed type. signed lanes reach the fold through the two's-complement
 * identity, and a signed compare or divide reads the lane through memcpy. */
typedef struct { _Alignas(16) float l[4]; } vec;
typedef struct { _Alignas(16) uint32_t l[4]; } mask;

static vec vadd(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] + y.l[i]; } return r; }
static vec vsub(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] - y.l[i]; } return r; }
static vec vmul(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] * y.l[i]; } return r; }
static vec vdiv(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = x.l[i] / y.l[i]; } return r; }
static mask vcmp_lt(vec x, vec y) { mask r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (x.l[i] < y.l[i]) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }
static mask vcmp_eq(vec x, vec y) { mask r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (x.l[i] == y.l[i]) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }

static uint64_t fold_vec(uint64_t h, vec v) { for (unsigned i = 0; i < 4u; i++) { h = mix_f32(h, v.l[i]); } return h; }
static uint64_t fold_mask(uint64_t h, mask v) { for (unsigned i = 0; i < 4u; i++) { h = mix_u32(h, v.l[i]); } return h; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float s = (float)seed;
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vadd(a, b));
    }
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vsub(a, b));
    }
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vmul(a, b));
    }
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_vec(h, vdiv(a, b));
    }
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_mask(h, vcmp_lt(a, b));
    }
    {
        vec a = {{ 1.5f, -2.25f, 1024.0f, 0.125f }};
        vec b = {{ 0.5f, 4.0f, -2.0f, 8.0f }};
        a.l[0] = a.l[0] + s;
        h = fold_mask(h, vcmp_eq(a, b));
    }
    return h;
}
