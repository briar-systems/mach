#include "corpus.h"

typedef struct {
    uint16_t tag;
    uint8_t a;
    uint64_t v;
} Cell;

typedef struct {
    Cell left;
    uint16_t right[3];
    uint32_t mark;
} Pair;

static int8_t as_i8(uint8_t v) { int8_t r; memcpy(&r, &v, sizeof r); return r; }
static int16_t as_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }
static int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint8_t bits_u8(int8_t v) { uint8_t r; memcpy(&r, &v, sizeof r); return r; }
static uint16_t bits_u16(int16_t v) { uint16_t r; memcpy(&r, &v, sizeof r); return r; }
static uint32_t bits_u32(int32_t v) { uint32_t r; memcpy(&r, &v, sizeof r); return r; }
static uint64_t bits_u64(int64_t v) { uint64_t r; memcpy(&r, &v, sizeof r); return r; }

#define K_U64 UINT64_C(11400714819323198485)
static const int32_t K_I32 = -1234567;
static const double K_F64 = 0.15625;

static uint8_t g_u8 = UINT8_C(219);
static uint16_t g_u16 = UINT16_C(48879);
static uint32_t g_u32 = UINT32_C(3735928559);
static uint64_t g_u64 = UINT64_C(18364758544493064720);
static int8_t g_i8 = -100;
static int16_t g_i16 = -30000;
static int32_t g_i32 = -2000000000;
static int64_t g_i64 = INT64_C(-4000000000000000000);
static float g_f32 = 1.5f;
static double g_f64 = -2.25;
static uint32_t g_arr[6] = {10, 20, 30, 40, 50, 60};
static Cell g_cell = { UINT16_C(4660), UINT8_C(200), UINT64_C(1099511628211) };
static Pair g_pair = { { UINT16_C(1), UINT8_C(2), UINT64_C(3) },
                       { UINT16_C(7), UINT16_C(8), UINT16_C(9) },
                       UINT32_C(305419896) };

static uint64_t fold_cell(uint64_t h, Cell c) {
    h = mix_u16(h, c.tag);
    h = mix_u8(h, c.a);
    h = mix_u64(h, c.v);
    return h;
}

static uint64_t fold_globals(uint64_t h) {
    h = mix_u8(h, g_u8);
    h = mix_u16(h, g_u16);
    h = mix_u32(h, g_u32);
    h = mix_u64(h, g_u64);
    h = mix_i8(h, g_i8);
    h = mix_i16(h, g_i16);
    h = mix_i32(h, g_i32);
    h = mix_i64(h, g_i64);
    h = mix_f32(h, g_f32);
    h = mix_f64(h, g_f64);
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, g_arr[i]);
    }
    h = fold_cell(h, g_cell);
    h = fold_cell(h, g_pair.left);
    h = mix_u16(h, g_pair.right[0]);
    h = mix_u16(h, g_pair.right[1]);
    h = mix_u16(h, g_pair.right[2]);
    h = mix_u32(h, g_pair.mark);
    return h;
}

static void advance(uint64_t step) {
    g_u8 = (uint8_t)(g_u8 + (uint8_t)step);
    g_u16 = (uint16_t)(g_u16 + (uint16_t)step);
    g_u32 = (uint32_t)(g_u32 + (uint32_t)step);
    g_u64 = (uint64_t)(g_u64 + step);
    g_i8 = as_i8((uint8_t)(bits_u8(g_i8) + (uint8_t)step));
    g_i16 = as_i16((uint16_t)(bits_u16(g_i16) + (uint16_t)step));
    g_i32 = as_i32((uint32_t)(bits_u32(g_i32) + (uint32_t)step));
    g_i64 = as_i64((uint64_t)(bits_u64(g_i64) + step));
    g_f32 = g_f32 * 1.5f;
    g_f64 = g_f64 * 0.5 + 1.0;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        g_arr[i] = (uint32_t)(g_arr[i] ^ (uint32_t)((uint32_t)step * ((uint32_t)i + UINT32_C(1))));
    }
    g_cell.tag = (uint16_t)(g_cell.tag + (uint16_t)step);
    g_cell.a = (uint8_t)(g_cell.a ^ (uint8_t)step);
    g_cell.v = (uint64_t)(g_cell.v * UINT64_C(3) + step);
    g_pair.left = g_cell;
    g_pair.right[step % UINT64_C(3)] =
        (uint16_t)(g_pair.right[step % UINT64_C(3)] + UINT16_C(100));
    g_pair.mark = (uint32_t)(g_pair.mark - (uint32_t)step);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    h = fold_globals(h);

    h = mix_u64(h, (uint64_t)(K_U64 + seed));
    h = mix_i32(h, K_I32);
    h = mix_f64(h, K_F64);

    for (uint64_t n = 0; n < UINT64_C(5); n = (uint64_t)(n + UINT64_C(1))) {
        advance((uint64_t)(n + UINT64_C(1) + seed));
        h = fold_globals(h);
    }

    uint64_t t = g_u64;
    t = (uint64_t)(t ^ K_U64);
    g_u64 = t;
    h = mix_u64(h, g_u64);

    Cell c = g_cell;
    c.v = (uint64_t)(c.v + UINT64_C(1));
    g_cell = c;
    h = fold_cell(h, g_cell);
    h = fold_globals(h);
    return h;
}
