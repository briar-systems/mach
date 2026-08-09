#include "corpus.h"

typedef struct {
    uint16_t tag;
    uint8_t a;
    uint64_t v;
} Cell;

typedef struct {
    Cell head;
    uint64_t body[4];
    uint8_t tail;
} Nest;

static int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint8_t z_u8;
static uint16_t z_u16;
static uint32_t z_u32;
static uint64_t z_u64;
static int32_t z_i32;
static int64_t z_i64;
static float z_f32;
static double z_f64;
static uint64_t z_explicit = 0;
static uint32_t z_arr[512];
static Cell z_cells[8];
static Nest z_nest;

static uint64_t fold_cell(uint64_t h, Cell c) {
    h = mix_u16(h, c.tag);
    h = mix_u8(h, c.a);
    h = mix_u64(h, c.v);
    return h;
}

static uint64_t fold_zeros(uint64_t h) {
    h = mix_u8(h, z_u8);
    h = mix_u16(h, z_u16);
    h = mix_u32(h, z_u32);
    h = mix_u64(h, z_u64);
    h = mix_i32(h, z_i32);
    h = mix_i64(h, z_i64);
    h = mix_f32(h, z_f32);
    h = mix_f64(h, z_f64);
    h = mix_u64(h, z_explicit);
    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, z_arr[i]);
    }
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        h = fold_cell(h, z_cells[i]);
    }
    h = fold_cell(h, z_nest.head);
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u64(h, z_nest.body[i]);
    }
    h = mix_u8(h, z_nest.tail);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    h = fold_zeros(h);

    z_u8 = (uint8_t)(UINT8_C(200) + (uint8_t)seed);
    z_u16 = (uint16_t)(UINT16_C(40000) + (uint16_t)seed);
    z_u32 = (uint32_t)(UINT32_C(3000000000) + (uint32_t)seed);
    z_u64 = (uint64_t)(UINT64_C(18364758544493064720) + seed);
    z_i32 = as_i32((uint32_t)(UINT32_C(2000000000) + (uint32_t)seed));
    z_i64 = as_i64((uint64_t)(UINT64_C(9000000000000000000) + seed));
    z_f32 = 1.75f;
    z_f64 = -0.0;
    z_explicit = (uint64_t)(UINT64_C(1099511628211) + seed);

    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        z_arr[i] = (uint32_t)(UINT64_C(2654435761) * (uint64_t)(i + UINT64_C(1)) + seed);
    }
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        z_cells[i].tag = (uint16_t)(i * UINT64_C(11) + UINT64_C(3) + seed);
        z_cells[i].a = (uint8_t)(i + UINT64_C(250));
        z_cells[i].v =
            (uint64_t)(UINT64_C(11400714819323198485) * (uint64_t)(i + UINT64_C(1)) + seed);
    }
    z_nest.head = z_cells[3];
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        z_nest.body[i] = (uint64_t)(UINT64_C(305419896) * (uint64_t)(i + UINT64_C(1)) + seed);
    }
    z_nest.tail = UINT8_C(77);
    h = fold_zeros(h);

    h = mix_u32(h, z_arr[511]);
    h = mix_u32(h, z_arr[256]);
    h = mix_u32(h, z_arr[(seed + UINT64_C(500)) % UINT64_C(512)]);

    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        z_arr[i] = 0;
    }
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        z_cells[i] = (Cell){ 0, 0, 0 };
    }
    z_u8 = 0;
    z_u16 = 0;
    z_u32 = 0;
    z_u64 = 0;
    z_i32 = 0;
    z_i64 = 0;
    z_f32 = 0.0f;
    z_f64 = 0.0;
    z_explicit = 0;
    z_nest.head = (Cell){ 0, 0, 0 };
    for (uint64_t i = 0; i < UINT64_C(4); i = (uint64_t)(i + UINT64_C(1))) {
        z_nest.body[i] = 0;
    }
    z_nest.tail = 0;
    h = fold_zeros(h);
    return h;
}
