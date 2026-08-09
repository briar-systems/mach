#include "corpus.h"

/* every shape here fills the vector register, so each C struct aligns to 16 and has
 * the same size as the mach type.
 *
 * a mach comparison yields an unsigned mask of the input's lane width, all-ones for
 * true and all-zeros for false. the reference builds that mask explicitly rather
 * than relying on C's 0/1 comparison result.
 *
 * the signed comparisons are done on unsigned lanes through the sign-bit-flip
 * identity, so no signed type is involved and nothing here is UB. the float bit
 * reinterprets go through memcpy for the same reason. */
typedef struct { _Alignas(16) uint8_t l[16]; } u8x16;
typedef struct { _Alignas(16) uint16_t l[8]; } u16x8;
typedef struct { _Alignas(16) uint32_t l[4]; } u32x4;
typedef struct { _Alignas(16) uint64_t l[2]; } u64x2;
typedef struct { _Alignas(16) float l[4]; } f32x4;
typedef struct { _Alignas(16) double l[2]; } f64x2;

#define M8(c)  ((uint8_t)((c) ? 0xFFu : 0u))
#define M16(c) ((uint16_t)((c) ? 0xFFFFu : 0u))
#define M32(c) ((uint32_t)((c) ? UINT32_C(0xFFFFFFFF) : UINT32_C(0)))
#define M64(c) ((uint64_t)((c) ? UINT64_C(0xFFFFFFFFFFFFFFFF) : UINT64_C(0)))

#define S32(x) ((uint32_t)((x) ^ UINT32_C(0x80000000)))

static uint32_t f32_bits(float v) {
    uint32_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static float f32_from(uint32_t b) {
    float v;
    memcpy(&v, &b, sizeof v);
    return v;
}

static uint64_t fold_u8x16(uint64_t h, u8x16 v) {
    for (unsigned i = 0; i < 16u; i++) { h = mix_u8(h, v.l[i]); }
    return h;
}

static uint64_t fold_u16x8(uint64_t h, u16x8 v) {
    for (unsigned i = 0; i < 8u; i++) { h = mix_u16(h, v.l[i]); }
    return h;
}

static uint64_t fold_u32x4(uint64_t h, u32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_u32(h, v.l[i]); }
    return h;
}

static uint64_t fold_u64x2(uint64_t h, u64x2 v) {
    for (unsigned i = 0; i < 2u; i++) { h = mix_u64(h, v.l[i]); }
    return h;
}

