#include "corpus.h"

/* each lane shifted by its count read as unsigned, saturating at the lane width. */

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
    V v1 = vi(16, 8, 0, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0xA5), UINT64_C(0xFF), UINT64_C(0x3), UINT64_C(0x40), UINT64_C(0x81), UINT64_C(0x1), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0xA5), UINT64_C(0xFF), UINT64_C(0x3), UINT64_C(0x40), UINT64_C(0x81)});
    seed_lane(&v1, seed);
    h = fold_v(h, shl(v1, splat(v1, 2)));
    h = fold_v(h, shr(v1, splat(v1, 7)));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x0))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x0))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x1))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x1))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x7))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x7))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x8))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x8))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x9))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x9))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x10))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x10))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0x21))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0x21))));
    h = fold_v(h, shl(v1, splat(v1, UINT64_C(0xFF))));
    h = fold_v(h, shr(v1, splat(v1, UINT64_C(0xFF))));
    V v2 = vi(16, 8, 0, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x3), UINT64_C(0xC8), UINT64_C(0x5), UINT64_C(0x9), UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x3), UINT64_C(0xC8), UINT64_C(0x5), UINT64_C(0x9)});
    h = fold_v(h, shl(v1, v2));
    h = fold_v(h, shr(v1, v2));
    V v3 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0xA5), UINT64_C(0xFF), UINT64_C(0x3), UINT64_C(0x40), UINT64_C(0x81), UINT64_C(0x1), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0xA5), UINT64_C(0xFF), UINT64_C(0x3), UINT64_C(0x40), UINT64_C(0x81)});
    seed_lane(&v3, seed);
    h = fold_v(h, shl(v3, splat(v3, 2)));
    h = fold_v(h, shr(v3, splat(v3, 7)));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x0))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x0))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x1))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x1))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x7))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x7))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x8))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x8))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x9))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x9))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x10))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x10))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x21))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x21))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x7F))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x7F))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0xFF))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0xFF))));
    h = fold_v(h, shl(v3, splat(v3, UINT64_C(0x80))));
    h = fold_v(h, shr(v3, splat(v3, UINT64_C(0x80))));
    V v4 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x3), UINT64_C(0xFF), UINT64_C(0x5), UINT64_C(0x9), UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x7), UINT64_C(0x8), UINT64_C(0x3), UINT64_C(0xFF), UINT64_C(0x5), UINT64_C(0x9)});
    h = fold_v(h, shl(v3, v4));
    h = fold_v(h, shr(v3, v4));
    V v5 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x8000), UINT64_C(0x7FFF), UINT64_C(0xA55A), UINT64_C(0xFFFF), UINT64_C(0x3), UINT64_C(0x4000), UINT64_C(0x8001)});
    seed_lane(&v5, seed);
    h = fold_v(h, shl(v5, splat(v5, 5)));
    h = fold_v(h, shr(v5, splat(v5, 15)));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x0))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x0))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x1))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x1))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0xF))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0xF))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x10))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x10))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x11))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x11))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x21))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x21))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0x101))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0x101))));
    h = fold_v(h, shl(v5, splat(v5, UINT64_C(0xFFFF))));
    h = fold_v(h, shr(v5, splat(v5, UINT64_C(0xFFFF))));
    V v6 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0xF), UINT64_C(0x10), UINT64_C(0x3), UINT64_C(0x12C), UINT64_C(0x5), UINT64_C(0x11)});
    h = fold_v(h, shl(v5, v6));
    h = fold_v(h, shr(v5, v6));
    V v7 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x100), UINT64_C(0x10), UINT64_C(0xFFFF), UINT64_C(0x201), UINT64_C(0x2), UINT64_C(0x11), UINT64_C(0x102)});
    h = fold_v(h, shl(v5, v7));
    h = fold_v(h, shr(v5, v7));
    V v8 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x8000), UINT64_C(0x7FFF), UINT64_C(0xA55A), UINT64_C(0xFFFF), UINT64_C(0x3), UINT64_C(0x4000), UINT64_C(0x8001)});
    seed_lane(&v8, seed);
    h = fold_v(h, shl(v8, splat(v8, 5)));
    h = fold_v(h, shr(v8, splat(v8, 15)));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x0))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x0))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x1))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x1))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0xF))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0xF))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x10))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x10))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x11))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x11))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x21))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x21))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x101))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x101))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0xFFFF))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0xFFFF))));
    h = fold_v(h, shl(v8, splat(v8, UINT64_C(0x8000))));
    h = fold_v(h, shr(v8, splat(v8, UINT64_C(0x8000))));
    V v9 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0xF), UINT64_C(0x10), UINT64_C(0x3), UINT64_C(0xFFFF), UINT64_C(0x5), UINT64_C(0x11)});
    h = fold_v(h, shl(v8, v9));
    h = fold_v(h, shr(v8, v9));
    V v10 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x100), UINT64_C(0x10), UINT64_C(0xFFFF), UINT64_C(0x201), UINT64_C(0x2), UINT64_C(0x11), UINT64_C(0x8001)});
    h = fold_v(h, shl(v8, v10));
    h = fold_v(h, shr(v8, v10));
    V v11 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x80000000), UINT64_C(0x7FFFFFFF), UINT64_C(0xDEADBEEF)});
    seed_lane(&v11, seed);
    h = fold_v(h, shl(v11, splat(v11, 10)));
    h = fold_v(h, shr(v11, splat(v11, 31)));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x0))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x0))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x1))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x1))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x1F))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x1F))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x20))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x20))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x21))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x21))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x40))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x40))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0x101))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0x101))));
    h = fold_v(h, shl(v11, splat(v11, UINT64_C(0xFFFFFFFF))));
    h = fold_v(h, shr(v11, splat(v11, UINT64_C(0xFFFFFFFF))));
    V v12 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1F), UINT64_C(0x20), UINT64_C(0x28)});
    h = fold_v(h, shl(v11, v12));
    h = fold_v(h, shr(v11, v12));
    V v13 = vi(4, 32, 0, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x100), UINT64_C(0x21), UINT64_C(0xFFFFFF01)});
    h = fold_v(h, shl(v11, v13));
    h = fold_v(h, shr(v11, v13));
    V v14 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0x80000000), UINT64_C(0x7FFFFFFF), UINT64_C(0xDEADBEEF)});
    seed_lane(&v14, seed);
    h = fold_v(h, shl(v14, splat(v14, 10)));
    h = fold_v(h, shr(v14, splat(v14, 31)));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x0))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x0))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x1))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x1))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x1F))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x1F))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x20))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x20))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x21))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x21))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x40))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x40))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0xFFFFFFFF))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0xFFFFFFFF))));
    h = fold_v(h, shl(v14, splat(v14, UINT64_C(0x80000000))));
    h = fold_v(h, shr(v14, splat(v14, UINT64_C(0x80000000))));
    V v15 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x0), UINT64_C(0x1F), UINT64_C(0x20), UINT64_C(0xFFFFFFFF)});
    h = fold_v(h, shl(v14, v15));
    h = fold_v(h, shr(v14, v15));
    V v16 = vi(4, 32, 1, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x100), UINT64_C(0x21), UINT64_C(0xFFFFFF01)});
    h = fold_v(h, shl(v14, v16));
    h = fold_v(h, shr(v14, v16));
    V v17 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x8000000000000001), UINT64_C(0x123456789ABCDEF)});
    seed_lane(&v17, seed);
    h = fold_v(h, shl(v17, splat(v17, 21)));
    h = fold_v(h, shr(v17, splat(v17, 63)));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x0))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x0))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x1))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x1))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x3F))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x3F))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x40))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x40))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x41))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x41))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x80))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x80))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0x100000001))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0x100000001))));
    h = fold_v(h, shl(v17, splat(v17, UINT64_C(0xFFFFFFFFFFFFFFFF))));
    h = fold_v(h, shr(v17, splat(v17, UINT64_C(0xFFFFFFFFFFFFFFFF))));
    V v18 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x3F), UINT64_C(0x40)});
    h = fold_v(h, shl(v17, v18));
    h = fold_v(h, shr(v17, v18));
    V v19 = vi(2, 64, 0, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x41)});
    h = fold_v(h, shl(v17, v19));
    h = fold_v(h, shr(v17, v19));
    V v20 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x8000000000000001), UINT64_C(0x123456789ABCDEF)});
    seed_lane(&v20, seed);
    h = fold_v(h, shl(v20, splat(v20, 21)));
    h = fold_v(h, shr(v20, splat(v20, 63)));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x0))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x0))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x1))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x1))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x3F))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x3F))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x40))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x40))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x41))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x41))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x80))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x80))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0xFFFFFFFFFFFFFFFF))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0xFFFFFFFFFFFFFFFF))));
    h = fold_v(h, shl(v20, splat(v20, UINT64_C(0x8000000000000001))));
    h = fold_v(h, shr(v20, splat(v20, UINT64_C(0x8000000000000001))));
    V v21 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x1), UINT64_C(0xFFFFFFFFFFFFFFC0)});
    h = fold_v(h, shl(v20, v21));
    h = fold_v(h, shr(v20, v21));
    V v22 = vi(2, 64, 1, (const uint64_t[]){UINT64_C(0x101), UINT64_C(0x100)});
    h = fold_v(h, shl(v20, v22));
    h = fold_v(h, shr(v20, v22));
    return h;
}
