#include "corpus.h"

static uint64_t f32_u32(uint64_t h, float p) { return mix_u32(h, (uint32_t)p); }
static uint64_t f64_u32(uint64_t h, double q) { return mix_u32(h, (uint32_t)q); }
static uint64_t f32_u64(uint64_t h, float p) { return mix_u64(h, (uint64_t)p); }
static uint64_t f64_u64(uint64_t h, double q) { return mix_u64(h, (uint64_t)q); }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z32 = (float)seed;
    const double z64 = (double)seed;

    h = f32_u32(h, 2147483520.0f + z32);
    h = f32_u32(h, 2147483648.0f + z32);
    h = f32_u32(h, 2147483904.0f + z32);
    h = f32_u32(h, 3221225472.0f + z32);
    h = f32_u32(h, 4294967040.0f + z32);

    h = f64_u32(h, 2147483647.0 + z64);
    h = f64_u32(h, 2147483648.0 + z64);
    h = f64_u32(h, 2147483649.0 + z64);
    h = f64_u32(h, 3000000000.0 + z64);
    h = f64_u32(h, 4294967295.0 + z64);

    h = f32_u64(h, 4611686018427387904.0f + z32);
    h = f32_u64(h, 9223372036854775808.0f + z32);
    h = f32_u64(h, 9223373136366403584.0f + z32);
    h = f32_u64(h, 13835058055282163712.0f + z32);
    h = f32_u64(h, 18446742974197923840.0f + z32);

    h = f64_u64(h, 9223372036854774784.0 + z64);
    h = f64_u64(h, 9223372036854775808.0 + z64);
    h = f64_u64(h, 9223372036854777856.0 + z64);
    h = f64_u64(h, 13835058055282163712.0 + z64);
    h = f64_u64(h, 18446744073709549568.0 + z64);

    return h;
}
