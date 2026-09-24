#include "corpus.h"

/* a range is the consecutive elements it names, a vector load or store moves
 * them as they are, and a half widened lane by lane extends each lane by the
 * source's signedness. every lane or element folds in order. */

static uint64_t fold_f32s(uint64_t h, const float *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_f32(h, p[k]); }
    return h;
}

static uint64_t fold_f64s(uint64_t h, const double *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_f64(h, p[k]); }
    return h;
}

static uint64_t fold_i16s(uint64_t h, const int16_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_i16(h, p[k]); }
    return h;
}

static uint64_t fold_u16s(uint64_t h, const uint16_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u16(h, p[k]); }
    return h;
}

static uint64_t fold_u8s(uint64_t h, const uint8_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u8(h, p[k]); }
    return h;
}

static uint64_t fold_i32s(uint64_t h, const int32_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_i32(h, p[k]); }
    return h;
}

static uint64_t fold_u32s(uint64_t h, const uint32_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u32(h, p[k]); }
    return h;
}

static uint64_t fold_i64s(uint64_t h, const int64_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_i64(h, p[k]); }
    return h;
}

static uint64_t fold_u64s(uint64_t h, const uint64_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u64(h, p[k]); }
    return h;
}

static void store_f32(float *p, uint64_t i, const float *v) {
    for (unsigned k = 0; k < 4u; k++) { p[i + k] = v[k] * v[k]; }
}

static void store_i32(int32_t *p, uint64_t i, const int32_t *v) {
    for (unsigned k = 0; k < 4u; k++) { p[i + k] = (int32_t)((uint32_t)v[k] + (uint32_t)v[k]); }
}

static void store_u8(uint8_t *p, uint64_t i, const uint8_t *v) {
    for (unsigned k = 0; k < 16u; k++) { p[i + k] = (uint8_t)(v[k] + v[k]); }
}

