#include "corpus.h"
#include <stddef.h>

typedef struct {
    uint16_t tag;
    uint8_t a;
    uint64_t v;
} Cell;

typedef struct {
    uint8_t pre;
    uint32_t xs[7];
    uint16_t post;
} Holder;

static uint64_t fold_cell(uint64_t h, Cell c) {
    h = mix_u16(h, c.tag);
    h = mix_u8(h, c.a);
    h = mix_u64(h, c.v);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint32_t flat[32];
    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        flat[i] = (uint32_t)(UINT64_C(2654435761) * (uint64_t)(i + UINT64_C(1)) + s);
    }

    h = mix_u32(h, flat[0]);
    h = mix_u32(h, flat[1]);
    h = mix_u32(h, flat[15]);
    h = mix_u32(h, flat[31]);

    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, flat[(uint64_t)(i * UINT64_C(7) + s) & UINT64_C(31)]);
    }
    for (uint64_t i = 32; i > 0;) {
        i = (uint64_t)(i - UINT64_C(1));
        h = mix_u32(h, flat[i]);
    }

    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(5))) {
        h = mix_u32(h, flat[i]);
    }

    uint16_t grid[4][6];
    for (uint64_t r = 0; r < UINT64_C(4); r = (uint64_t)(r + UINT64_C(1))) {
        for (uint64_t c = 0; c < UINT64_C(6); c = (uint64_t)(c + UINT64_C(1))) {
            grid[r][c] = (uint16_t)(r * UINT64_C(100) + c * UINT64_C(7) + s);
        }
    }
    h = mix_u16(h, grid[0][0]);
    h = mix_u16(h, grid[3][5]);
    h = mix_u16(h, grid[(s + UINT64_C(2)) & UINT64_C(3)][(s + UINT64_C(4)) & UINT64_C(3)]);
    for (uint64_t r = 0; r < UINT64_C(4); r = (uint64_t)(r + UINT64_C(1))) {
        for (uint64_t c = 0; c < UINT64_C(6); c = (uint64_t)(c + UINT64_C(1))) {
            h = mix_u16(h, grid[r][c]);
        }
    }
    for (uint64_t c2 = 0; c2 < UINT64_C(6); c2 = (uint64_t)(c2 + UINT64_C(1))) {
        for (uint64_t r = 0; r < UINT64_C(4); r = (uint64_t)(r + UINT64_C(1))) {
            h = mix_u16(h, grid[r][c2]);
        }
    }

    uint8_t cube[3][3][3];
    for (uint64_t x = 0; x < UINT64_C(3); x = (uint64_t)(x + UINT64_C(1))) {
        for (uint64_t y = 0; y < UINT64_C(3); y = (uint64_t)(y + UINT64_C(1))) {
            for (uint64_t z = 0; z < UINT64_C(3); z = (uint64_t)(z + UINT64_C(1))) {
                cube[x][y][z] =
                    (uint8_t)(x * UINT64_C(9) + y * UINT64_C(3) + z + s);
            }
        }
    }
    for (uint64_t x = 0; x < UINT64_C(3); x = (uint64_t)(x + UINT64_C(1))) {
        for (uint64_t y = 0; y < UINT64_C(3); y = (uint64_t)(y + UINT64_C(1))) {
            for (uint64_t z = 0; z < UINT64_C(3); z = (uint64_t)(z + UINT64_C(1))) {
                h = mix_u8(h, cube[z][y][x]);
            }
        }
    }
    h = mix_u8(h, cube[2][0][1]);
    h = mix_u8(h, cube[(s + UINT64_C(1)) % UINT64_C(3)][(s + UINT64_C(2)) % UINT64_C(3)]
                     [s % UINT64_C(3)]);

    Cell cells[5];
    for (uint64_t k = 0; k < UINT64_C(5); k = (uint64_t)(k + UINT64_C(1))) {
        cells[k].tag = (uint16_t)(k * UINT64_C(3) + UINT64_C(1) + s);
        cells[k].a = (uint8_t)(k + UINT64_C(200));
        cells[k].v = (uint64_t)(UINT64_C(1099511628211) * (uint64_t)(k + UINT64_C(1)) + s);
    }
    for (uint64_t k = 0; k < UINT64_C(5); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_cell(h, cells[k]);
    }
    h = fold_cell(h, cells[(s + UINT64_C(3)) % UINT64_C(5)]);
    h = mix_u64(h, cells[4].v);
    h = mix_u16(h, cells[(s + UINT64_C(1)) % UINT64_C(5)].tag);
    h = mix_u64(h, (uint64_t)sizeof(Cell));

    Holder hold;
    hold.pre = UINT8_C(17);
    hold.post = UINT16_C(4660);
    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        hold.xs[k] = (uint32_t)(((uint32_t)k * UINT32_C(305419896)) + (uint32_t)s);
    }
    h = mix_u8(h, hold.pre);
    for (uint64_t k = 0; k < UINT64_C(7); k = (uint64_t)(k + UINT64_C(1))) {
        h = mix_u32(h, hold.xs[(uint64_t)(k * UINT64_C(3) + s) % UINT64_C(7)]);
    }
    h = mix_u16(h, hold.post);
    h = mix_u64(h, (uint64_t)offsetof(Holder, xs));

    Holder hold2 = hold;
    hold2.xs[3] = (uint32_t)(hold2.xs[3] ^ UINT32_C(4042322160));
    h = mix_u32(h, hold.xs[3]);
    h = mix_u32(h, hold2.xs[3]);
    h = mix_u8(h, hold2.pre);
    h = mix_u16(h, hold2.post);
    return h;
}
