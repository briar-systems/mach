#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint32_t s32 = (uint32_t)seed;
    uint32_t a = (uint32_t)(s32 + UINT32_C(2047));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(2048));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(4095));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(4096));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(2147483647));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(2147483648));
    h = mix_u32(h, a);
    a = (uint32_t)(a + UINT32_C(4294967295));
    h = mix_u32(h, a);

    const uint64_t s64 = seed;
    uint64_t b = (uint64_t)(s64 + UINT64_C(2047));
    h = mix_u64(h, b);
    b = (uint64_t)(b - UINT64_C(2048));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(16773120));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(16777215));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(2147483647));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(2147483648));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(4294967295));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(4294967296));
    h = mix_u64(h, b);
    b = (uint64_t)(b + UINT64_C(9223372036854775808));
    h = mix_u64(h, b);

    /* signed chains carry their two's-complement bit pattern in an unsigned
     * type throughout, so every add/sub is well-defined unsigned wraparound,
     * and cast to the signed width only at the point mix_iNN reads it. */
    const uint32_t si32 = (uint32_t)seed;
    uint32_t c = (uint32_t)(si32 + UINT32_C(2047));
    h = mix_i32(h, (int32_t)c);
    c = (uint32_t)(c - UINT32_C(2048));
    h = mix_i32(h, (int32_t)c);
    c = (uint32_t)(c + UINT32_C(2147483647));
    h = mix_i32(h, (int32_t)c);
    c = (uint32_t)(c - UINT32_C(2147483648));
    h = mix_i32(h, (int32_t)c);

    const uint64_t si64 = seed;
    uint64_t d = (uint64_t)(si64 + UINT64_C(2047));
    h = mix_i64(h, (int64_t)d);
    d = (uint64_t)(d - UINT64_C(2048));
    h = mix_i64(h, (int64_t)d);
    d = (uint64_t)(d + UINT64_C(2147483647));
    h = mix_i64(h, (int64_t)d);
    d = (uint64_t)(d - UINT64_C(9223372036854775808));
    h = mix_i64(h, (int64_t)d);

    return h;
}
