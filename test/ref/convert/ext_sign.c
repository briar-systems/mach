#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint8_t ua8 = (uint8_t)((uint8_t)seed | UINT8_C(128));
    const int8_t a8 = (int8_t)ua8;
    const int8_t b8 = (int8_t)(uint8_t)(~ua8);

    const uint16_t ua16 = (uint16_t)((uint16_t)seed | UINT16_C(32768));
    const int16_t a16 = (int16_t)ua16;
    const int16_t b16 = (int16_t)(uint16_t)(~ua16);

    const uint32_t ua32 = (uint32_t)((uint32_t)seed | UINT32_C(2147483648));
    const int32_t a32 = (int32_t)ua32;
    const int32_t b32 = (int32_t)(uint32_t)(~ua32);

    h = mix_i16(h, (int16_t)a8);
    h = mix_i16(h, (int16_t)b8);
    h = mix_i32(h, (int32_t)a8);
    h = mix_i32(h, (int32_t)b8);
    h = mix_i64(h, (int64_t)a8);
    h = mix_i64(h, (int64_t)b8);

    h = mix_i32(h, (int32_t)a16);
    h = mix_i32(h, (int32_t)b16);
    h = mix_i64(h, (int64_t)a16);
    h = mix_i64(h, (int64_t)b16);

    h = mix_i64(h, (int64_t)a32);
    h = mix_i64(h, (int64_t)b32);

    h = mix_i64(h, (int64_t)((int64_t)a8 + (int64_t)a16 + (int64_t)a32));
    h = mix_i64(h, (int64_t)((int64_t)b8 + (int64_t)b16 + (int64_t)b32));

    return h;
}
