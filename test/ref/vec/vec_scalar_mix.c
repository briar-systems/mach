#include "corpus.h"

/* each shape fills the vector register, so each C struct aligns to 16 and matches
 * the mach type's size.
 *
 * integer lanes are held unsigned. s32_to_f32 reproduces mach's i32-to-f32 value
 * conversion without ever forming a signed integer: a negative lane is negated as
 * unsigned first and the float is negated afterwards. f32_to_s32 goes the other way
 * through int64_t, which is in range for every value this case produces, and then
 * back to unsigned by the modular conversion C defines. */
typedef struct { _Alignas(16) float l[4]; } f32x4;
typedef struct { _Alignas(16) uint32_t l[4]; } i32x4;
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;

static uint64_t sx32(uint32_t v) {
    return (uint64_t)(((uint64_t)v ^ UINT64_C(0x80000000)) - UINT64_C(0x80000000));
}

static float s32_to_f32(uint32_t v) {
    if ((v & UINT32_C(0x80000000)) != 0) { return -(float)(uint32_t)(-(uint32_t)v); }
    return (float)v;
}

static uint32_t f32_to_s32(float v) {
    return (uint32_t)(uint64_t)(int64_t)v;
}

static uint64_t fold_f32x4(uint64_t h, f32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

static uint64_t fold_i32x4(uint64_t h, i32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_u64(h, sx32(v.l[i])); }
    return h;
}

static uint64_t fold_u32x4(uint64_t h, u32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_u32(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;
    const uint32_t s = (uint32_t)seed;
    unsigned i;

    f32x4 a = { { 1.5f, -2.25f, 3.75f, -0.5f } };
    f32x4 b = { { 0.5f, 4.0f, -0.125f, 8.0f } };
    a.l[0] = a.l[0] + z;
    b.l[2] = b.l[2] + z;

    const float p0 = a.l[0] * b.l[0];
    const float p1 = a.l[1] * b.l[1];
    const float p2 = a.l[2] * b.l[2];
    const float p3 = a.l[3] * b.l[3];
    h = mix_f32(h, p0);
    h = mix_f32(h, p1);
    h = mix_f32(h, p2);
    h = mix_f32(h, p3);

    float dot = p0 + p1;
    h = mix_f32(h, dot);
    dot = dot + p2;
    h = mix_f32(h, dot);
    dot = dot + p3;
    h = mix_f32(h, dot);

    f32x4 c = a;
    c.l[0] = dot;
    c.l[1] = a.l[3] - b.l[0];
    c.l[2] = b.l[1] + a.l[2];
    c.l[3] = 0.0f - a.l[1];
    h = fold_f32x4(h, c);
    f32x4 cb;
    for (i = 0; i < 4u; i++) { cb.l[i] = c.l[i] * b.l[i]; }
    h = fold_f32x4(h, cb);

    float mx = a.l[0];
    if (a.l[1] > mx) { mx = a.l[1]; }
    if (a.l[2] > mx) { mx = a.l[2]; }
    if (a.l[3] > mx) { mx = a.l[3]; }
    float mn = a.l[0];
    if (a.l[1] < mn) { mn = a.l[1]; }
    if (a.l[2] < mn) { mn = a.l[2]; }
    if (a.l[3] < mn) { mn = a.l[3]; }
    h = mix_f32(h, mx);
    h = mix_f32(h, mn);
    h = mix_f32(h, mx - mn);

    i32x4 iv = { { UINT32_C(7), (uint32_t)-UINT32_C(13), UINT32_C(21), (uint32_t)-UINT32_C(31) } };
    iv.l[1] = (uint32_t)(iv.l[1] + s);
    f32x4 fv;
    for (i = 0; i < 4u; i++) { fv.l[i] = s32_to_f32(iv.l[i]); }
    h = fold_f32x4(h, fv);
    f32x4 fva;
    for (i = 0; i < 4u; i++) { fva.l[i] = fv.l[i] * a.l[i]; }
    h = fold_f32x4(h, fva);

    i32x4 back;
    for (i = 0; i < 4u; i++) { back.l[i] = f32_to_s32(fv.l[i] * 4.0f); }
    h = fold_i32x4(h, back);
    i32x4 bt;
    for (i = 0; i < 4u; i++) { bt.l[i] = (uint32_t)(back.l[i] * iv.l[i]); }
    h = fold_i32x4(h, bt);
    for (i = 0; i < 4u; i++) { bt.l[i] = (uint32_t)(back.l[i] - iv.l[i]); }
    h = fold_i32x4(h, bt);

    u32x4 uv = { { UINT32_C(4294967295), UINT32_C(1), UINT32_C(2863311530), UINT32_C(305419896) } };
    uv.l[3] = (uint32_t)(uv.l[3] + s);
    uint32_t acc = 0;
    acc = (uint32_t)(acc + uv.l[0]);
    h = mix_u32(h, acc);
    acc = (uint32_t)(acc ^ uv.l[1]);
    h = mix_u32(h, acc);
    acc = (uint32_t)(acc * uv.l[2]);
    h = mix_u32(h, acc);
    acc = (uint32_t)(acc - uv.l[3]);
    h = mix_u32(h, acc);

    u32x4 out;
    out.l[0] = acc;
    out.l[1] = (uint32_t)(acc ^ uv.l[0]);
    out.l[2] = (uint32_t)(acc + uv.l[1]);
    out.l[3] = (uint32_t)(acc - uv.l[2]);
    h = fold_u32x4(h, out);
    u32x4 ou;
    for (i = 0; i < 4u; i++) { ou.l[i] = (uint32_t)(out.l[i] + uv.l[i]); }
    h = fold_u32x4(h, ou);

    f32x4 vv = a;
    for (uint64_t n = 0; n < UINT64_C(4); n = (uint64_t)(n + UINT64_C(1))) {
        const float t0 = vv.l[0] + vv.l[3];
        const float t1 = vv.l[1] - vv.l[2];
        const float t2 = vv.l[2] * 0.5f;
        const float t3 = vv.l[3] + 1.25f;
        vv.l[0] = t0;
        vv.l[1] = t1;
        vv.l[2] = t2;
        vv.l[3] = t3;
        h = fold_f32x4(h, vv);
    }
    return h;
}
