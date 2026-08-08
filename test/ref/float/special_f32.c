#include "corpus.h"

static float opaque_f32(uint32_t bits, uint32_t s) {
    uint32_t b = (uint32_t)(bits + s);
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint32_t f32_to_bits(float v) {
    uint32_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static int32_t bits_to_i32(uint32_t b) {
    int32_t v;
    memcpy(&v, &b, sizeof v);
    return v;
}

static uint64_t fold32(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const float one  = opaque_f32(UINT32_C(0x3F800000), s);
    const float mone = opaque_f32(UINT32_C(0xBF800000), s);
    const float pz   = opaque_f32(UINT32_C(0x00000000), s);
    const float nz   = opaque_f32(UINT32_C(0x80000000), s);
    const float pinf = opaque_f32(UINT32_C(0x7F800000), s);
    const float ninf = opaque_f32(UINT32_C(0xFF800000), s);
    const float nan_ = opaque_f32(UINT32_C(0x7FC00000), s);
    const float fmax = opaque_f32(UINT32_C(0x7F7FFFFF), s);
    const float fmin = opaque_f32(UINT32_C(0x00800000), s);
    const float dmax = opaque_f32(UINT32_C(0x007FFFFF), s);
    const float tiny = opaque_f32(UINT32_C(0x00000001), s);

    h = fold32(h, pz);
    h = fold32(h, nz);
    h = fold32(h, pz + pz);
    h = fold32(h, pz + nz);
    h = fold32(h, nz + pz);
    h = fold32(h, nz + nz);
    h = fold32(h, pz - pz);
    h = fold32(h, pz - nz);
    h = fold32(h, nz - pz);
    h = fold32(h, nz - nz);
    h = fold32(h, pz * one);
    h = fold32(h, pz * mone);
    h = fold32(h, nz * one);
    h = fold32(h, nz * mone);
    h = fold32(h, nz * nz);
    h = fold32(h, pz * nz);
    h = fold32(h, -pz);
    h = fold32(h, -nz);
    h = fold32(h, pz / one);
    h = fold32(h, pz / mone);
    h = fold32(h, nz / one);
    h = fold32(h, nz / mone);
    h = mix_u8(h, (uint8_t)(pz == nz));
    h = mix_u8(h, (uint8_t)(pz < nz));
    h = mix_u8(h, (uint8_t)(nz < pz));

    h = fold32(h, pinf);
    h = fold32(h, ninf);
    h = fold32(h, -pinf);
    h = fold32(h, one / pz);
    h = fold32(h, one / nz);
    h = fold32(h, mone / pz);
    h = fold32(h, mone / nz);
    h = fold32(h, pinf + one);
    h = fold32(h, pinf + pinf);
    h = fold32(h, pinf - ninf);
    h = fold32(h, pinf * pinf);
    h = fold32(h, pinf * ninf);
    h = fold32(h, pinf * mone);
    h = fold32(h, one / pinf);
    h = fold32(h, mone / pinf);
    h = fold32(h, one / ninf);
    h = fold32(h, pinf / one);
    h = fold32(h, fmax + fmax);
    h = mix_u8(h, (uint8_t)(pinf > fmax));
    h = mix_u8(h, (uint8_t)(ninf < (float)(0.0f - fmax)));

    h = fold32(h, pinf - pinf);
    h = fold32(h, ninf + pinf);
    h = fold32(h, pinf * pz);
    h = fold32(h, ninf * nz);
    h = fold32(h, pinf / pinf);
    h = fold32(h, pz / pz);
    h = fold32(h, nz / pz);

    h = mix_u32(h, f32_to_bits(nan_));
    h = mix_u8(h, (uint8_t)(nan_ != nan_));
    h = mix_u8(h, (uint8_t)(nan_ == nan_));
    h = mix_u8(h, (uint8_t)(nan_ < nan_));
    h = mix_u8(h, (uint8_t)(nan_ >= nan_));
    h = mix_u32(h, f32_to_bits(-nan_));
    h = mix_u32(h, f32_to_bits(-(float)(-nan_)));
    h = fold32(h, nan_ + one);
    h = fold32(h, one + nan_);
    h = fold32(h, nan_ * pz);
    h = fold32(h, nan_ - nan_);
    h = fold32(h, nan_ / nan_);
    h = fold32(h, nan_ / pinf);

    h = fold32(h, tiny);
    h = fold32(h, dmax);
    h = fold32(h, fmin);
    h = fold32(h, dmax + tiny);
    h = fold32(h, fmin - tiny);
    h = fold32(h, tiny + tiny);
    h = fold32(h, tiny * 1.5f);
    h = fold32(h, tiny * 0.5f);
    h = fold32(h, -tiny);
    h = fold32(h, tiny * 8388608.0f);
    h = fold32(h, dmax / fmin);
    h = mix_u8(h, (uint8_t)(tiny > pz));
    h = mix_u8(h, (uint8_t)(tiny == pz));
    h = mix_u8(h, (uint8_t)((float)(0.0f - tiny) < nz));

    float d = fmin;
    for (uint32_t i = 0; i < UINT32_C(30); i = (uint32_t)(i + UINT32_C(1))) {
        d = d * 0.5f;
        h = fold32(h, d);
    }

    const uint32_t rt[8] = {
        UINT32_C(0x00000000), UINT32_C(0x80000000), UINT32_C(0x00000001), UINT32_C(0x7F800000),
        UINT32_C(0xFF800000), UINT32_C(0x7FC00000), UINT32_C(0xFFC0DEAD), UINT32_C(0x7F800001)
    };
    for (uint32_t k = 0; k < UINT32_C(8); k = (uint32_t)(k + UINT32_C(1))) {
        const float f = opaque_f32(rt[k], s);
        h = mix_u32(h, f32_to_bits(f));
    }

    const uint32_t ua = (uint32_t)(UINT32_C(16777216) + s);
    const uint32_t ub = (uint32_t)(UINT32_C(16777217) + s);
    const uint32_t uc = (uint32_t)(UINT32_C(16777219) + s);
    const uint32_t ud = (uint32_t)(UINT32_C(4294967295) + s);
    const int32_t ia = bits_to_i32((uint32_t)(UINT32_C(0xFEFFFFFF) + s));
    const int32_t ib = bits_to_i32((uint32_t)(UINT32_C(0x80000000) + s));
    h = fold32(h, (float)ua);
    h = fold32(h, (float)ub);
    h = fold32(h, (float)uc);
    h = fold32(h, (float)ud);
    h = fold32(h, (float)s);
    h = fold32(h, (float)ia);
    h = fold32(h, (float)ib);

    const float ca = 1.5f * one;
    const float cb = 16777216.0f * one;
    const float cc = 2147483520.0f * one;
    const float cd = 0.0f - 1.5f * one;
    const float ce = 0.5f * one;
    h = mix_u32(h, (uint32_t)ca);
    h = mix_u32(h, (uint32_t)cb);
    h = mix_u32(h, (uint32_t)cc);
    h = mix_u32(h, (uint32_t)ce);
    h = mix_u32(h, (uint32_t)pz);
    h = mix_i32(h, (int32_t)cd);
    h = mix_i32(h, (int32_t)cc);
    h = mix_i32(h, (int32_t)(float)(0.0f - ce));
    h = mix_i64(h, (int64_t)cd);
    h = mix_i64(h, (int64_t)(float)(0.0f - cb));
    return h;
}
