#include <math.h>
#include "corpus.h"

static uint64_t fold_i8(uint64_t h, const int8_t *v) {
    uint64_t a = h;
    a = mix_i8(a, v[0]);
    a = mix_i8(a, v[1]);
    a = mix_i8(a, v[2]);
    a = mix_i8(a, v[3]);
    return a;
}

static uint64_t fold_u8(uint64_t h, const uint8_t *v) {
    uint64_t a = h;
    a = mix_u8(a, v[0]);
    a = mix_u8(a, v[1]);
    a = mix_u8(a, v[2]);
    a = mix_u8(a, v[3]);
    return a;
}

static uint64_t fold_i16(uint64_t h, const int16_t *v) {
    uint64_t a = h;
    a = mix_i16(a, v[0]);
    a = mix_i16(a, v[1]);
    a = mix_i16(a, v[2]);
    a = mix_i16(a, v[3]);
    return a;
}

static uint64_t fold_u16(uint64_t h, const uint16_t *v) {
    uint64_t a = h;
    a = mix_u16(a, v[0]);
    a = mix_u16(a, v[1]);
    a = mix_u16(a, v[2]);
    a = mix_u16(a, v[3]);
    return a;
}

static uint64_t fold_i32(uint64_t h, const int32_t *v) {
    uint64_t a = h;
    a = mix_i32(a, v[0]);
    a = mix_i32(a, v[1]);
    a = mix_i32(a, v[2]);
    a = mix_i32(a, v[3]);
    return a;
}

static uint64_t fold_u32(uint64_t h, const uint32_t *v) {
    uint64_t a = h;
    a = mix_u32(a, v[0]);
    a = mix_u32(a, v[1]);
    a = mix_u32(a, v[2]);
    a = mix_u32(a, v[3]);
    return a;
}

static uint64_t fold_i64(uint64_t h, const int64_t *v) {
    uint64_t a = h;
    a = mix_i64(a, v[0]);
    a = mix_i64(a, v[1]);
    a = mix_i64(a, v[2]);
    a = mix_i64(a, v[3]);
    return a;
}

static uint64_t fold_u64(uint64_t h, const uint64_t *v) {
    uint64_t a = h;
    a = mix_u64(a, v[0]);
    a = mix_u64(a, v[1]);
    a = mix_u64(a, v[2]);
    a = mix_u64(a, v[3]);
    return a;
}

static uint64_t fold_f32(uint64_t h, const float *v) {
    uint64_t a = h;
    a = mix_f32(a, v[0]);
    a = mix_f32(a, v[1]);
    a = mix_f32(a, v[2]);
    a = mix_f32(a, v[3]);
    return a;
}

