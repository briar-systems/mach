#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint32_t s32 = (uint32_t)seed;
    uint32_t a = (uint32_t)(s32 | UINT32_C(1431655765));      /* 0x55555555 */
    h = mix_u32(h, a);
    a = (uint32_t)((s32 ^ UINT32_C(1431655765)) & UINT32_C(252645135));  /* 0x0f0f0f0f */
    h = mix_u32(h, a);
    a = (uint32_t)(s32 | UINT32_C(4294901760));               /* 0xffff0000 */
    h = mix_u32(h, a);
    a = (uint32_t)(s32 & UINT32_C(65535));                    /* 0x0000ffff */
    h = mix_u32(h, a);
    a = (uint32_t)(~s32);
    h = mix_u32(h, a);
    a = (uint32_t)(~(a | UINT32_C(1431655765)));
    h = mix_u32(h, a);
    a = (uint32_t)((s32 & UINT32_C(252645135)) ^ UINT32_C(4294901760));
    h = mix_u32(h, a);

    const uint64_t s64 = seed;
    uint64_t b = (uint64_t)(s64 | UINT64_C(6148914691236517205));   /* 0x5555555555555555 */
    h = mix_u64(h, b);
    b = (uint64_t)((s64 ^ UINT64_C(6148914691236517205)) & UINT64_C(1085102592571150095));  /* 0x0f0f0f0f0f0f0f0f */
    h = mix_u64(h, b);
    b = (uint64_t)(s64 | UINT64_C(18446462603027742720));     /* 0xffff0000ffff0000 */
    h = mix_u64(h, b);
    b = (uint64_t)(s64 & UINT64_C(71777214294589695));        /* 0x0000ffff0000ffff */
    h = mix_u64(h, b);
    b = (uint64_t)(~s64);
    h = mix_u64(h, b);
    b = (uint64_t)(~(b | UINT64_C(6148914691236517205)));
    h = mix_u64(h, b);
    b = (uint64_t)((s64 & UINT64_C(1085102592571150095)) ^ UINT64_C(18446462603027742720));
    h = mix_u64(h, b);

    /* signed chains carry their bit pattern in an unsigned type throughout,
     * per the two's-complement identity, and cast to signed only at the
     * point mix_iNN reads it. bitwise ops on the unsigned pattern are
     * bit-for-bit what mach's signed bitwise ops produce. */
    const uint32_t si32 = (uint32_t)seed;
    uint32_t c = (uint32_t)(si32 | UINT32_C(1431655765));
    h = mix_i32(h, (int32_t)c);
    c = (uint32_t)(~si32);
    h = mix_i32(h, (int32_t)c);
    c = (uint32_t)((si32 ^ c) & UINT32_C(252645135));
    h = mix_i32(h, (int32_t)c);

    const uint64_t si64 = seed;
    uint64_t d = (uint64_t)(si64 | UINT64_C(6148914691236517205));
    h = mix_i64(h, (int64_t)d);
    d = (uint64_t)(~si64);
    h = mix_i64(h, (int64_t)d);
    d = (uint64_t)((si64 ^ d) & UINT64_C(1085102592571150095));
    h = mix_i64(h, (int64_t)d);

    return h;
}
