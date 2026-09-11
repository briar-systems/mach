#include "corpus.h"

typedef struct { uint8_t d; } P1;
typedef struct { uint8_t d; union { uint8_t b[2]; } p; } T3;
typedef struct { uint8_t d; union { uint32_t b; } p; } S8;
typedef struct { uint8_t d; union { uint8_t b[8]; } p; } T9;
typedef struct { uint8_t d; union { int64_t b; } p; } T16;
typedef struct { uint8_t d; union { int64_t b[2]; uint8_t c; } p; } O24;
typedef struct { uint8_t d; union { double b; } p; } TF16;
typedef struct { uint8_t d; union { float b; } p; } TF8;
typedef struct { uint8_t d; union { double b; int64_t c; } p; } TFM;
typedef struct { uint8_t d; union { S8 b; } p; } Nest;

static P1 p1_a(void) { P1 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static P1 p1_b(void) { P1 v; memset(&v, 0, sizeof v); v.d = 1; return v; }
static T3 t3_a(void) { T3 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static T3 t3_b(uint8_t x, uint8_t y) { T3 v; memset(&v, 0, sizeof v); v.p.b[0] = x; v.p.b[1] = y; v.d = 1; return v; }
static S8 s8_a(void) { S8 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static S8 s8_b(uint32_t x) { S8 v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }
static T9 t9_a(void) { T9 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static T9 t9_b(const uint8_t *x) { T9 v; memset(&v, 0, sizeof v); memcpy(v.p.b, x, 8); v.d = 1; return v; }
static T16 t16_a(void) { T16 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static T16 t16_b(int64_t x) { T16 v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }
static O24 o24_a(void) { O24 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static O24 o24_b(int64_t x, int64_t y) { O24 v; memset(&v, 0, sizeof v); v.p.b[0] = x; v.p.b[1] = y; v.d = 1; return v; }
static O24 o24_c(uint8_t x) { O24 v; memset(&v, 0, sizeof v); v.p.c = x; v.d = 2; return v; }
static TF16 tf16_a(void) { TF16 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static TF16 tf16_b(double x) { TF16 v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }
static TF8 tf8_a(void) { TF8 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static TF8 tf8_b(float x) { TF8 v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }
static TFM tfm_a(void) { TFM v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static TFM tfm_b(double x) { TFM v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }
static TFM tfm_c(int64_t x) { TFM v; memset(&v, 0, sizeof v); v.p.c = x; v.d = 2; return v; }
static Nest nest_a(void) { Nest v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static Nest nest_b(S8 x) { Nest v; memset(&v, 0, sizeof v); v.p.b = x; v.d = 1; return v; }

static P1 flip_p1(P1 v) {
    if (v.d == 0) { return p1_b(); }
    return p1_a();
}

static T3 bump_t3(T3 v) {
    if (v.d == 1) { return t3_b((uint8_t)(v.p.b[1] + 1), (uint8_t)(v.p.b[0] + 2)); }
    return t3_b(1, 2);
}

static S8 bump_s8(S8 v) {
    if (v.d == 1) { return s8_b((uint32_t)(v.p.b * UINT32_C(3))); }
    return v;
}

static uint64_t sum_t9(T9 v) {
    if (v.d != 1) { return 0; }
    uint64_t acc = 0;
    uint64_t i = 0;
    while (i < 8) {
        acc = (uint64_t)(acc * UINT64_C(31) + (uint64_t)v.p.b[i]);
        i = i + 1;
    }
    return acc;
}

static T9 rot_t9(T9 v) {
    if (v.d != 1) { return v; }
    uint8_t out[8];
    memset(out, 0, sizeof out);
    uint64_t i = 0;
    while (i < 8) {
        out[i] = v.p.b[(i + 1) & 7];
        i = i + 1;
    }
    return t9_b(out);
}

static T16 bump_t16(T16 v) {
    if (v.d == 1) { return t16_b(v.p.b + 1); }
    return t16_b(-1);
}

static int64_t sum_o24(O24 v) {
    int64_t out = -1;
    if (v.d == 1) { out = v.p.b[0] * 7 + v.p.b[1]; }
    else if (v.d == 2) { out = (int64_t)v.p.c; }
    return out;
}

static O24 swap_o24(O24 v) {
    if (v.d == 1) { return o24_b(v.p.b[1], v.p.b[0]); }
    return o24_c(200);
}

static TF16 half_tf16(TF16 v) {
    if (v.d == 1) { return tf16_b(v.p.b * 0.5); }
    return tf16_b(1.0);
}

static TF8 double_tf8(TF8 v) {
    if (v.d == 1) { return tf8_b(v.p.b * 2.0f); }
    return tf8_b(0.25f);
}

static TFM cross_tfm(TFM v) {
    TFM out = v;
    if (v.d == 1) { out = tfm_c((int64_t)v.p.b); }
    else if (v.d == 2) { out = tfm_b((double)v.p.c + 0.5); }
    return out;
}

static uint32_t nest_of(Nest v) {
    uint32_t out = 0;
    if (v.d == 1) {
        out = 1;
        if (v.p.b.d == 1) { out = (uint32_t)(v.p.b.p.b + UINT32_C(1)); }
    }
    return out;
}

static T16 three(T16 x, T9 y, O24 z, uint64_t k) {
    int64_t acc = (int64_t)k;
    if (x.d == 1) { acc = acc + x.p.b; }
    acc = acc + (int64_t)sum_t9(y);
    acc = acc + sum_o24(z);
    return t16_b(acc);
}

static void poke(T16 *p, int64_t by) {
    if (p->d == 1) { p->p.b = p->p.b + by; }
    else { *p = t16_b(by); }
}

static uint64_t fold_t3(uint64_t h, T3 v) {
    if (v.d == 1) { return mix_u8(mix_u8(h, v.p.b[0]), v.p.b[1]); }
    return mix_u8(h, 255);
}

static uint64_t fold_s8(uint64_t h, S8 v) {
    if (v.d == 1) { return mix_u32(h, v.p.b); }
    return mix_u32(h, UINT32_C(4294967295));
}

static uint64_t fold_t16(uint64_t h, T16 v) {
    if (v.d == 1) { return mix_i64(h, v.p.b); }
    return mix_i64(h, -2);
}

static uint64_t fold_tf16(uint64_t h, TF16 v) {
    if (v.d == 1) { return mix_f64(h, v.p.b); }
    return mix_u8(h, 0);
}

static uint64_t fold_tf8(uint64_t h, TF8 v) {
    if (v.d == 1) { return mix_f32(h, v.p.b); }
    return mix_u8(h, 0);
}

static uint64_t fold_tfm(uint64_t h, TFM v) {
    uint64_t out = mix_u8(h, 0);
    if (v.d == 1) { out = mix_f64(h, v.p.b); }
    else if (v.d == 2) { out = mix_i64(h, v.p.c); }
    return out;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;
    const int64_t si = (int64_t)seed;

    P1 p = flip_p1(p1_a());
    if (p.d == 1) { h = mix_u8(h, 1); } else { h = mix_u8(h, 0); }
    p = flip_p1(p);
    if (p.d == 1) { h = mix_u8(h, 1); } else { h = mix_u8(h, 0); }

    T3 t3 = bump_t3(t3_a());
    h = fold_t3(h, t3);
    t3 = bump_t3(t3_b((uint8_t)(10 + s), (uint8_t)(20 + s)));
    h = fold_t3(h, t3);

    S8 s8 = bump_s8(s8_b((uint32_t)(UINT32_C(1000) + (uint32_t)seed)));
    h = fold_s8(h, s8);
    h = fold_s8(h, bump_s8(s8_a()));

    const uint8_t init9[8] = { (uint8_t)(1 + s), 2, 3, 4, 5, 6, 7, 8 };
    T9 t9 = t9_b(init9);
    h = mix_u64(h, sum_t9(t9));
    t9 = rot_t9(t9);
    h = mix_u64(h, sum_t9(t9));
    h = mix_u64(h, sum_t9(t9_a()));

    T16 t16 = bump_t16(t16_b(123456789 + si));
    h = fold_t16(h, t16);
    h = fold_t16(h, bump_t16(t16_a()));

    O24 o24 = o24_b(3 + si, 4);
    h = mix_i64(h, sum_o24(o24));
    o24 = swap_o24(o24);
    h = mix_i64(h, sum_o24(o24));
    o24 = swap_o24(o24_a());
    h = mix_i64(h, sum_o24(o24));
    h = mix_i64(h, sum_o24(o24_a()));

    TF16 tf16 = half_tf16(tf16_b(5.0 + (double)si));
    h = fold_tf16(h, tf16);
    h = fold_tf16(h, half_tf16(tf16_a()));
    TF8 tf8 = double_tf8(tf8_b(1.25f + (float)si));
    h = fold_tf8(h, tf8);
    h = fold_tf8(h, double_tf8(tf8_a()));
    TFM tfm = cross_tfm(tfm_b(9.0 + (double)si));
    h = fold_tfm(h, tfm);
    tfm = cross_tfm(tfm);
    h = fold_tfm(h, tfm);
    h = fold_tfm(h, cross_tfm(tfm_a()));

    h = mix_u32(h, nest_of(nest_b(s8_b((uint32_t)(UINT32_C(77) + (uint32_t)seed)))));
    h = mix_u32(h, nest_of(nest_b(s8_a())));
    h = mix_u32(h, nest_of(nest_a()));

    h = fold_t16(h, three(t16, t9, o24, (uint64_t)(UINT64_C(5) + seed)));
    h = fold_t16(h, three(t16_a(), t9_a(), o24_a(), seed));

    T16 q = t16_a();
    poke(&q, 40 + si);
    h = fold_t16(h, q);
    poke(&q, 2);
    h = fold_t16(h, q);
    return h;
}