static uint64_t fold_f64(uint64_t h, const double *v) {
    uint64_t a = h;
    a = mix_f64(a, v[0]);
    a = mix_f64(a, v[1]);
    a = mix_f64(a, v[2]);
    a = mix_f64(a, v[3]);
    return a;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float zero_f32 = (float)seed;
    const double zero_f64 = (double)seed;
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int8_t x[4] = {(int8_t)(-127 - 1), (int8_t)127, (int8_t)-1, (int8_t)53};
        x[3] = (int8_t)(x[3] ^ (int8_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint8_t x[4] = {(uint8_t)0, (uint8_t)255, (uint8_t)128, (uint8_t)53};
        x[3] = (uint8_t)(x[3] ^ (uint8_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int16_t x[4] = {(int16_t)(-32767 - 1), (int16_t)32767, (int16_t)-1, (int16_t)13689};
        x[3] = (int16_t)(x[3] ^ (int16_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint16_t x[4] = {(uint16_t)0, (uint16_t)65535, (uint16_t)32768, (uint16_t)13689};
        x[3] = (uint16_t)(x[3] ^ (uint16_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int32_t x[4] = {(int32_t)(-2147483647 - 1), (int32_t)2147483647, (int32_t)-1, (int32_t)324508639};
        x[3] = (int32_t)(x[3] ^ (int32_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint32_t x[4] = {(uint32_t)0, (uint32_t)4294967295, (uint32_t)2147483648, (uint32_t)324508639};
        x[3] = (uint32_t)(x[3] ^ (uint32_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)x[k];
        h = fold_u64(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        int64_t x[4] = {(int64_t)(-INT64_C(9223372036854775807) - 1), (int64_t)9223372036854775807LL, (int64_t)-1LL, (int64_t)87109624524081870LL};
        x[3] = (int64_t)(x[3] ^ (int64_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)x[k];
        h = fold_i8(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)x[k];
        h = fold_u8(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)x[k];
        h = fold_i16(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)x[k];
        h = fold_u16(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)x[k];
        h = fold_i32(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)x[k];
        h = fold_u32(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)x[k];
        h = fold_i64(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)x[k];
        h = fold_f32(h, y);
    }
    {
        uint64_t x[4] = {(uint64_t)0ULL, (uint64_t)18446744073709551615ULL, (uint64_t)9223372036854775808ULL, (uint64_t)87109624524081870ULL};
        x[3] = (uint64_t)(x[3] ^ (uint64_t)seed);
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)x[k];
        h = fold_f64(h, y);
    }
    {
        float x[4] = {-2.5f, 3.75f, -100.875f, 126.5f};
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)(x[k] + zero_f32);
        h = fold_i8(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {0.0f, 2.5f, 200.75f, 254.875f};
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)(x[k] + zero_f32);
        h = fold_u8(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {-2.5f, 3.75f, -100.875f, 126.5f};
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)(x[k] + zero_f32);
        h = fold_i16(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {0.0f, 2.5f, 200.75f, 254.875f};
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)(x[k] + zero_f32);
        h = fold_u16(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {-2.5f, 3.75f, -100.875f, 126.5f};
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)(x[k] + zero_f32);
        h = fold_i32(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {0.0f, 2.5f, 200.75f, 254.875f};
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)(x[k] + zero_f32);
        h = fold_u32(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {-2.5f, 3.75f, -100.875f, 126.5f};
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)(x[k] + zero_f32);
        h = fold_i64(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {0.0f, 2.5f, 200.75f, 254.875f};
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)(x[k] + zero_f32);
        h = fold_u64(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        float x[4] = {-2.5f, 3.140625f, 1.0e30f, 0.000001f};
        double y[4];
        for (int k = 0; k < 4; k++) y[k] = (double)(x[k] + zero_f32);
        h = fold_f64(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {-2.5, 3.75, -100.875, 126.5};
        int8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int8_t)(x[k] + zero_f64);
        h = fold_i8(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {0.0, 2.5, 200.75, 254.875};
        uint8_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint8_t)(x[k] + zero_f64);
        h = fold_u8(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {-2.5, 3.75, -100.875, 126.5};
        int16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int16_t)(x[k] + zero_f64);
        h = fold_i16(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {0.0, 2.5, 200.75, 254.875};
        uint16_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint16_t)(x[k] + zero_f64);
        h = fold_u16(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {-2.5, 3.75, -100.875, 126.5};
        int32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int32_t)(x[k] + zero_f64);
        h = fold_i32(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {0.0, 2.5, 200.75, 254.875};
        uint32_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint32_t)(x[k] + zero_f64);
        h = fold_u32(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {-2.5, 3.75, -100.875, 126.5};
        int64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (int64_t)(x[k] + zero_f64);
        h = fold_i64(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {0.0, 2.5, 200.75, 254.875};
        uint64_t y[4];
        for (int k = 0; k < 4; k++) y[k] = (uint64_t)(x[k] + zero_f64);
        h = fold_u64(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    {
        double x[4] = {-2.5, 3.140625, 1.0e30, 0.000001};
        float y[4];
        for (int k = 0; k < 4; k++) y[k] = (float)(x[k] + zero_f64);
        h = fold_f32(h, y);
        for (int k = 0; k < 4; k++) h = mix_u8(h, 1);
    }
    return h;
}
