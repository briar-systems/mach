#include "corpus.h"

static uint64_t fold_pos(uint64_t h, float p, double q) {
    uint64_t a = mix_u8(h, (uint8_t)p);
    a = mix_u16(a, (uint16_t)p);
    a = mix_u32(a, (uint32_t)p);
    a = mix_u64(a, (uint64_t)p);
    a = mix_i8(a, (int8_t)p);
    a = mix_i16(a, (int16_t)p);
    a = mix_i32(a, (int32_t)p);
    a = mix_i64(a, (int64_t)p);
    a = mix_u8(a, (uint8_t)q);
    a = mix_u16(a, (uint16_t)q);
    a = mix_u32(a, (uint32_t)q);
    a = mix_u64(a, (uint64_t)q);
    a = mix_i8(a, (int8_t)q);
    a = mix_i16(a, (int16_t)q);
    a = mix_i32(a, (int32_t)q);
    a = mix_i64(a, (int64_t)q);
    return a;
}

static uint64_t fold_neg(uint64_t h, float p, double q) {
    uint64_t a = mix_i8(h, (int8_t)p);
    a = mix_i16(a, (int16_t)p);
    a = mix_i32(a, (int32_t)p);
    a = mix_i64(a, (int64_t)p);
    a = mix_i8(a, (int8_t)q);
    a = mix_i16(a, (int16_t)q);
    a = mix_i32(a, (int32_t)q);
    a = mix_i64(a, (int64_t)q);
    return a;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z32 = (float)seed;
    const double z64 = (double)seed;

    const float pos32 = 40.75f + z32;
    const double pos64 = 40.75 + z64;
    h = fold_pos(h, pos32, pos64);

    const float neg32 = -40.75f - z32;
    const double neg64 = -40.75 - z64;
    h = fold_neg(h, neg32, neg64);

    const float e1 = 127.0f + z32;
    h = mix_i8(h, (int8_t)e1);
    const float e2 = 255.0f + z32;
    h = mix_u8(h, (uint8_t)e2);
    const double e3 = 0.9999 + z64;
    h = mix_u8(h, (uint8_t)e3);
    h = mix_i8(h, (int8_t)e3);
    const double e4 = -0.9999 - z64;
    h = mix_i8(h, (int8_t)e4);

    return h;
}
