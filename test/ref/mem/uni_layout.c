#include "corpus.h"

/* reading a union member other than the one last written is defined in C, and
 * every union here has one size for every member, so the read is always fully
 * covered by the write. the byte and half-word views read the little-endian order
 * every registered mach target uses. */
typedef union {
    float f;
    uint32_t u;
    int32_t i;
    uint8_t b[4];
} W4;

typedef union {
    double d;
    uint64_t u;
    int64_t i;
    uint32_t w[2];
    uint8_t b[8];
} W8;

typedef struct { uint32_t x; uint32_t y; } Aa;
typedef struct { uint16_t p; uint16_t q; uint32_t r; } Bb;

typedef union {
    Aa a;
    Bb b;
} RU;

typedef struct { uint8_t kind; W8 val; } Tagged;

static int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint32_t safe32(uint32_t x) {
    return (uint32_t)((x & UINT32_C(2155872255)) | UINT32_C(1048576000));
}

static uint64_t safe64(uint64_t x) {
    return (uint64_t)((x & UINT64_C(9223376434903384063)) | UINT64_C(4600877379321698816));
}

static uint64_t fold_w4(uint64_t h, W4 v) {
    h = mix_u32(h, v.u);
    h = mix_i32(h, v.i);
    h = mix_f32(h, v.f);
    h = mix_u8(h, v.b[0]);
    h = mix_u8(h, v.b[1]);
    h = mix_u8(h, v.b[2]);
    h = mix_u8(h, v.b[3]);
    return h;
}

static uint64_t fold_w8(uint64_t h, W8 v) {
    h = mix_u64(h, v.u);
    h = mix_i64(h, v.i);
    h = mix_f64(h, v.d);
    h = mix_u32(h, v.w[0]);
    h = mix_u32(h, v.w[1]);
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        h = mix_u8(h, v.b[k]);
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t raw =
            (uint64_t)(UINT64_C(11400714819323198485) * (uint64_t)(i + UINT64_C(1)) + seed);

        W4 a;
        a.u = safe32((uint32_t)raw);
        h = fold_w4(h, a);

        a.i = as_i32(safe32((uint32_t)(raw >> 7)));
        h = fold_w4(h, a);

        a.f = a.f * 2.0f;
        h = fold_w4(h, a);

        W8 c;
        c.u = safe64(raw);
        h = fold_w8(h, c);

        c.i = as_i64(safe64(raw >> 9));
        h = fold_w8(h, c);

        c.d = c.d * 3.0 + 1.0;
        h = fold_w8(h, c);
    }

    RU r;
    for (uint64_t j = 0; j < UINT64_C(4); j = (uint64_t)(j + UINT64_C(1))) {
        const uint32_t raw =
            (uint32_t)(UINT32_C(2654435761) * (uint32_t)((uint32_t)j + UINT32_C(1))
                       + (uint32_t)seed);
        r.a = (Aa){ raw, (uint32_t)(raw ^ UINT32_C(4042322160)) };
        h = mix_u32(h, r.a.x);
        h = mix_u32(h, r.a.y);
        h = mix_u16(h, r.b.p);
        h = mix_u16(h, r.b.q);
        h = mix_u32(h, r.b.r);
        r.b = (Bb){ (uint16_t)raw, (uint16_t)(raw >> 16), (uint32_t)(raw + UINT32_C(1)) };
        h = mix_u32(h, r.a.x);
        h = mix_u32(h, r.a.y);
    }

    Tagged t;
    for (uint64_t n = 0; n < UINT64_C(4); n = (uint64_t)(n + UINT64_C(1))) {
        t.kind = (uint8_t)(n + UINT64_C(1));
        t.val.u = safe64((uint64_t)(UINT64_C(14695981039346656037) * (uint64_t)(n + UINT64_C(1))
                                    + seed));
        h = mix_u8(h, t.kind);
        h = fold_w8(h, t.val);
        t.val.d = t.val.d + 0.5;
        h = mix_u8(h, t.kind);
        h = fold_w8(h, t.val);
    }

    W8 us[3];
    for (uint64_t m = 0; m < UINT64_C(3); m = (uint64_t)(m + UINT64_C(1))) {
        us[m].u = safe64((uint64_t)(UINT64_C(1099511628211) * (uint64_t)(m + UINT64_C(1)) + seed));
    }
    for (uint64_t m = 0; m < UINT64_C(3); m = (uint64_t)(m + UINT64_C(1))) {
        h = fold_w8(h, us[m]);
    }
    return h;
}