static uint64_t fold_f32x4(uint64_t h, f32x4 v) {
    for (unsigned i = 0; i < 4u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;
    const uint32_t t = (uint32_t)seed;
    const float z = (float)seed;
    const uint16_t w = (uint16_t)seed;
    const uint8_t e = (uint8_t)seed;
    unsigned i;
    u32x4 m32;
    u16x8 m16;
    u8x16 m8;
    u64x2 m64;
    f32x4 fr;

    u32x4 a = { { UINT32_C(10), UINT32_C(20), UINT32_C(30), UINT32_C(4294967295) } };
    u32x4 b = { { UINT32_C(20), UINT32_C(20), UINT32_C(10), UINT32_C(1) } };
    a.l[0] = (uint32_t)(a.l[0] + s);
    b.l[3] = (uint32_t)(b.l[3] + s);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] <  b.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] >  b.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] == b.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] != b.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] <= b.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(a.l[i] >= b.l[i]); } h = fold_u32x4(h, m32);

    u32x4 m;
    for (i = 0; i < 4u; i++) { m.l[i] = M32(a.l[i] < b.l[i]); }
    for (i = 0; i < 4u; i++) { m32.l[i] = (uint32_t)((m.l[i] & a.l[i]) | (~m.l[i] & b.l[i])); }
    h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = (uint32_t)((m.l[i] & b.l[i]) | (~m.l[i] & a.l[i])); }
    h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = (uint32_t)(m.l[i] & (uint32_t)(a.l[i] + b.l[i])); }
    h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = (uint32_t)(~m.l[i] | (uint32_t)(a.l[i] - b.l[i])); }
    h = fold_u32x4(h, m32);

    u32x4 p = { { (uint32_t)-UINT32_C(1), UINT32_C(5), (uint32_t)-UINT32_C(2000000000), UINT32_C(7) } };
    u32x4 q = { { UINT32_C(1), UINT32_C(5), UINT32_C(2000000000), (uint32_t)-UINT32_C(7) } };
    p.l[0] = (uint32_t)(p.l[0] + t);
    q.l[3] = (uint32_t)(q.l[3] + t);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(S32(p.l[i]) <  S32(q.l[i])); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(S32(p.l[i]) >  S32(q.l[i])); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(p.l[i] == q.l[i]); }           h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(p.l[i] != q.l[i]); }           h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(S32(p.l[i]) <= S32(q.l[i])); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(S32(p.l[i]) >= S32(q.l[i])); } h = fold_u32x4(h, m32);

    f32x4 fa = { { 1.5f, -0.0f, 3.0f, -2.0f } };
    f32x4 fb = { { 2.5f, 0.0f, 3.0f, -3.0f } };
    fa.l[0] = fa.l[0] + z;
    h = fold_f32x4(h, fa);
    h = fold_f32x4(h, fb);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] <  fb.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] >  fb.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] == fb.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] != fb.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] <= fb.l[i]); } h = fold_u32x4(h, m32);
    for (i = 0; i < 4u; i++) { m32.l[i] = M32(fa.l[i] >= fb.l[i]); } h = fold_u32x4(h, m32);

    u32x4 fm;
    for (i = 0; i < 4u; i++) { fm.l[i] = M32(fa.l[i] < fb.l[i]); }
    f32x4 lo;
    f32x4 hi;
    for (i = 0; i < 4u; i++) {
        lo.l[i] = f32_from((uint32_t)((fm.l[i] & f32_bits(fa.l[i])) | (~fm.l[i] & f32_bits(fb.l[i]))));
        hi.l[i] = f32_from((uint32_t)((fm.l[i] & f32_bits(fb.l[i])) | (~fm.l[i] & f32_bits(fa.l[i]))));
    }
    h = fold_f32x4(h, lo);
    h = fold_f32x4(h, hi);
    for (i = 0; i < 4u; i++) { fr.l[i] = hi.l[i] - lo.l[i]; }
    h = fold_f32x4(h, fr);

    f64x2 ga = { { 1.5, -0.0 } };
    f64x2 gb = { { 2.5, 0.0 } };
    ga.l[0] = ga.l[0] + (double)seed;
    for (i = 0; i < 2u; i++) { m64.l[i] = M64(ga.l[i] <  gb.l[i]); } h = fold_u64x2(h, m64);
    for (i = 0; i < 2u; i++) { m64.l[i] = M64(ga.l[i] == gb.l[i]); } h = fold_u64x2(h, m64);
    for (i = 0; i < 2u; i++) { m64.l[i] = M64(ga.l[i] != gb.l[i]); } h = fold_u64x2(h, m64);
    for (i = 0; i < 2u; i++) { m64.l[i] = M64(ga.l[i] >= gb.l[i]); } h = fold_u64x2(h, m64);

    u16x8 ha = { { 1, 65535, 32768, 7, 7, 0, 300, 65534 } };
    u16x8 hb = { { 2, 1, 32767, 7, 8, 65535, 300, 65535 } };
    ha.l[0] = (uint16_t)((uint32_t)ha.l[0] + (uint32_t)w);
    hb.l[7] = (uint16_t)((uint32_t)hb.l[7] + (uint32_t)w);
    for (i = 0; i < 8u; i++) { m16.l[i] = M16(ha.l[i] <  hb.l[i]); } h = fold_u16x8(h, m16);
    for (i = 0; i < 8u; i++) { m16.l[i] = M16(ha.l[i] == hb.l[i]); } h = fold_u16x8(h, m16);
    for (i = 0; i < 8u; i++) { m16.l[i] = M16(ha.l[i] >  hb.l[i]); } h = fold_u16x8(h, m16);
    u16x8 hm;
    for (i = 0; i < 8u; i++) { hm.l[i] = M16(ha.l[i] < hb.l[i]); }
    for (i = 0; i < 8u; i++) {
        m16.l[i] = (uint16_t)(((uint32_t)hm.l[i] & (uint32_t)ha.l[i]) | (~(uint32_t)hm.l[i] & (uint32_t)hb.l[i]));
    }
    h = fold_u16x8(h, m16);

    u8x16 ka = { { 1, 255, 128, 7, 7, 0, 200, 254, 3, 9, 11, 13, 17, 19, 23, 29 } };
    u8x16 kb = { { 2, 1, 127, 7, 8, 255, 200, 255, 3, 8, 12, 13, 18, 19, 22, 30 } };
    ka.l[0] = (uint8_t)((uint32_t)ka.l[0] + (uint32_t)e);
    kb.l[15] = (uint8_t)((uint32_t)kb.l[15] + (uint32_t)e);
    for (i = 0; i < 16u; i++) { m8.l[i] = M8(ka.l[i] <  kb.l[i]); } h = fold_u8x16(h, m8);
    for (i = 0; i < 16u; i++) { m8.l[i] = M8(ka.l[i] == kb.l[i]); } h = fold_u8x16(h, m8);
    for (i = 0; i < 16u; i++) { m8.l[i] = M8(ka.l[i] >= kb.l[i]); } h = fold_u8x16(h, m8);
    u8x16 km;
    for (i = 0; i < 16u; i++) { km.l[i] = M8(ka.l[i] < kb.l[i]); }
    for (i = 0; i < 16u; i++) {
        m8.l[i] = (uint8_t)(((uint32_t)km.l[i] & (uint32_t)ka.l[i]) | (~(uint32_t)km.l[i] & (uint32_t)kb.l[i]));
    }
    h = fold_u8x16(h, m8);
    return h;
}
