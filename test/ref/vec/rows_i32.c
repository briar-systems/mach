#include "corpus.h"

/* lanes are held as uint32_t; every lane operation widens to uint64_t before it
 * computes and truncates on the way back, so no intermediate overflows a
 * signed type. signed lanes reach the fold through the two's-complement
 * identity, and a signed compare or divide reads the lane through memcpy. */
typedef struct { _Alignas(16) uint32_t l[4]; } vec;

static int32_t sx(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static uint32_t ux(int32_t v) { uint32_t r; memcpy(&r, &v, sizeof r); return r; }

static vec vadd(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] + (uint64_t)y.l[i]); } return r; }
static vec vsub(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] - (uint64_t)y.l[i]); } return r; }
static vec vmul(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] * (uint64_t)y.l[i]); } return r; }
static vec vand(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] & (uint64_t)y.l[i]); } return r; }
static vec vor(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] | (uint64_t)y.l[i]); } return r; }
static vec vxor(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] ^ (uint64_t)y.l[i]); } return r; }
static vec vnot(vec x) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)(~(uint64_t)x.l[i]); } return r; }
static vec vdiv_u(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)((uint64_t)x.l[i] / (uint64_t)y.l[i]); } return r; }
static vec vdiv_s(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (uint32_t)ux((int32_t)((int64_t)sx(x.l[i]) / (int64_t)sx(y.l[i]))); } return r; }
static vec vcmp_u_lt(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (x.l[i] < y.l[i]) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }
static vec vcmp_s_lt(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (sx(x.l[i]) < sx(y.l[i])) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }
static vec vcmp_u_eq(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (x.l[i] == y.l[i]) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }
static vec vcmp_s_eq(vec x, vec y) { vec r; for (unsigned i = 0; i < 4u; i++) { r.l[i] = (sx(x.l[i]) == sx(y.l[i])) ? (uint32_t)UINT64_C(4294967295) : (uint32_t)0; } return r; }

static uint64_t fold_u(uint64_t h, vec v) { for (unsigned i = 0; i < 4u; i++) { h = mix_u32(h, v.l[i]); } return h; }
static uint64_t fold_s(uint64_t h, vec v) { for (unsigned i = 0; i < 4u; i++) { h = mix_i32(h, sx(v.l[i])); } return h; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vadd(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vadd(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vsub(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vsub(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vmul(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vmul(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vdiv_s(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vdiv_u(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vand(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vand(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vor(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vor(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vxor(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vxor(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_s(h, vnot(a));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vnot(a));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_s_lt(a, b));
    }
    {
        vec a = {{ UINT64_C(2147483647), UINT64_C(2147483648), UINT64_C(7), UINT64_C(4294967293) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(4294967289), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_s_eq(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_u_lt(a, b));
    }
    {
        vec a = {{ UINT64_C(4294967295), UINT64_C(0), UINT64_C(7), UINT64_C(3) }};
        vec b = {{ UINT64_C(3), UINT64_C(2), UINT64_C(7), UINT64_C(5) }};
        a.l[0] = (uint32_t)((uint64_t)a.l[0] + (uint64_t)s);
        h = fold_u(h, vcmp_u_eq(a, b));
    }
    return h;
}
