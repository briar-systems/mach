#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    int8_t a8;   { uint8_t t = (uint8_t)((uint8_t)seed | 128u); memcpy(&a8, &t, 1); }
    int16_t a16; { uint16_t t = (uint16_t)((uint16_t)seed | 32768u); memcpy(&a16, &t, 2); }
    int32_t a32; { uint32_t t = (uint32_t)((uint32_t)seed | 2147483648u); memcpy(&a32, &t, 4); }
    int64_t a64; { uint64_t t = seed | UINT64_C(9223372036854775808); memcpy(&a64, &t, 8); }
    const uint8_t u8v = (uint8_t)a8;
    const uint16_t u16v = (uint16_t)a16;
    const uint32_t u32v = (uint32_t)a32;
    const uint64_t u64v = (uint64_t)a64;

    h = mix_i128(h, (corpus_i128)a8);
    h = mix_i128(h, (corpus_i128)a16);
    h = mix_i128(h, (corpus_i128)a32);
    h = mix_i128(h, (corpus_i128)a64);
    h = mix_u128(h, (corpus_u128)u8v);
    h = mix_u128(h, (corpus_u128)u16v);
    h = mix_u128(h, (corpus_u128)u32v);
    h = mix_u128(h, (corpus_u128)u64v);
    h = mix_u128(h, (corpus_u128)a64);
    h = mix_i128(h, (corpus_i128)u64v);

    const corpus_u128 w = ((corpus_u128)u64v << 64) | (corpus_u128)(seed ^ UINT64_C(0x5555555555555555));
    corpus_i128 x; memcpy(&x, &w, 16);
    h = mix_u8(h, (uint8_t)w);
    h = mix_u16(h, (uint16_t)w);
    h = mix_u32(h, (uint32_t)w);
    h = mix_u64(h, (uint64_t)w);
    h = mix_i8(h, (int8_t)x);
    h = mix_i16(h, (int16_t)x);
    h = mix_i32(h, (int32_t)x);
    h = mix_i64(h, (int64_t)x);
    h = mix_i128(h, x);
    h = mix_u128(h, (corpus_u128)x);
    h = mix_i128(h, (corpus_i128)w);
    h = mix_u128(h, (corpus_u128)x);

    h = mix_f64(h, (double)w);
    h = mix_f64(h, (double)x);
    h = mix_f64(h, (double)(w >> 3));
    h = mix_f64(h, (double)(x >> 3));
    h = mix_f64(h, (double)(((corpus_u128)1 << 100) + (corpus_u128)seed));
    h = mix_f64(h, (double)(0 - (((corpus_i128)1 << 100) + (corpus_i128)seed)));
    h = mix_f64(h, (double)(((corpus_u128)1 << 64) + 1));
    h = mix_f64(h, (double)(((corpus_u128)1 << 64) + ((corpus_u128)1 << 11) + 1));
    h = mix_f32(h, (float)(w >> 4));
    h = mix_f32(h, (float)x);

    const double big = 1.0e30 + (double)seed;
    h = mix_u128(h, (corpus_u128)big);
    h = mix_i128(h, (corpus_i128)big);
    h = mix_i128(h, (corpus_i128)(0.0 - big));
    h = mix_u128(h, (corpus_u128)18446744073709551616.0);
    h = mix_u128(h, (corpus_u128)1.5);
    h = mix_i128(h, (corpus_i128)(-1.5));
    h = mix_u128(h, (corpus_u128)(3.0e38 + (double)seed));
    h = mix_i128(h, (corpus_i128)(1.5e38 + (double)seed));
    h = mix_i128(h, (corpus_i128)(-1.5e38 - (double)seed));
    h = mix_u128(h, (corpus_u128)(float)1.0e20);
    return h;
}
