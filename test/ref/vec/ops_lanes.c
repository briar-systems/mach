#include "corpus.h"

/* the lanes as written and as each operation leaves them. */

/* a vector is its lane count, lane width and kind, with integer lanes held as
 * their bit pattern in a uint64_t and float lanes as the value in a double,
 * rounded to float after every operation on a 32-bit lane. every integer
 * operation is unsigned arithmetic on the pattern, signed lanes read through
 * the two's-complement identity, so nothing here overflows a signed type. */
typedef struct { unsigned n, w, sgn, fl; uint64_t l[16]; double d[16]; } V;

static uint64_t mask_w(unsigned w) { return w >= 64u ? ~UINT64_C(0) : (uint64_t)((UINT64_C(1) << w) - 1u); }
static uint64_t tr(uint64_t v, unsigned w) { return v & mask_w(w); }
static int64_t sx(uint64_t v, unsigned w) {
    const uint64_t m = UINT64_C(1) << (w - 1u);
    const uint64_t t = (uint64_t)((tr(v, w) ^ m) - m);
    int64_t r;
    memcpy(&r, &t, sizeof r);
    return r;
}
static uint64_t ux(int64_t v) { uint64_t r; memcpy(&r, &v, sizeof r); return r; }
static double rd(double x, unsigned w) { return w == 32u ? (double)(float)x : x; }

static V vi(unsigned n, unsigned w, unsigned sgn, const uint64_t *l) {
    V r;
    memset(&r, 0, sizeof r);
    r.n = n; r.w = w; r.sgn = sgn; r.fl = 0;
    for (unsigned i = 0; i < n; i++) { r.l[i] = tr(l[i], w); }
    return r;
}
static V vf(unsigned n, unsigned w, const double *d) {
    V r;
    memset(&r, 0, sizeof r);
    r.n = n; r.w = w; r.sgn = 1; r.fl = 1;
    for (unsigned i = 0; i < n; i++) { r.d[i] = rd(d[i], w); }
    return r;
}
static void seed_lane(V *v, uint64_t seed) {
    if (v->fl) { v->d[0] = v->w == 32u ? (double)((float)v->d[0] + (float)seed) : v->d[0] + (double)seed; }
    else { v->l[0] = tr(v->l[0] ^ seed, v->w); }
}
static V like(const V *a) { V r = *a; return r; }

static V add(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { if (a.fl) { r.d[i] = a.w == 32u ? (double)((float)a.d[i] + (float)b.d[i]) : a.d[i] + b.d[i]; } else { r.l[i] = tr(a.l[i] + b.l[i], a.w); } } return r; }
static V sub(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { if (a.fl) { r.d[i] = a.w == 32u ? (double)((float)a.d[i] - (float)b.d[i]) : a.d[i] - b.d[i]; } else { r.l[i] = tr(a.l[i] - b.l[i], a.w); } } return r; }
static V mul(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { if (a.fl) { r.d[i] = a.w == 32u ? (double)((float)a.d[i] * (float)b.d[i]) : a.d[i] * b.d[i]; } else { r.l[i] = tr(a.l[i] * b.l[i], a.w); } } return r; }
static V dv(V a, V b) {
    V r = like(&a);
    for (unsigned i = 0; i < a.n; i++) {
        if (a.fl) { r.d[i] = a.w == 32u ? (double)((float)a.d[i] / (float)b.d[i]) : a.d[i] / b.d[i]; }
        else if (a.sgn) { r.l[i] = tr(ux(sx(a.l[i], a.w) / sx(b.l[i], a.w)), a.w); }
        else { r.l[i] = a.l[i] / b.l[i]; }
    }
    return r;
}
static V band(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = a.l[i] & b.l[i]; } return r; }
static V bor(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = a.l[i] | b.l[i]; } return r; }
static V bxor(V a, V b) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = a.l[i] ^ b.l[i]; } return r; }
static V bnot(V a) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = tr(~a.l[i], a.w); } return r; }

/* p: 0 lt, 1 le, 2 gt, 3 ge, 4 eq, 5 ne; the result is the unsigned mask */
static V cmp(V a, V b, int p) {
    V r = like(&a);
    r.sgn = 0; r.fl = 0;
    for (unsigned i = 0; i < a.n; i++) {
        int lt, eq;
        if (a.fl) { lt = a.d[i] < b.d[i]; eq = a.d[i] == b.d[i]; }
        else if (a.sgn) { lt = sx(a.l[i], a.w) < sx(b.l[i], a.w); eq = a.l[i] == b.l[i]; }
        else { lt = a.l[i] < b.l[i]; eq = a.l[i] == b.l[i]; }
        const int gt = !lt && !eq;
        int t = 0;
        switch (p) {
            case 0: t = lt; break;
            case 1: t = lt || eq; break;
            case 2: t = gt; break;
            case 3: t = gt || eq; break;
            case 4: t = eq; break;
            default: t = !eq; break;
        }
        r.l[i] = t ? mask_w(a.w) : 0;
    }
    return r;
}

