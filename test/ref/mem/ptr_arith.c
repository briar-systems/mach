#include "corpus.h"

typedef struct {
    uint16_t tag;
    uint8_t a;
    uint64_t v;
} Cell;

typedef struct {
    uint32_t pre;
    uint32_t xs[8];
    uint32_t post;
} Holder;

static uint64_t fold_cell(uint64_t h, Cell c) {
    h = mix_u16(h, c.tag);
    h = mix_u8(h, c.a);
    h = mix_u64(h, c.v);
    return h;
}

static void bump(uint32_t *p, uint64_t n) {
    for (uint64_t i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        p[i] = (uint32_t)(p[i] + (uint32_t)i + UINT32_C(1));
    }
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint32_t flat[16];
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        flat[i] = (uint32_t)(UINT64_C(2654435761) * (uint64_t)(i + UINT64_C(1)) + s);
    }

    uint32_t *const base = &flat[0];
    uint32_t *const mid = &flat[8];

    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, base[i]);
    }

    for (int64_t d = -8; d < 8; d = (int64_t)(d + 1)) {
        h = mix_u32(h, mid[d]);
    }
    h = mix_u32(h, *mid);
    h = mix_u32(h, mid[-1]);
    h = mix_u32(h, mid[7]);

    bump(mid, UINT64_C(8));
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, flat[i]);
    }

    uint32_t *const al = &flat[5];
    uint32_t *const bl = &flat[5];
    *al = (uint32_t)(*al ^ UINT32_C(4042322160));
    h = mix_u32(h, *bl);
    *bl = (uint32_t)(*bl + UINT32_C(3));
    h = mix_u32(h, *al);
    h = mix_u32(h, flat[5]);

    h = mix_u8(h, (uint8_t)(al == bl));
    h = mix_u8(h, (uint8_t)(al == base));
    h = mix_u8(h, (uint8_t)(base == NULL));
    h = mix_u8(h, (uint8_t)(mid != base));

    Cell cells[6];
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        cells[k].tag = (uint16_t)(k * UINT64_C(5) + UINT64_C(1) + s);
        cells[k].a = (uint8_t)(k + UINT64_C(100));
        cells[k].v = (uint64_t)(UINT64_C(1099511628211) * (uint64_t)(k + UINT64_C(1)) + s);
    }
    Cell *const cp = &cells[2];
    for (int64_t e = -2; e < 4; e = (int64_t)(e + 1)) {
        h = fold_cell(h, cp[e]);
    }
    cp[1].v = (uint64_t)(cp[1].v + UINT64_C(17));
    cp[-1].tag = (uint16_t)(cp[-1].tag + UINT16_C(9));
    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_cell(h, cells[k]);
    }

    uint64_t *const vp = &cells[4].v;
    *vp = (uint64_t)(*vp * UINT64_C(3));
    h = mix_u64(h, cells[4].v);
    uint16_t *const tp = &cells[0].tag;
    *tp = (uint16_t)(*tp + UINT16_C(1));
    h = mix_u16(h, cells[0].tag);

    Holder hold;
    hold.pre = (uint32_t)(UINT32_C(305419896) + (uint32_t)s);
    hold.post = (uint32_t)(UINT32_C(2596069104) + (uint32_t)s);
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        hold.xs[k] = (uint32_t)(((uint32_t)k * UINT32_C(7) + UINT32_C(3)) + (uint32_t)s);
    }
    uint32_t *const xp = &hold.xs[3];
    for (int64_t g = -3; g < 5; g = (int64_t)(g + 1)) {
        h = mix_u32(h, xp[g]);
    }
    bump(&hold.xs[0], UINT64_C(8));
    h = mix_u32(h, hold.pre);
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        h = mix_u32(h, hold.xs[k]);
    }
    h = mix_u32(h, hold.post);
    return h;
}
