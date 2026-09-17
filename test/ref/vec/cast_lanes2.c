#include <math.h>
#include "corpus.h"

static uint64_t fold_i8(uint64_t h, const int8_t *v) {
    uint64_t a = h;
    a = mix_i8(a, v[0]);
    a = mix_i8(a, v[1]);
    return a;
}

static uint64_t fold_u8(uint64_t h, const uint8_t *v) {
    uint64_t a = h;
    a = mix_u8(a, v[0]);
    a = mix_u8(a, v[1]);
    return a;
}

static uint64_t fold_i16(uint64_t h, const int16_t *v) {
    uint64_t a = h;
    a = mix_i16(a, v[0]);
    a = mix_i16(a, v[1]);
    return a;
}

static uint64_t fold_u16(uint64_t h, const uint16_t *v) {
    uint64_t a = h;
    a = mix_u16(a, v[0]);
    a = mix_u16(a, v[1]);
    return a;
}

static uint64_t fold_i32(uint64_t h, const int32_t *v) {
    uint64_t a = h;
    a = mix_i32(a, v[0]);
    a = mix_i32(a, v[1]);
    return a;
}

static uint64_t fold_u32(uint64_t h, const uint32_t *v) {
    uint64_t a = h;
    a = mix_u32(a, v[0]);
    a = mix_u32(a, v[1]);
    return a;
}

static uint64_t fold_i64(uint64_t h, const int64_t *v) {
    uint64_t a = h;
    a = mix_i64(a, v[0]);
    a = mix_i64(a, v[1]);
    return a;
}

static uint64_t fold_u64(uint64_t h, const uint64_t *v) {
    uint64_t a = h;
    a = mix_u64(a, v[0]);
    a = mix_u64(a, v[1]);
    return a;
}

static uint64_t fold_f32(uint64_t h, const float *v) {
    uint64_t a = h;
    a = mix_f32(a, v[0]);
    a = mix_f32(a, v[1]);
    return a;
}

