#include "corpus.h"

/* mach's vectors are lane-derived and packed, so a struct of N floats holds the
 * same lanes in the same order. these are argument values only, never placed in a
 * record or an array, so the reference needs lane fidelity and nothing else. */
typedef struct { float l[3]; } f32x3;
typedef struct { float l[4]; } f32x4;
typedef struct { float l[2]; } f32x2;

static inline int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }
static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static float as_f32(uint64_t v) {
    return ((float)(uint32_t)(v & UINT64_C(4095)) - 2048.0f) / 8.0f;
}

static double as_f64(uint64_t v) {
    return ((double)(uint32_t)(v & UINT64_C(1048575)) - 524288.0) / 64.0;
}

static uint64_t mixed(uint64_t a0, float x0, uint32_t a1, double y0, f32x3 v0,
                      int64_t a2, float x1, uint16_t a3, f32x4 v1, double y1,
                      uint64_t a4, float x2, uint8_t a5, double y2, f32x2 v2,
                      uint64_t a6, float x3, int32_t a7, double y3, uint64_t a8) {
    uint64_t h = fold_init();
    h = mix_u64(h, a0);
    h = mix_f32(h, x0);
    h = mix_u32(h, a1);
    h = mix_f64(h, y0);
    h = mix_f32(h, v0.l[0]);
    h = mix_f32(h, v0.l[1]);
    h = mix_f32(h, v0.l[2]);
    h = mix_i64(h, a2);
    h = mix_f32(h, x1);
    h = mix_u16(h, a3);
    h = mix_f32(h, v1.l[0]);
    h = mix_f32(h, v1.l[1]);
    h = mix_f32(h, v1.l[2]);
    h = mix_f32(h, v1.l[3]);
    h = mix_f64(h, y1);
    h = mix_u64(h, a4);
    h = mix_f32(h, x2);
    h = mix_u8(h, a5);
    h = mix_f64(h, y2);
    h = mix_f32(h, v2.l[0]);
    h = mix_f32(h, v2.l[1]);
    h = mix_u64(h, a6);
    h = mix_f32(h, x3);
    h = mix_i32(h, a7);
    h = mix_f64(h, y3);
    h = mix_u64(h, a8);

    h = mix_u64(h, (uint64_t)(a0 + a4 * a6 - a8));
    h = mix_f32(h, x0 + x1 * x2 - x3);
    h = mix_f64(h, y0 + y1 * y2 - y3);
    f32x3 w;
    w.l[0] = v0.l[0] + v1.l[0];
    w.l[1] = v0.l[1] + v1.l[1];
    w.l[2] = v0.l[2] + v2.l[0];
    h = mix_f32(h, w.l[0]);
    h = mix_f32(h, w.l[1]);
    h = mix_f32(h, w.l[2]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[24];
    for (uint64_t i = 0; i < UINT64_C(24); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    uint64_t r = 0;
    for (uint64_t pass = 0; pass < UINT64_C(3); pass = (uint64_t)(pass + UINT64_C(1))) {
        const uint64_t d = (uint64_t)(pass * UINT64_C(11) + seed);

        const f32x3 q0 = { { as_f32(v[16]), as_f32(v[17]), as_f32(v[18]) } };
        const f32x4 q1 = { { as_f32(v[19]), as_f32(v[20]), as_f32(v[21]), as_f32(v[22]) } };
        const f32x2 q2 = { { as_f32(v[23]), as_f32((uint64_t)(v[0] + d)) } };

        r = mixed((uint64_t)(v[0] + d), as_f32(v[1]), (uint32_t)v[2], as_f64(v[3]), q0,
                  as_i64(v[4]), as_f32(v[5]), (uint16_t)v[6], q1, as_f64(v[7]),
                  v[8], as_f32(v[9]), (uint8_t)v[10], as_f64(v[11]), q2,
                  v[12], as_f32(v[13]), as_i32((uint32_t)v[14]), as_f64(v[15]), (uint64_t)(v[1] + d));
        h = mix_u64(h, r);
    }

    const f32x3 e0 = { { as_f32(r), as_f32(v[2]), as_f32(v[3]) } };
    const f32x4 e1 = { { as_f32(v[4]), as_f32(v[5]), as_f32(v[6]), as_f32(v[7]) } };
    const f32x2 e2 = { { as_f32(v[8]), as_f32(v[9]) } };
    h = mix_u64(h, mixed(
        mixed(v[0], as_f32(v[1]), (uint32_t)v[2], as_f64(v[3]), e0,
              as_i64(v[4]), as_f32(v[5]), (uint16_t)v[6], e1, as_f64(v[7]),
              v[8], as_f32(v[9]), (uint8_t)v[10], as_f64(v[11]), e2,
              v[12], as_f32(v[13]), as_i32((uint32_t)v[14]), as_f64(v[15]), v[1]),
        as_f32(v[3]), (uint32_t)v[5], as_f64(v[7]), e0,
        as_i64(v[9]), as_f32(v[11]), (uint16_t)v[13], e1, as_f64(v[15]),
        v[17], as_f32(v[19]), (uint8_t)v[21], as_f64(v[23]), e2,
        v[2], as_f32(v[4]), as_i32((uint32_t)v[6]), as_f64(v[8]), r));

    return h;
}
