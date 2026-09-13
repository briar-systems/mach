#include "corpus.h"

/* lanes are held as uint64_t; every lane operation widens to uint64_t before it
 * computes and truncates on the way back, so no intermediate overflows a
 * signed type. signed lanes reach the fold through the two's-complement
 * identity, and a signed compare or divide reads the lane through memcpy. */
typedef struct { _Alignas(16) uint64_t l[2]; } vec;

static int64_t sx(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }
static uint64_t ux(int64_t v) { uint64_t r; memcpy(&r, &v, sizeof r); return r; }

static vec vadd(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] + (uint64_t)y.l[i]); } return r; }
static vec vsub(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] - (uint64_t)y.l[i]); } return r; }
static vec vmul(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] * (uint64_t)y.l[i]); } return r; }
static vec vand(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] & (uint64_t)y.l[i]); } return r; }
static vec vor(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] | (uint64_t)y.l[i]); } return r; }
static vec vxor(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] ^ (uint64_t)y.l[i]); } return r; }
static vec vnot(vec x) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)(~(uint64_t)x.l[i]); } return r; }
static vec vdiv_u(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((uint64_t)x.l[i] / (uint64_t)y.l[i]); } return r; }
static vec vdiv_s(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)ux((int64_t)((int64_t)sx(x.l[i]) / (int64_t)sx(y.l[i]))); } return r; }
static vec vcmp_u_lt(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (x.l[i] < y.l[i]) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }
static vec vcmp_s_lt(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (sx(x.l[i]) < sx(y.l[i])) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }
static vec vcmp_u_eq(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (x.l[i] == y.l[i]) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }
static vec vcmp_s_eq(vec x, vec y) { vec r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = (sx(x.l[i]) == sx(y.l[i])) ? (uint64_t)UINT64_C(18446744073709551615) : (uint64_t)0; } return r; }

static uint64_t fold_u(uint64_t h, vec v) { for (unsigned i = 0; i < 2u; i++) { h = mix_u64(h, v.l[i]); } return h; }
static uint64_t fold_s(uint64_t h, vec v) { for (unsigned i = 0; i < 2u; i++) { h = mix_i64(h, sx(v.l[i])); } return h; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = (uint64_t)seed;
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vadd(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vadd(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vsub(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vsub(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vmul(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vmul(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vdiv_s(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vdiv_u(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vand(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vand(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vor(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vor(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vxor(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vxor(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vnot(a));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vnot(a));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_s_lt(a, b));
    }
    {
        vec a = {{ UINT64_C(9223372036854775807), UINT64_C(9223372036854775808) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_s_eq(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_u_lt(a, b));
    }
    {
        vec a = {{ UINT64_C(18446744073709551615), UINT64_C(0) }};
        vec b = {{ UINT64_C(3), UINT64_C(2) }};
        a.l[0] = (uint64_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_u_eq(a, b));
    }
    return h;
}
