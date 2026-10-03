#include "corpus.h"

/* each lane widened by its own signedness, then multiplied at the wide width. */

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
    V v1 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0x80), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0x7F), UINT64_C(0x80), UINT64_C(0xFF), UINT64_C(0x0), UINT64_C(0x1), UINT64_C(0x81), UINT64_C(0x7E), UINT64_C(0x7), UINT64_C(0xF9), UINT64_C(0x64), UINT64_C(0x9C), UINT64_C(0x1), UINT64_C(0xFF)});
    seed_lane(&v1, seed);
    V v2 = vi(16, 8, 1, (const uint64_t[]){UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0x7F), UINT64_C(0x80), UINT64_C(0xFF), UINT64_C(0x80), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0x3), UINT64_C(0xFB), UINT64_C(0x80), UINT64_C(0x7F), UINT64_C(0x2), UINT64_C(0x64), UINT64_C(0x0), UINT64_C(0xFF)});
    seed_lane(&v2, seed);
    h = fold_v(h, mul(widen(v1, 16, 1), widen(v2, 16, 1)));
    h = fold_v(h, mul(widen(part(v1, 0, 8), 16, 1), widen(part(v2, 0, 8), 16, 1)));
    h = fold_v(h, mul(widen(part(v1, 8, 8), 16, 1), widen(part(v2, 8, 8), 16, 1)));
    V v3 = vi(8, 8, 0, (const uint64_t[]){UINT64_C(0xFF), UINT64_C(0xFF), UINT64_C(0x0), UINT64_C(0xFF), UINT64_C(0x80), UINT64_C(0x1), UINT64_C(0xC8), UINT64_C(0x11)});
    seed_lane(&v3, seed);
    V v4 = vi(8, 8, 0, (const uint64_t[]){UINT64_C(0xFF), UINT64_C(0x0), UINT64_C(0xFF), UINT64_C(0x1), UINT64_C(0x80), UINT64_C(0xFF), UINT64_C(0xC8), UINT64_C(0xF)});
    seed_lane(&v4, seed);
    h = fold_v(h, mul(widen(v3, 16, 0), widen(v4, 16, 0)));
    V v5 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x8000), UINT64_C(0x8000), UINT64_C(0x7FFF), UINT64_C(0x8000), UINT64_C(0xFFFF), UINT64_C(0x7FFF), UINT64_C(0x12C), UINT64_C(0xFFFD)});
    seed_lane(&v5, seed);
    V v6 = vi(8, 16, 1, (const uint64_t[]){UINT64_C(0x8000), UINT64_C(0x7FFF), UINT64_C(0x7FFF), UINT64_C(0xFFFF), UINT64_C(0x8000), UINT64_C(0xFFFF), UINT64_C(0xFED4), UINT64_C(0x7FFF)});
    seed_lane(&v6, seed);
    h = fold_v(h, mul(widen(v5, 32, 1), widen(v6, 32, 1)));
    h = fold_v(h, mul(widen(part(v5, 0, 4), 32, 1), widen(part(v6, 0, 4), 32, 1)));
    h = fold_v(h, mul(widen(part(v5, 4, 4), 32, 1), widen(part(v6, 4, 4), 32, 1)));
    V v7 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0xFFFF), UINT64_C(0xFFFF), UINT64_C(0x0), UINT64_C(0xFFFF), UINT64_C(0x8000), UINT64_C(0x1), UINT64_C(0x9C40), UINT64_C(0x101)});
    seed_lane(&v7, seed);
    V v8 = vi(8, 16, 0, (const uint64_t[]){UINT64_C(0xFFFF), UINT64_C(0x0), UINT64_C(0xFFFF), UINT64_C(0x1), UINT64_C(0x8000), UINT64_C(0xFFFF), UINT64_C(0x9C40), UINT64_C(0xFF)});
    seed_lane(&v8, seed);
    h = fold_v(h, mul(widen(v7, 32, 0), widen(v8, 32, 0)));
    h = fold_v(h, mul(widen(part(v7, 0, 4), 32, 0), widen(part(v8, 0, 4), 32, 0)));
    h = fold_v(h, mul(widen(part(v7, 4, 4), 32, 0), widen(part(v8, 4, 4), 32, 0)));
    V v9 = vi(4, 16, 1, (const uint64_t[]){UINT64_C(0x8000), UINT64_C(0x7FFF), UINT64_C(0xFFFF), UINT64_C(0x5)});
    seed_lane(&v9, seed);
    V v10 = vi(4, 16, 0, (const uint64_t[]){UINT64_C(0xFFFF), UINT64_C(0xFFFF), UINT64_C(0xFFFF), UINT64_C(0x0)});
    seed_lane(&v10, seed);
    h = fold_v(h, mul(widen(v9, 32, 1), widen(v10, 32, 1)));
    V v11 = vi(2, 32, 1, (const uint64_t[]){UINT64_C(0x80000000), UINT64_C(0x80000000)});
    seed_lane(&v11, seed);
    V v12 = vi(2, 32, 1, (const uint64_t[]){UINT64_C(0x80000000), UINT64_C(0x7FFFFFFF)});
    seed_lane(&v12, seed);
    h = fold_v(h, mul(widen(v11, 64, 1), widen(v12, 64, 1)));
    V v13 = vi(2, 32, 1, (const uint64_t[]){UINT64_C(0x7FFFFFFF), UINT64_C(0x75BCD15)});
    seed_lane(&v13, seed);
    V v14 = vi(2, 32, 1, (const uint64_t[]){UINT64_C(0xFFFFFFFF), UINT64_C(0xC521974F)});
    seed_lane(&v14, seed);
    h = fold_v(h, mul(widen(v13, 64, 1), widen(v14, 64, 1)));
    V v15 = vi(2, 32, 0, (const uint64_t[]){UINT64_C(0xFFFFFFFF), UINT64_C(0xFFFFFFFF)});
    seed_lane(&v15, seed);
    V v16 = vi(2, 32, 0, (const uint64_t[]){UINT64_C(0xFFFFFFFF), UINT64_C(0x2)});
    seed_lane(&v16, seed);
    h = fold_v(h, mul(widen(v15, 64, 0), widen(v16, 64, 0)));
    V v17 = vi(2, 32, 0, (const uint64_t[]){UINT64_C(0x80000000), UINT64_C(0x0)});
    seed_lane(&v17, seed);
    V v18 = vi(2, 32, 0, (const uint64_t[]){UINT64_C(0x80000000), UINT64_C(0xFFFFFFFF)});
    seed_lane(&v18, seed);
    h = fold_v(h, mul(widen(v17, 64, 0), widen(v18, 64, 0)));
    return h;
}
