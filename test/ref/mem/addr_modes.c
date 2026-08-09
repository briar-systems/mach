#include "corpus.h"
#include <stddef.h>

typedef struct {
    uint16_t tag;
    uint8_t a;
    uint64_t v;
} Cell;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} Odd;

typedef struct {
    uint64_t lead;
    uint64_t pad[1024];
    uint32_t mid;
    uint32_t pad2[512];
    uint16_t tail;
} Far;

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint8_t a8[64];
    uint16_t a16[64];
    uint32_t a32[64];
    uint64_t a64[64];
    Cell ac[64];
    Odd ao[64];

    for (uint64_t i = 0; i < UINT64_C(64); i = (uint64_t)(i + UINT64_C(1))) {
        a8[i] = (uint8_t)(i * UINT64_C(3) + UINT64_C(1) + s);
        a16[i] = (uint16_t)(i * UINT64_C(1103) + UINT64_C(7) + s);
        a32[i] = (uint32_t)(i * UINT64_C(2654435761) + UINT64_C(11) + s);
        a64[i] = (uint64_t)(UINT64_C(11400714819323198485) * (uint64_t)(i + UINT64_C(1)) + s);
        ac[i].tag = (uint16_t)(i * UINT64_C(5) + UINT64_C(2) + s);
        ac[i].a = (uint8_t)(i + UINT64_C(128));
        ac[i].v = (uint64_t)(UINT64_C(1099511628211) * (uint64_t)(i + UINT64_C(1)) + s);
        ao[i].x = (uint32_t)(i * UINT64_C(17) + UINT64_C(1) + s);
        ao[i].y = (uint32_t)(i * UINT64_C(17) + UINT64_C(2) + s);
        ao[i].z = (uint32_t)(i * UINT64_C(17) + UINT64_C(3) + s);
    }

    for (uint64_t i = 0; i < UINT64_C(64); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t k = (uint64_t)(i * UINT64_C(13) + s) & UINT64_C(63);
        h = mix_u8(h, a8[k]);
        h = mix_u16(h, a16[k]);
        h = mix_u32(h, a32[k]);
        h = mix_u64(h, a64[k]);
        h = mix_u16(h, ac[k].tag);
        h = mix_u8(h, ac[k].a);
        h = mix_u64(h, ac[k].v);
        h = mix_u32(h, ao[k].x);
        h = mix_u32(h, ao[k].y);
        h = mix_u32(h, ao[k].z);
    }

    for (uint64_t i = 0; i < UINT64_C(32); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, a32[i * UINT64_C(2)]);
        h = mix_u32(h, a32[i << 1]);
        h = mix_u64(h, a64[(uint64_t)(i + UINT64_C(1)) * UINT64_C(2) - UINT64_C(2)]);
        h = mix_u8(h, a8[UINT64_C(63) - i]);
        h = mix_u64(h, ac[(uint64_t)(i * UINT64_C(2) + s) & UINT64_C(63)].v);
    }

    uint32_t grid[8][8];
    for (uint64_t r = 0; r < UINT64_C(8); r = (uint64_t)(r + UINT64_C(1))) {
        for (uint64_t c = 0; c < UINT64_C(8); c = (uint64_t)(c + UINT64_C(1))) {
            grid[r][c] = (uint32_t)(r * UINT64_C(71) + c * UINT64_C(13) + s);
        }
    }
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, grid[i][3]);
        h = mix_u32(h, grid[3][i]);
        h = mix_u32(h, grid[i][(uint64_t)(i + s) & UINT64_C(7)]);
    }
    h = mix_u32(h, grid[7][7]);
    h = mix_u32(h, grid[0][0]);

    Far f;
    f.lead = (uint64_t)(UINT64_C(305419896) + s);
    f.mid = (uint32_t)(UINT32_C(2596069104) + (uint32_t)s);
    f.tail = (uint16_t)(UINT16_C(4660) + (uint16_t)s);
    for (uint64_t i = 0; i < UINT64_C(1024); i = (uint64_t)(i + UINT64_C(1))) {
        f.pad[i] = (uint64_t)(i * UINT64_C(31) + s);
    }
    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        f.pad2[i] = (uint32_t)(i * UINT64_C(37) + s);
    }
    h = mix_u64(h, f.lead);
    h = mix_u64(h, f.pad[0]);
    h = mix_u64(h, f.pad[1023]);
    h = mix_u64(h, f.pad[(s + UINT64_C(700)) % UINT64_C(1024)]);
    h = mix_u32(h, f.mid);
    h = mix_u32(h, f.pad2[0]);
    h = mix_u32(h, f.pad2[511]);
    h = mix_u32(h, f.pad2[(s + UINT64_C(300)) % UINT64_C(512)]);
    h = mix_u16(h, f.tail);
    h = mix_u64(h, (uint64_t)offsetof(Far, mid));
    h = mix_u64(h, (uint64_t)offsetof(Far, pad2));
    h = mix_u64(h, (uint64_t)offsetof(Far, tail));
    h = mix_u64(h, (uint64_t)sizeof(Far));

    uint64_t acc = 0;
    for (uint64_t i = 0; i < UINT64_C(1024); i = (uint64_t)(i + UINT64_C(1))) {
        acc = (uint64_t)(acc ^ (uint64_t)(f.pad[i] * (uint64_t)(i + UINT64_C(1))));
    }
    h = mix_u64(h, acc);
    acc = 0;
    for (uint64_t i = 0; i < UINT64_C(512); i = (uint64_t)(i + UINT64_C(1))) {
        acc = (uint64_t)(acc + (uint64_t)((uint64_t)f.pad2[i] * (uint64_t)(i + UINT64_C(3))));
    }
    h = mix_u64(h, acc);
    return h;
}
