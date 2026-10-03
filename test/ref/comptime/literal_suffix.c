#include "corpus.h"

static uint32_t through_u32(uint32_t x) { return x; }
static uint64_t through_u64(uint64_t x) { return x; }
static float through_f32(float x) { return x; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    h = mix_u64(h, (uint64_t)(UINT64_C(18446744073709551615) - seed));
    h = mix_i8(h, (int8_t)(uint8_t)(UINT8_C(128) + (uint8_t)seed));
    h = mix_i64(h, (int64_t)(UINT64_C(9223372036854775807) - seed));
    h = mix_u8(h, (uint8_t)(UINT8_C(255) - (uint8_t)seed));
    h = mix_u32(h, (uint32_t)(UINT32_C(493) + s));
    h = mix_f32(h, 150.0f + (float)s);
    h = mix_f32(h, 0.1f + (float)s);
    h = mix_f64(h, 0.1 + (double)s);
    h = mix_f32(h, through_f32(0.5f));

    h = mix_u8(h, 1);
    h = mix_u8(h, 1);
    h = mix_u8(h, 1);
    h = mix_u8(h, 1);
    h = mix_u32(h, (uint32_t)(UINT32_C(3000000000) + s));
    h = mix_u32(h, through_u32((uint32_t)(UINT32_C(4000000000) + s)));
    h = mix_u32(h, (uint32_t)(UINT32_C(3999999999) + s));
    h = mix_u32(h, (uint32_t)(UINT32_C(8) + s));
    h = mix_u64(h, (uint64_t)(UINT64_C(7) + seed));

    h = mix_u32(h, (uint32_t)(UINT32_C(15) + s));
    h = mix_u32(h, (uint32_t)(UINT32_C(15) + s));

    h = mix_u64(h, (uint64_t)(through_u64(UINT64_C(18446744073709551615)) - seed));
    h = mix_u64(h, (uint64_t)(through_u64(0) + seed));
    h = mix_u64(h, (uint64_t)(UINT64_C(18446744073709551615) - seed));
    h = mix_u64(h, (uint64_t)(through_u64(UINT64_C(18446744073709551615)) - seed));
    h = mix_u64(h, (uint64_t)(UINT64_C(18446744073709551608) - seed));
    h = mix_u64(h, (uint64_t)(through_u64(UINT64_C(18446744073709551608)) - seed));
    return h;
}