static uint64_t array_i16(uint64_t h, int16_t a, int16_t b, uint64_t i) {
    int16_t arr[14] = {
        a, b, (int16_t)(uint16_t)((uint16_t)a + (uint16_t)b), (int16_t)(uint16_t)((uint16_t)a - (uint16_t)b),
        (int16_t)(uint16_t)((uint16_t)b - (uint16_t)a), a, b, 7, -7, a, b, 3, 11, -11
    };
    int16_t win[8];
    for (unsigned k = 0; k < 8u; k++) { win[k] = (int16_t)(uint16_t)((uint16_t)arr[i + k] + 1u); }
    for (unsigned k = 0; k < 8u; k++) { arr[i + 2u + k] = win[k]; }
    return fold_i16s(h, &arr[2], 8u);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t at = seed & 3u;
    {
        float f[16];
        for (unsigned k = 0; k < 16u; k++) { f[k] = ((float)k - 5.5f) * 1.25f + (float)seed; }
        h = fold_f32s(h, &f[at + 1u], 4u);
        h = fold_f32s(h, &f[4], 4u);
        {
            float v[4] = { 1.5f, -2.0f, 0.25f, 8.0f };
            store_f32(f, at + 3u, v);
        }
        {
            float v[4] = { 3.0f, -0.5f, 2.5f, -4.0f };
            store_f32(f, 8u, v);
        }
        h = fold_f32s(h, f, 16u);
    }
    {
        double f[8];
        for (unsigned k = 0; k < 8u; k++) { f[k] = ((double)k - 3.5) * 0.75 + (double)seed; }
        h = fold_f64s(h, &f[at + 1u], 2u);
        h = fold_f64s(h, &f[2], 2u);
    }
    {
        int32_t a[12];
        for (unsigned k = 0; k < 12u; k++) {
            a[k] = (int32_t)(uint32_t)((uint32_t)(((int32_t)k - 6) * 1000003) + (uint32_t)seed);
        }
        {
            int32_t v[4] = { 1073741823, -1073741823, 5, -9 };
            store_i32(a, at + 1u, v);
        }
        {
            int32_t v[4] = { 1, -1, 536870912, -536870912 };
            store_i32(a, 8u, v);
        }
        h = fold_i32s(h, a, 12u);
    }
    {
        uint8_t b[40];
        for (unsigned k = 0; k < 40u; k++) { b[k] = (uint8_t)((uint64_t)k * 37u + seed); }
        h = fold_u8s(h, &b[at + 1u], 16u);
        {
            uint8_t v[16];
            for (unsigned k = 0; k < 16u; k++) { v[k] = b[k]; }
            store_u8(b, at + 17u, v);
        }
        h = fold_u8s(h, b, 40u);
    }
    h = array_i16(h, (int16_t)(uint16_t)(3u + (uint16_t)seed), -20000, at);
    {
        int8_t v[16] = { -128, 127, -1, 0, 1, -127, 126, 7, -128, -2, 3, 127, -127, 64, -64, 100 };
        int16_t w[8];
        v[1] = (int8_t)(uint8_t)((uint8_t)v[1] ^ (uint8_t)seed);
        for (unsigned k = 0; k < 8u; k++) { w[k] = (int16_t)v[k]; }
        h = fold_i16s(h, w, 8u);
        for (unsigned k = 0; k < 8u; k++) { w[k] = (int16_t)v[8u + k]; }
        h = fold_i16s(h, w, 8u);
    }
    {
        uint8_t v[16] = { 0, 255, 254, 0, 1, 1, 254, 7, 128, 129, 3, 255, 2, 64, 192, 100 };
        uint16_t w[8];
        v[1] = (uint8_t)(v[1] ^ (uint8_t)seed);
        for (unsigned k = 0; k < 8u; k++) { w[k] = (uint16_t)v[k]; }
        h = fold_u16s(h, w, 8u);
        for (unsigned k = 0; k < 8u; k++) { w[k] = (uint16_t)v[8u + k]; }
        h = fold_u16s(h, w, 8u);
    }
    {
        int16_t v[8] = { -32768, 32767, -1, 0, 1, -32767, 32766, -7 };
        int32_t w[4];
        v[1] = (int16_t)(uint16_t)((uint16_t)v[1] ^ (uint16_t)seed);
        for (unsigned k = 0; k < 4u; k++) { w[k] = (int32_t)v[k]; }
        h = fold_i32s(h, w, 4u);
        for (unsigned k = 0; k < 4u; k++) { w[k] = (int32_t)v[4u + k]; }
        h = fold_i32s(h, w, 4u);
        for (unsigned k = 0; k < 4u; k++) { w[k] = (int32_t)v[2u + k]; }
        h = fold_i32s(h, w, 4u);
    }
    {
        uint16_t v[8] = { 0, 65535, 65534, 0, 1, 32768, 32767, 7 };
        uint32_t w[4];
        v[1] = (uint16_t)(v[1] ^ (uint16_t)seed);
        for (unsigned k = 0; k < 4u; k++) { w[k] = (uint32_t)v[k]; }
        h = fold_u32s(h, w, 4u);
        for (unsigned k = 0; k < 4u; k++) { w[k] = (uint32_t)v[4u + k]; }
        h = fold_u32s(h, w, 4u);
    }
    {
        int32_t v[4] = { INT32_MIN, 2147483647, -1, -2147483647 };
        int64_t w[2];
        v[1] = (int32_t)((uint32_t)v[1] ^ (uint32_t)seed);
        for (unsigned k = 0; k < 2u; k++) { w[k] = (int64_t)v[k]; }
        h = fold_i64s(h, w, 2u);
        for (unsigned k = 0; k < 2u; k++) { w[k] = (int64_t)v[2u + k]; }
        h = fold_i64s(h, w, 2u);
    }
    {
        uint32_t v[4] = { 0u, 4294967295u, 2147483648u, 7u };
        uint64_t w[2];
        uint32_t s[4];
        v[1] = v[1] ^ (uint32_t)seed;
        for (unsigned k = 0; k < 2u; k++) { w[k] = (uint64_t)v[k]; }
        h = fold_u64s(h, w, 2u);
        for (unsigned k = 0; k < 2u; k++) { w[k] = (uint64_t)v[2u + k]; }
        h = fold_u64s(h, w, 2u);
        s[0] = v[2]; s[1] = v[3]; s[2] = v[0]; s[3] = v[1];
        h = fold_u32s(h, s, 4u);
    }
    {
        int64_t a = (int64_t)(5u + seed), b = -9;
        int64_t src[6] = {
            a, b, (int64_t)((uint64_t)a + (uint64_t)b), (int64_t)((uint64_t)a - (uint64_t)b),
            (int64_t)((uint64_t)a * (uint64_t)b), (int64_t)((uint64_t)b - (uint64_t)a)
        };
        int64_t dst[4];
        for (unsigned k = 0; k < 3u; k++) { dst[1u + k] = src[at + k]; }
        dst[0] = src[5];
        h = fold_i64s(h, dst, 4u);
    }
    return h;
}
