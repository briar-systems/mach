#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint64_t v64 = (uint64_t)((seed ^ UINT64_C(1311768467463790320)) | UINT64_C(1));
    const uint32_t v32 = (uint32_t)v64;
    const uint16_t v16 = (uint16_t)v64;
    const uint8_t v8 = (uint8_t)v64;

    h = mix_u32(h, v32);
    h = mix_u16(h, v16);
    h = mix_u8(h, v8);

    const uint32_t w32 = (uint32_t)(v32 ^ UINT32_C(1431655765));
    const uint16_t w16 = (uint16_t)w32;
    const uint8_t w8 = (uint8_t)w32;

    h = mix_u16(h, w16);
    h = mix_u8(h, w8);

    const uint16_t u16v = (uint16_t)(v16 ^ UINT16_C(21845));
    const uint8_t u8v = (uint8_t)u16v;
    h = mix_u8(h, u8v);

    h = mix_u64(h, (uint64_t)((uint64_t)v32 + (uint64_t)v16 + (uint64_t)v8));
    h = mix_u64(h, (uint64_t)((uint64_t)w16 + (uint64_t)w8 + (uint64_t)u8v));

    return h;
}
