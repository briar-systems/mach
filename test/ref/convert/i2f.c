#include "corpus.h"

static uint64_t fold_from(uint64_t h, uint8_t u8v, uint16_t u16v, uint32_t u32v, uint64_t u64v,
                           int8_t i8v, int16_t i16v, int32_t i32v, int64_t i64v) {
    uint64_t a = mix_f32(h, (float)u8v);
    a = mix_f64(a, (double)u8v);
    a = mix_f32(a, (float)u16v);
    a = mix_f64(a, (double)u16v);
    a = mix_f32(a, (float)u32v);
    a = mix_f64(a, (double)u32v);
    a = mix_f32(a, (float)u64v);
    a = mix_f64(a, (double)u64v);
    a = mix_f32(a, (float)i8v);
    a = mix_f64(a, (double)i8v);
    a = mix_f32(a, (float)i16v);
    a = mix_f64(a, (double)i16v);
    a = mix_f32(a, (float)i32v);
    a = mix_f64(a, (double)i32v);
    a = mix_f32(a, (float)i64v);
    a = mix_f64(a, (double)i64v);
    return a;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint8_t u8v = (uint8_t)((uint8_t)seed | UINT8_C(128));
    const int8_t i8v = (int8_t)u8v;
    const uint16_t u16v = (uint16_t)((uint16_t)seed | UINT16_C(32768));
    const int16_t i16v = (int16_t)u16v;
    const uint32_t u32v = (uint32_t)((uint32_t)seed | UINT32_C(2147483648));
    const int32_t i32v = (int32_t)u32v;
    const uint64_t u64v = (uint64_t)(seed | UINT64_C(9223372036854775808));
    const int64_t i64v = (int64_t)u64v;

    h = fold_from(h, u8v, u16v, u32v, u64v, i8v, i16v, i32v, i64v);

    const uint64_t small_u64 = (uint64_t)(seed | UINT64_C(1));
    const int64_t small_i64 = (int64_t)(0 - (int64_t)small_u64);
    h = mix_f32(h, (float)small_u64);
    h = mix_f64(h, (double)small_u64);
    h = mix_f32(h, (float)small_i64);
    h = mix_f64(h, (double)small_i64);

    return h;
}
