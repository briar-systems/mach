#include "corpus.h"

static inline float bits_to_f32(uint32_t v) {
    float f;
    memcpy(&f, &v, sizeof f);
    return f;
}

static inline double bits_to_f64(uint64_t v) {
    double d;
    memcpy(&d, &v, sizeof d);
    return d;
}

static inline uint32_t f32_to_bits(float v) {
    uint32_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static inline uint64_t f64_to_bits(double v) {
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const uint32_t s32 = (uint32_t)((uint32_t)seed ^ UINT32_C(3226013659));
    const float f = bits_to_f32(s32);
    const uint32_t back32 = f32_to_bits(f);
    h = mix_u32(h, s32);
    h = mix_f32(h, f);
    h = mix_u32(h, back32);

    const uint64_t s64 = (uint64_t)(seed ^ UINT64_C(4614256656552045848));
    const double d = bits_to_f64(s64);
    const uint64_t back64 = f64_to_bits(d);
    h = mix_u64(h, s64);
    h = mix_f64(h, d);
    h = mix_u64(h, back64);

    const float pi = 3.14159265f;
    const uint32_t pi_bits = f32_to_bits(pi);
    const float pi_back = bits_to_f32(pi_bits);
    h = mix_u32(h, pi_bits);
    h = mix_f32(h, pi_back);

    const double e = 2.718281828459045;
    const uint64_t e_bits = f64_to_bits(e);
    const double e_back = bits_to_f64(e_bits);
    h = mix_u64(h, e_bits);
    h = mix_f64(h, e_back);

    return h;
}