static uint64_t fold_f64(uint64_t h, const double *v) {
    uint64_t a = h;
    a = mix_f64(a, v[0]);
    a = mix_f64(a, v[1]);
    return a;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float zero_f32 = (float)seed;
    const double zero_f64 = (double)seed;
    {
        int8_t x[2] = {(int8_t)(-127 - 1), (int8_t)127};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int8_t x[2] = {(int8_t)-1, (int8_t)53};
        x[1] = (int8_t)(x[1] ^ (int8_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int8_t x[2] = {(int8_t)(-127 - 1), (int8_t)127};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int8_t x[2] = {(int8_t)-1, (int8_t)53};
        x[1] = (int8_t)(x[1] ^ (int8_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int8_t x[2] = {(int8_t)(-127 - 1), (int8_t)127};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int8_t x[2] = {(int8_t)-1, (int8_t)53};
        x[1] = (int8_t)(x[1] ^ (int8_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)0, (uint8_t)255};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)128, (uint8_t)53};
        x[1] = (uint8_t)(x[1] ^ (uint8_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)0, (uint8_t)255};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)128, (uint8_t)53};
        x[1] = (uint8_t)(x[1] ^ (uint8_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)0, (uint8_t)255};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint8_t x[2] = {(uint8_t)128, (uint8_t)53};
        x[1] = (uint8_t)(x[1] ^ (uint8_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)(-32767 - 1), (int16_t)32767};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)-1, (int16_t)13689};
        x[1] = (int16_t)(x[1] ^ (int16_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)(-32767 - 1), (int16_t)32767};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)-1, (int16_t)13689};
        x[1] = (int16_t)(x[1] ^ (int16_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)(-32767 - 1), (int16_t)32767};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int16_t x[2] = {(int16_t)-1, (int16_t)13689};
        x[1] = (int16_t)(x[1] ^ (int16_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)0, (uint16_t)65535};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)32768, (uint16_t)13689};
        x[1] = (uint16_t)(x[1] ^ (uint16_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)0, (uint16_t)65535};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)32768, (uint16_t)13689};
        x[1] = (uint16_t)(x[1] ^ (uint16_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)0, (uint16_t)65535};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint16_t x[2] = {(uint16_t)32768, (uint16_t)13689};
        x[1] = (uint16_t)(x[1] ^ (uint16_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)-1, (int32_t)324508639};
        x[1] = (int32_t)(x[1] ^ (int32_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)-1, (int32_t)324508639};
        x[1] = (int32_t)(x[1] ^ (int32_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int32_t x[2] = {(int32_t)-1, (int32_t)324508639};
        x[1] = (int32_t)(x[1] ^ (int32_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)0, (uint32_t)4294967295u};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)2147483648u, (uint32_t)324508639};
        x[1] = (uint32_t)(x[1] ^ (uint32_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)0, (uint32_t)4294967295u};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)2147483648u, (uint32_t)324508639};
        x[1] = (uint32_t)(x[1] ^ (uint32_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)0, (uint32_t)4294967295u};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint32_t x[2] = {(uint32_t)2147483648u, (uint32_t)324508639};
        x[1] = (uint32_t)(x[1] ^ (uint32_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int64_t x[2] = {(int64_t)(-9223372036854775807ll - 1), (int64_t)9223372036854775807ll};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int64_t x[2] = {(int64_t)-1, (int64_t)87109624524081870ll};
        x[1] = (int64_t)(x[1] ^ (int64_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)0, (uint64_t)18446744073709551615ull};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint64_t x[2] = {(uint64_t)9223372036854775808ull, (uint64_t)87109624524081870ull};
        x[1] = (uint64_t)(x[1] ^ (uint64_t)seed);
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        float x[2] = {-2.5, 3.75};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)(x[k] + zero_f32);
        h = fold_i64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        float x[2] = {-100.875, 126.5};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)(x[k] + zero_f32);
        h = fold_i64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        float x[2] = {0.0, 2.5};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)(x[k] + zero_f32);
        h = fold_u64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        float x[2] = {200.75, 254.875};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)(x[k] + zero_f32);
        h = fold_u64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        float x[2] = {-2.5, 3.140625};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)(x[k] + zero_f32);
        h = fold_f64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        float x[2] = {1.0e30, 0.000001};
        double y[2];
        for (int k = 0; k < 2; k++) y[k] = (double)(x[k] + zero_f32);
        h = fold_f64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-2.5, 3.75};
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)(x[k] + zero_f64);
        h = fold_i8(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-100.875, 126.5};
        int8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int8_t)(x[k] + zero_f64);
        h = fold_i8(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {0.0, 2.5};
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)(x[k] + zero_f64);
        h = fold_u8(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {200.75, 254.875};
        uint8_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint8_t)(x[k] + zero_f64);
        h = fold_u8(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-2.5, 3.75};
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)(x[k] + zero_f64);
        h = fold_i16(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-100.875, 126.5};
        int16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int16_t)(x[k] + zero_f64);
        h = fold_i16(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {0.0, 2.5};
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)(x[k] + zero_f64);
        h = fold_u16(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {200.75, 254.875};
        uint16_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint16_t)(x[k] + zero_f64);
        h = fold_u16(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-2.5, 3.75};
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)(x[k] + zero_f64);
        h = fold_i32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-100.875, 126.5};
        int32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int32_t)(x[k] + zero_f64);
        h = fold_i32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {0.0, 2.5};
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)(x[k] + zero_f64);
        h = fold_u32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {200.75, 254.875};
        uint32_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint32_t)(x[k] + zero_f64);
        h = fold_u32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-2.5, 3.75};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)(x[k] + zero_f64);
        h = fold_i64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-100.875, 126.5};
        int64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (int64_t)(x[k] + zero_f64);
        h = fold_i64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {0.0, 2.5};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)(x[k] + zero_f64);
        h = fold_u64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {200.75, 254.875};
        uint64_t y[2];
        for (int k = 0; k < 2; k++) y[k] = (uint64_t)(x[k] + zero_f64);
        h = fold_u64(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {-2.5, 3.140625};
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)(x[k] + zero_f64);
        h = fold_f32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    {
        double x[2] = {1.0e30, 0.000001};
        float y[2];
        for (int k = 0; k < 2; k++) y[k] = (float)(x[k] + zero_f64);
        h = fold_f32(h, y);
        for (int k = 0; k < 2; k++) h = mix_u8(h, 1);
    }
    return h;
}
