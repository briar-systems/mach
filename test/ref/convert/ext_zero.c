#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint8_t a8 = (uint8_t)((uint8_t)seed | UINT8_C(128));
    const uint8_t b8 = (uint8_t)(UINT8_C(255) - a8);
    const uint16_t a16 = (uint16_t)((uint16_t)seed | UINT16_C(32768));
    const uint16_t b16 = (uint16_t)(UINT16_C(65535) - a16);
    const uint32_t a32 = (uint32_t)((uint32_t)seed | UINT32_C(2147483648));
    const uint32_t b32 = (uint32_t)(UINT32_C(4294967295) - a32);

    h = mix_u16(h, (uint16_t)a8);
    h = mix_u16(h, (uint16_t)b8);
    h = mix_u32(h, (uint32_t)a8);
    h = mix_u32(h, (uint32_t)b8);
    h = mix_u64(h, (uint64_t)a8);
    h = mix_u64(h, (uint64_t)b8);

    h = mix_u32(h, (uint32_t)a16);
    h = mix_u32(h, (uint32_t)b16);
    h = mix_u64(h, (uint64_t)a16);
    h = mix_u64(h, (uint64_t)b16);

    h = mix_u64(h, (uint64_t)a32);
    h = mix_u64(h, (uint64_t)b32);

    h = mix_u64(h, (uint64_t)((uint64_t)a8 + (uint64_t)a16 + (uint64_t)a32));
    h = mix_u64(h, (uint64_t)((uint64_t)b8 + (uint64_t)b16 + (uint64_t)b32));

    return h;
}