/* a count at or past the lane width saturates; a signed count is read as its
 * unsigned pattern, so a negative count saturates too */
static uint64_t shl1(uint64_t x, uint64_t c, unsigned w) { return c >= w ? 0 : tr(x << c, w); }
static uint64_t shr1(uint64_t x, uint64_t c, unsigned w, unsigned sgn) {
    const int neg = sgn && ((x >> (w - 1u)) & 1u);
    if (c >= w) { return neg ? mask_w(w) : 0; }
    return neg ? tr(~(tr(~x, w) >> c), w) : (x >> c);
}
static V shl(V a, V c) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = shl1(a.l[i], c.l[i], a.w); } return r; }
static V shr(V a, V c) { V r = like(&a); for (unsigned i = 0; i < a.n; i++) { r.l[i] = shr1(a.l[i], c.l[i], a.w, a.sgn); } return r; }
static V splat(V like_v, uint64_t c) { V r = like(&like_v); for (unsigned i = 0; i < r.n; i++) { r.l[i] = tr(c, r.w); } return r; }

/* each lane extended to w bits by its own signedness, then reinterpreted as sgn */
static V widen(V a, unsigned w, unsigned sgn) {
    V r = like(&a);
    r.w = w; r.sgn = sgn;
    for (unsigned i = 0; i < a.n; i++) { r.l[i] = tr(a.sgn ? ux(sx(a.l[i], a.w)) : a.l[i], w); }
    return r;
}
static V part(V a, unsigned at, unsigned n) { V r = like(&a); r.n = n; for (unsigned i = 0; i < n; i++) { r.l[i] = a.l[at + i]; r.d[i] = a.d[at + i]; } return r; }

static uint64_t fold_v(uint64_t h, V v) {
    for (unsigned i = 0; i < v.n; i++) {
        if (v.fl && v.w == 32u) { h = mix_f32(h, (float)v.d[i]); }
        else if (v.fl) { h = mix_f64(h, v.d[i]); }
        else if (v.sgn) { h = mix_u64(h, ux(sx(v.l[i], v.w))); }
        else { h = mix_u64(h, v.l[i]); }
    }
    return h;
}

