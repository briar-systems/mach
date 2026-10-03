#include "corpus.h"

/* each predicate on the lanes as written, in the order the case folds them. */

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

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    V v1 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x10A), UINT64_C(0xFFFFFFFF), UINT64_C(0x5), UINT64_C(0x5)});
    seed_lane(&v1, seed);
    V v2 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x6)});
    seed_lane(&v2, seed);
    h = fold_v(h, cmp(v1, v2, 0));
    h = fold_v(h, cmp(v1, v2, 1));
    h = fold_v(h, cmp(v1, v2, 2));
    h = fold_v(h, cmp(v1, v2, 3));
    h = fold_v(h, cmp(v1, v2, 4));
    h = fold_v(h, cmp(v1, v2, 5));
    V v3 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0x10A), UINT64_C(0xFFFFFFFF), UINT64_C(0x5), UINT64_C(0x5)});
    seed_lane(&v3, seed);
    V v4 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x6)});
    seed_lane(&v4, seed);
    h = fold_v(h, cmp(v3, v4, 0));
    h = fold_v(h, cmp(v3, v4, 1));
    h = fold_v(h, cmp(v3, v4, 2));
    h = fold_v(h, cmp(v3, v4, 3));
    h = fold_v(h, cmp(v3, v4, 4));
    h = fold_v(h, cmp(v3, v4, 5));
    V v5 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x10A), UINT64_C(0xFFFF), UINT64_C(0x5), UINT64_C(0x5), UINT64_C(0x64), UINT64_C(0x64), UINT64_C(0x7), UINT64_C(0x8)});
    seed_lane(&v5, seed);
    V v6 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x64), UINT64_C(0x63), UINT64_C(0x7), UINT64_C(0x8)});
    seed_lane(&v6, seed);
    h = fold_v(h, cmp(v5, v6, 0));
    h = fold_v(h, cmp(v5, v6, 1));
    h = fold_v(h, cmp(v5, v6, 2));
    h = fold_v(h, cmp(v5, v6, 3));
    h = fold_v(h, cmp(v5, v6, 4));
    h = fold_v(h, cmp(v5, v6, 5));
    V v7 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0x10A), UINT64_C(0xFFFF), UINT64_C(0x5), UINT64_C(0x5), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v7, seed);
    V v8 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x6), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v8, seed);
    h = fold_v(h, cmp(v7, v8, 0));
    h = fold_v(h, cmp(v7, v8, 1));
    h = fold_v(h, cmp(v7, v8, 2));
    h = fold_v(h, cmp(v7, v8, 3));
    h = fold_v(h, cmp(v7, v8, 4));
    h = fold_v(h, cmp(v7, v8, 5));
    V v9 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x1), UINT64_C(0xFF), UINT64_C(0x5), UINT64_C(0x64), UINT64_C(0x64), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x9), UINT64_C(0xA), UINT64_C(0xB), UINT64_C(0xC), UINT64_C(0xD), UINT64_C(0xE), UINT64_C(0xF), UINT64_C(0x10)});
    seed_lane(&v9, seed);
    V v10 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x2), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x64), UINT64_C(0x63), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x9), UINT64_C(0xA), UINT64_C(0xB), UINT64_C(0xC), UINT64_C(0xD), UINT64_C(0xE), UINT64_C(0xF), UINT64_C(0x10)});
    seed_lane(&v10, seed);
    h = fold_v(h, cmp(v9, v10, 0));
    h = fold_v(h, cmp(v9, v10, 1));
    h = fold_v(h, cmp(v9, v10, 2));
    h = fold_v(h, cmp(v9, v10, 3));
    h = fold_v(h, cmp(v9, v10, 4));
    h = fold_v(h, cmp(v9, v10, 5));
    V v11 = vi(16, 8, 0, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x1), UINT64_C(0xFF), UINT64_C(0x5), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v11, seed);
    V v12 = vi(16, 8, 0, (const uint64_t[]){UINT64_C(0xA), UINT64_C(0x2), UINT64_C(0x0), UINT64_C(0x5), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0), UINT64_C(0x0)});
    seed_lane(&v12, seed);
    h = fold_v(h, cmp(v11, v12, 0));
    h = fold_v(h, cmp(v11, v12, 1));
    h = fold_v(h, cmp(v11, v12, 2));
    h = fold_v(h, cmp(v11, v12, 3));
    h = fold_v(h, cmp(v11, v12, 4));
    h = fold_v(h, cmp(v11, v12, 5));
    V v13 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x1FFFFFFFF), UINT64_C(0x100000001)});
    seed_lane(&v13, seed);
    V v14 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x100000001), UINT64_C(0x1FFFFFFFF)});
    seed_lane(&v14, seed);
    h = fold_v(h, cmp(v13, v14, 0));
    h = fold_v(h, cmp(v13, v14, 1));
    h = fold_v(h, cmp(v13, v14, 2));
    h = fold_v(h, cmp(v13, v14, 3));
    h = fold_v(h, cmp(v13, v14, 4));
    h = fold_v(h, cmp(v13, v14, 5));
    V v15 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0xFFFFFFFFFFFFFFFF), UINT64_C(0xFFFFFFFF00000000)});
    seed_lane(&v15, seed);
    V v16 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0xFFFFFFFF00000000), UINT64_C(0xFFFFFFFFFFFFFFFF)});
    seed_lane(&v16, seed);
    h = fold_v(h, cmp(v15, v16, 0));
    h = fold_v(h, cmp(v15, v16, 1));
    h = fold_v(h, cmp(v15, v16, 2));
    h = fold_v(h, cmp(v15, v16, 3));
    h = fold_v(h, cmp(v15, v16, 4));
    h = fold_v(h, cmp(v15, v16, 5));
    V v17 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x8000000000000000), UINT64_C(0x7FFFFFFFFFFFFFFF)});
    seed_lane(&v17, seed);
    V v18 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x7FFFFFFFFFFFFFFF), UINT64_C(0x8000000000000000)});
    seed_lane(&v18, seed);
    h = fold_v(h, cmp(v17, v18, 0));
    h = fold_v(h, cmp(v17, v18, 1));
    h = fold_v(h, cmp(v17, v18, 2));
    h = fold_v(h, cmp(v17, v18, 3));
    h = fold_v(h, cmp(v17, v18, 4));
    h = fold_v(h, cmp(v17, v18, 5));
    V v19 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x8000000000000000), UINT64_C(0x123456789ABCDEF0)});
    seed_lane(&v19, seed);
    V v20 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0xFFFFFFFFFFFFFFFF), UINT64_C(0x123456789ABCDEF0)});
    seed_lane(&v20, seed);
    h = fold_v(h, cmp(v19, v20, 0));
    h = fold_v(h, cmp(v19, v20, 1));
    h = fold_v(h, cmp(v19, v20, 2));
    h = fold_v(h, cmp(v19, v20, 3));
    h = fold_v(h, cmp(v19, v20, 4));
    h = fold_v(h, cmp(v19, v20, 5));
    V v21 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x200000000), UINT64_C(0x0)});
    seed_lane(&v21, seed);
    V v22 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x1FFFFFFFF), UINT64_C(0xFFFFFFFFFFFFFFFF)});
    seed_lane(&v22, seed);
    h = fold_v(h, cmp(v21, v22, 0));
    h = fold_v(h, cmp(v21, v22, 1));
    h = fold_v(h, cmp(v21, v22, 2));
    h = fold_v(h, cmp(v21, v22, 3));
    h = fold_v(h, cmp(v21, v22, 4));
    h = fold_v(h, cmp(v21, v22, 5));
    V v23 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x8000000000000000), UINT64_C(0x7FFFFFFFFFFFFFFF)});
    seed_lane(&v23, seed);
    V v24 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x7FFFFFFFFFFFFFFF), UINT64_C(0x8000000000000000)});
    seed_lane(&v24, seed);
    h = fold_v(h, cmp(v23, v24, 0));
    h = fold_v(h, cmp(v23, v24, 1));
    h = fold_v(h, cmp(v23, v24, 2));
    h = fold_v(h, cmp(v23, v24, 3));
    h = fold_v(h, cmp(v23, v24, 4));
    h = fold_v(h, cmp(v23, v24, 5));
    V v25 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x1FFFFFFFF), UINT64_C(0x100000001)});
    seed_lane(&v25, seed);
    V v26 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x100000001), UINT64_C(0x1FFFFFFFF)});
    seed_lane(&v26, seed);
    h = fold_v(h, cmp(v25, v26, 0));
    h = fold_v(h, cmp(v25, v26, 1));
    h = fold_v(h, cmp(v25, v26, 2));
    h = fold_v(h, cmp(v25, v26, 3));
    h = fold_v(h, cmp(v25, v26, 4));
    h = fold_v(h, cmp(v25, v26, 5));
    V v27 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0xFFFFFFFFFFFFFFFF), UINT64_C(0x0)});
    seed_lane(&v27, seed);
    V v28 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0xFFFFFFFFFFFFFFFF)});
    seed_lane(&v28, seed);
    h = fold_v(h, cmp(v27, v28, 0));
    h = fold_v(h, cmp(v27, v28, 1));
    h = fold_v(h, cmp(v27, v28, 2));
    h = fold_v(h, cmp(v27, v28, 3));
    h = fold_v(h, cmp(v27, v28, 4));
    h = fold_v(h, cmp(v27, v28, 5));
    V v29 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x8000000080000000), UINT64_C(0xFFFFFFFF00000000)});
    seed_lane(&v29, seed);
    V v30 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x8000000080000000), UINT64_C(0xFFFFFFFEFFFFFFFF)});
    seed_lane(&v30, seed);
    h = fold_v(h, cmp(v29, v30, 0));
    h = fold_v(h, cmp(v29, v30, 1));
    h = fold_v(h, cmp(v29, v30, 2));
    h = fold_v(h, cmp(v29, v30, 3));
    h = fold_v(h, cmp(v29, v30, 4));
    h = fold_v(h, cmp(v29, v30, 5));
    V v31 = vf(4, 32, (const double[]){1.0, 2.0, 3.0, 3.0});
    seed_lane(&v31, seed);
    V v32 = vf(4, 32, (const double[]){2.0, 2.0, 1.0, 3.0});
    seed_lane(&v32, seed);
    h = fold_v(h, cmp(v31, v32, 0));
    h = fold_v(h, cmp(v31, v32, 1));
    h = fold_v(h, cmp(v31, v32, 2));
    h = fold_v(h, cmp(v31, v32, 3));
    h = fold_v(h, cmp(v31, v32, 4));
    h = fold_v(h, cmp(v31, v32, 5));
    V v33 = vf(2, 64, (const double[]){1.0, 3.0});
    seed_lane(&v33, seed);
    V v34 = vf(2, 64, (const double[]){2.0, 3.0});
    seed_lane(&v34, seed);
    h = fold_v(h, cmp(v33, v34, 0));
    h = fold_v(h, cmp(v33, v34, 1));
    h = fold_v(h, cmp(v33, v34, 2));
    h = fold_v(h, cmp(v33, v34, 3));
    h = fold_v(h, cmp(v33, v34, 4));
    h = fold_v(h, cmp(v33, v34, 5));
    return h;
}
