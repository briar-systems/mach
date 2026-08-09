#include "corpus.h"

typedef struct {
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b3;
    uint16_t h0;
    uint16_t h1;
    uint32_t w0;
    uint64_t d0;
} Packed;

typedef struct {
    int8_t a;
    int16_t b;
    int32_t c;
    int64_t d;
} Signed;

/* a bit pattern read as the signed type of the same width. memcpy rather than a
 * conversion, because converting an out-of-range unsigned value to a signed type
 * is not a two's-complement reinterpret in C. */
static int8_t as_i8(uint8_t v) { int8_t r; memcpy(&r, &v, sizeof r); return r; }
static int16_t as_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }
static int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint8_t bits_u8(int8_t v) { uint8_t r; memcpy(&r, &v, sizeof r); return r; }
static uint16_t bits_u16(int16_t v) { uint16_t r; memcpy(&r, &v, sizeof r); return r; }
static uint32_t bits_u32(int32_t v) { uint32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t fold_packed(uint64_t h, Packed p) {
    h = mix_u8(h, p.b0);
    h = mix_u8(h, p.b1);
    h = mix_u8(h, p.b2);
    h = mix_u8(h, p.b3);
    h = mix_u16(h, p.h0);
    h = mix_u16(h, p.h1);
    h = mix_u32(h, p.w0);
    h = mix_u64(h, p.d0);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    Packed p = {0, 0, 0, 0, 0, 0, 0, 0};
    h = fold_packed(h, p);

    const uint64_t wide = (uint64_t)(UINT64_C(18364758544493064720) + s);
    p.b0 = (uint8_t)wide;
    p.b1 = (uint8_t)(wide >> 8);
    p.b2 = (uint8_t)(wide >> 16);
    p.b3 = (uint8_t)(wide >> 24);
    p.h0 = (uint16_t)wide;
    p.h1 = (uint16_t)(wide >> 32);
    p.w0 = (uint32_t)wide;
    p.d0 = wide;
    h = fold_packed(h, p);

    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(wide * (uint64_t)(i + UINT64_C(1)) + s);
        p.b1 = (uint8_t)v;
        p.h1 = (uint16_t)(v >> 3);
        p.w0 = (uint32_t)(v >> 5);
        h = fold_packed(h, p);
    }

    Signed q = {0, 0, 0, 0};
    for (uint64_t j = 0; j < UINT64_C(6); j = (uint64_t)(j + UINT64_C(1))) {
        const uint64_t bits =
            (uint64_t)(UINT64_C(11400714819323198485) * (uint64_t)(j + UINT64_C(1)) + s);
        q.a = as_i8((uint8_t)bits);
        q.b = as_i16((uint16_t)(bits >> 8));
        q.c = as_i32((uint32_t)(bits >> 16));
        q.d = as_i64(bits);
        h = mix_i8(h, q.a);
        h = mix_i16(h, q.b);
        h = mix_i32(h, q.c);
        h = mix_i64(h, q.d);
        h = mix_i64(h, (int64_t)q.a);
        h = mix_i64(h, (int64_t)q.b);
        h = mix_i64(h, (int64_t)q.c);
        h = mix_u64(h, (uint64_t)bits_u8(q.a));
        h = mix_u64(h, (uint64_t)bits_u16(q.b));
        h = mix_u64(h, (uint64_t)bits_u32(q.c));
    }

    uint8_t bytes[16];
    for (uint64_t k = 0; k < UINT64_C(16); k = (uint64_t)(k + UINT64_C(1))) {
        bytes[k] = (uint8_t)((uint8_t)(wide >> ((k & UINT64_C(7)) * UINT64_C(8)))
                             + (uint8_t)k);
    }
    for (uint64_t k = 0; k < UINT64_C(16); k = (uint64_t)(k + UINT64_C(1))) {
        h = mix_u8(h, bytes[k]);
    }

    uint64_t x = wide;
    uint64_t *xp = &x;
    *xp = (uint64_t)(*xp ^ UINT64_C(11400714819323198485));
    h = mix_u64(h, x);
    x = (uint64_t)(x + UINT64_C(1));
    h = mix_u64(h, *xp);
    return h;
}