static V to_bits(V a) {
    V r = like(&a);
    r.fl = 0; r.sgn = 0;
    for (unsigned i = 0; i < a.n; i++) {
        if (a.w == 32u) { float f = (float)a.d[i]; uint32_t b; memcpy(&b, &f, sizeof b); r.l[i] = b; }
        else { double f = a.d[i]; uint64_t b; memcpy(&b, &f, sizeof b); r.l[i] = b; }
    }
    return r;
}
static V from_bits(V a) {
    V r = like(&a);
    r.fl = 1; r.sgn = 1;
    for (unsigned i = 0; i < a.n; i++) {
        if (a.w == 32u) { uint32_t b = (uint32_t)a.l[i]; float f; memcpy(&f, &b, sizeof f); r.d[i] = (double)f; }
        else { uint64_t b = a.l[i]; double f; memcpy(&f, &b, sizeof f); r.d[i] = f; }
    }
    return r;
}
static V seti(V a, unsigned i, uint64_t x) { a.l[i] = tr(x, a.w); return a; }
static V setf(V a, unsigned i, double x) { a.d[i] = rd(x, a.w); return a; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    V v1 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x2), UINT64_C(0x3), UINT64_C(0x4), UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x9), UINT64_C(0xA), UINT64_C(0xB), UINT64_C(0xC), UINT64_C(0xD), UINT64_C(0xE), UINT64_C(0xF)});
    seed_lane(&v1, seed);
    v1 = seti(v1, 0, UINT64_C(0x80));
    v1 = seti(v1, 15, UINT64_C(0x64));
    v1 = seti(v1, 7, UINT64_C(0xF9));
    h = fold_v(h, v1);
    V v2 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x2), UINT64_C(0x3), UINT64_C(0x4), UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x7)});
    seed_lane(&v2, seed);
    v2 = seti(v2, 0, UINT64_C(0x12C));
    v2 = seti(v2, 7, UINT64_C(0xFED4));
    h = fold_v(h, v2);
    V v3 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x2), UINT64_C(0x3)});
    seed_lane(&v3, seed);
    v3 = seti(v3, 1, UINT64_C(0x11170));
    v3 = seti(v3, 3, UINT64_C(0xFFFEEE90));
    h = fold_v(h, v3);
    V v4 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1)});
    seed_lane(&v4, seed);
    v4 = seti(v4, 0, UINT64_C(0x100000000));
    h = fold_v(h, v4);
    V v5 = vf(4, 32, (const double[]){1.0, 2.0, 3.0, 4.0});
    seed_lane(&v5, seed);
    v5 = setf(v5, 0, 1.5);
    v5 = setf(v5, 3, -4.5);
    h = fold_v(h, v5);
    V v6 = vf(2, 64, (const double[]){1.0, 2.0});
    seed_lane(&v6, seed);
    v6 = setf(v6, 1, 9.25);
    h = fold_v(h, v6);
    V v7 = vf(4, 32, (const double[]){1.0, 2.0, 3.0, 4.0});
    seed_lane(&v7, seed);
    V v8 = vf(4, 32, (const double[]){10.0, 20.0, 30.0, 40.0});
    seed_lane(&v8, seed);
    h = fold_v(h, add(v7, v8));
    V v9 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xFFFF), UINT64_C(0x1), UINT64_C(0x2), UINT64_C(0x3)});
    seed_lane(&v9, seed);
    V v10 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x1), UINT64_C(0x1), UINT64_C(0x1)});
    seed_lane(&v10, seed);
    V v11 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x7), UINT64_C(0x8)});
    seed_lane(&v11, seed);
    V v12 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x3E8), UINT64_C(0x7D0), UINT64_C(0xBB8), UINT64_C(0xFA0)});
    seed_lane(&v12, seed);
    h = fold_v(h, add(v9, v10));
    h = fold_v(h, v11);
    h = fold_v(h, v9);
    h = fold_v(h, add(add(add(v9, v10), v11), v12));
    V v13 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xFFFF), UINT64_C(0x1), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v13, seed);
    V v14 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x2), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v14, seed);
    V v15 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x2), UINT64_C(0x3), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v15, seed);
    V v16 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x3), UINT64_C(0x4), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v16, seed);
    V v17 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x4), UINT64_C(0x5), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v17, seed);
    V v18 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v18, seed);
    V v19 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x6), UINT64_C(0x7), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v19, seed);
    V v20 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v20, seed);
    V v21 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x8), UINT64_C(0x9), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v21, seed);
    V v22 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x9), UINT64_C(0xA), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v22, seed);
    V v23 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0xB), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v23, seed);
    V v24 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xB), UINT64_C(0xC), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v24, seed);
    V v25 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xC), UINT64_C(0xD), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v25, seed);
    V v26 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xD), UINT64_C(0xE), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v26, seed);
    V v27 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xE), UINT64_C(0xF), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v27, seed);
    V v28 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xF), UINT64_C(0x10), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v28, seed);
    h = fold_v(h, add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(v13, v14), v15), v16), v17), v18), v19), v20), v21), v22), v23), v24), v25), v26), v27), v28));
    h = fold_v(h, add(add(add(add(add(add(add(add(add(add(add(add(add(add(add(v28, v27), v26), v25), v24), v23), v22), v21), v20), v19), v18), v17), v16), v15), v14), v13));
    V v29 = vf(4, 32, (const double[]){1.0, -2.0, 3.5, -8.0});
    seed_lane(&v29, seed);
    h = fold_v(h, to_bits(v29));
    h = fold_v(h, from_bits(to_bits(v29)));
    V v30 = vf(2, 64, (const double[]){1.5, -0.5});
    seed_lane(&v30, seed);
    h = fold_v(h, to_bits(v30));
    h = fold_v(h, from_bits(to_bits(v30)));
    V v31 = vf(4, 32, (const double[]){1.0, -2.0, 3.0, -4.0});
    seed_lane(&v31, seed);
    V v32 = vf(4, 32, (const double[]){10.0, 20.0, -30.0, 40.0});
    seed_lane(&v32, seed);
    V v33 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0xFFFFFFFF), UINT64_C(0x0), UINT64_C(0xFFFFFFFF), UINT64_C(0x0)});
    h = fold_v(h, from_bits(bor(band(to_bits(v31), v33), band(to_bits(v32), bnot(v33)))));
    V v34 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xA0B0C0C), UINT64_C(0x11223343), UINT64_C(0x55667787), UINT64_C(0x7FEEDDCB)});
    seed_lane(&v34, seed);
    V v35 = vf(4, 32, (const double[]){1.0, 2.0, 4.0, 8.0});
    seed_lane(&v35, seed);
    h = fold_v(h, add(v34, vi(4, 32, 1, (const uint64_t[]){1, 1, 1, 1})));
    return h;
}
