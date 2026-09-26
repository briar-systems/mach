/* the f32 and f64 values a lowering is most likely to lose: zeros, infinities, NaN
 * and denormals. one noinline part per width. */
#include "corpus.h"

static float opaque_f32_special_f32(uint32_t bits, uint32_t s) {
    uint32_t b = (uint32_t)(bits + s);
    float f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint32_t f32_to_bits_special_f32(float v) {
    uint32_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static int32_t bits_to_i32_special_f32(uint32_t b) {
    int32_t v;
    memcpy(&v, &b, sizeof v);
    return v;
}

static uint64_t fold32_special_f32(uint64_t h, float v) {
    if (v != v) { return mix_u32(h, UINT32_C(0xFFFFFFFF)); }
    return mix_f32(h, v);
}

static uint64_t special_f32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const float one  = opaque_f32_special_f32(UINT32_C(0x3F800000), s);
    const float mone = opaque_f32_special_f32(UINT32_C(0xBF800000), s);
    const float pz   = opaque_f32_special_f32(UINT32_C(0x00000000), s);
    const float nz   = opaque_f32_special_f32(UINT32_C(0x80000000), s);
    const float pinf = opaque_f32_special_f32(UINT32_C(0x7F800000), s);
    const float ninf = opaque_f32_special_f32(UINT32_C(0xFF800000), s);
    const float nan_ = opaque_f32_special_f32(UINT32_C(0x7FC00000), s);
    const float fmax = opaque_f32_special_f32(UINT32_C(0x7F7FFFFF), s);
    const float fmin = opaque_f32_special_f32(UINT32_C(0x00800000), s);
    const float dmax = opaque_f32_special_f32(UINT32_C(0x007FFFFF), s);
    const float tiny = opaque_f32_special_f32(UINT32_C(0x00000001), s);

    h = fold32_special_f32(h, pz);
    h = fold32_special_f32(h, nz);
    h = fold32_special_f32(h, pz + pz);
    h = fold32_special_f32(h, pz + nz);
    h = fold32_special_f32(h, nz + pz);
    h = fold32_special_f32(h, nz + nz);
    h = fold32_special_f32(h, pz - pz);
    h = fold32_special_f32(h, pz - nz);
    h = fold32_special_f32(h, nz - pz);
    h = fold32_special_f32(h, nz - nz);
    h = fold32_special_f32(h, pz * one);
    h = fold32_special_f32(h, pz * mone);
    h = fold32_special_f32(h, nz * one);
    h = fold32_special_f32(h, nz * mone);
    h = fold32_special_f32(h, nz * nz);
    h = fold32_special_f32(h, pz * nz);
    h = fold32_special_f32(h, -pz);
    h = fold32_special_f32(h, -nz);
    h = fold32_special_f32(h, pz / one);
    h = fold32_special_f32(h, pz / mone);
    h = fold32_special_f32(h, nz / one);
    h = fold32_special_f32(h, nz / mone);
    h = mix_u8(h, (uint8_t)(pz == nz));
    h = mix_u8(h, (uint8_t)(pz < nz));
    h = mix_u8(h, (uint8_t)(nz < pz));

    h = fold32_special_f32(h, pinf);
    h = fold32_special_f32(h, ninf);
    h = fold32_special_f32(h, -pinf);
    h = fold32_special_f32(h, one / pz);
    h = fold32_special_f32(h, one / nz);
    h = fold32_special_f32(h, mone / pz);
    h = fold32_special_f32(h, mone / nz);
    h = fold32_special_f32(h, pinf + one);
    h = fold32_special_f32(h, pinf + pinf);
    h = fold32_special_f32(h, pinf - ninf);
    h = fold32_special_f32(h, pinf * pinf);
    h = fold32_special_f32(h, pinf * ninf);
    h = fold32_special_f32(h, pinf * mone);
    h = fold32_special_f32(h, one / pinf);
    h = fold32_special_f32(h, mone / pinf);
    h = fold32_special_f32(h, one / ninf);
    h = fold32_special_f32(h, pinf / one);
    h = fold32_special_f32(h, fmax + fmax);
    h = mix_u8(h, (uint8_t)(pinf > fmax));
    h = mix_u8(h, (uint8_t)(ninf < (float)(0.0f - fmax)));

    h = fold32_special_f32(h, pinf - pinf);
    h = fold32_special_f32(h, ninf + pinf);
    h = fold32_special_f32(h, pinf * pz);
    h = fold32_special_f32(h, ninf * nz);
    h = fold32_special_f32(h, pinf / pinf);
    h = fold32_special_f32(h, pz / pz);
    h = fold32_special_f32(h, nz / pz);

    h = mix_u32(h, f32_to_bits_special_f32(nan_));
    h = mix_u8(h, (uint8_t)(nan_ != nan_));
    h = mix_u8(h, (uint8_t)(nan_ == nan_));
    h = mix_u8(h, (uint8_t)(nan_ < nan_));
    h = mix_u8(h, (uint8_t)(nan_ >= nan_));
    h = mix_u32(h, f32_to_bits_special_f32(-nan_));
    h = mix_u32(h, f32_to_bits_special_f32(-(float)(-nan_)));
    h = fold32_special_f32(h, nan_ + one);
    h = fold32_special_f32(h, one + nan_);
    h = fold32_special_f32(h, nan_ * pz);
    h = fold32_special_f32(h, nan_ - nan_);
    h = fold32_special_f32(h, nan_ / nan_);
    h = fold32_special_f32(h, nan_ / pinf);

    h = fold32_special_f32(h, tiny);
    h = fold32_special_f32(h, dmax);
    h = fold32_special_f32(h, fmin);
    h = fold32_special_f32(h, dmax + tiny);
    h = fold32_special_f32(h, fmin - tiny);
    h = fold32_special_f32(h, tiny + tiny);
    h = fold32_special_f32(h, tiny * 1.5f);
    h = fold32_special_f32(h, tiny * 0.5f);
    h = fold32_special_f32(h, -tiny);
    h = fold32_special_f32(h, tiny * 8388608.0f);
    h = fold32_special_f32(h, dmax / fmin);
    h = mix_u8(h, (uint8_t)(tiny > pz));
    h = mix_u8(h, (uint8_t)(tiny == pz));
    h = mix_u8(h, (uint8_t)((float)(0.0f - tiny) < nz));

    float d = fmin;
    for (uint32_t i = 0; i < UINT32_C(30); i = (uint32_t)(i + UINT32_C(1))) {
        d = d * 0.5f;
        h = fold32_special_f32(h, d);
    }

    const uint32_t rt[8] = {
        UINT32_C(0x00000000), UINT32_C(0x80000000), UINT32_C(0x00000001), UINT32_C(0x7F800000),
        UINT32_C(0xFF800000), UINT32_C(0x7FC00000), UINT32_C(0xFFC0DEAD), UINT32_C(0x7F800001)
    };
    for (uint32_t k = 0; k < UINT32_C(8); k = (uint32_t)(k + UINT32_C(1))) {
        const float f = opaque_f32_special_f32(rt[k], s);
        h = mix_u32(h, f32_to_bits_special_f32(f));
    }

    const uint32_t ua = (uint32_t)(UINT32_C(16777216) + s);
    const uint32_t ub = (uint32_t)(UINT32_C(16777217) + s);
    const uint32_t uc = (uint32_t)(UINT32_C(16777219) + s);
    const uint32_t ud = (uint32_t)(UINT32_C(4294967295) + s);
    const int32_t ia = bits_to_i32_special_f32((uint32_t)(UINT32_C(0xFEFFFFFF) + s));
    const int32_t ib = bits_to_i32_special_f32((uint32_t)(UINT32_C(0x80000000) + s));
    h = fold32_special_f32(h, (float)ua);
    h = fold32_special_f32(h, (float)ub);
    h = fold32_special_f32(h, (float)uc);
    h = fold32_special_f32(h, (float)ud);
    h = fold32_special_f32(h, (float)s);
    h = fold32_special_f32(h, (float)ia);
    h = fold32_special_f32(h, (float)ib);

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

static double opaque_f64_special_f64(uint64_t bits, uint64_t s) {
    uint64_t b = (uint64_t)(bits + s);
    double f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t f64_to_bits_special_f64(double v) {
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static int64_t bits_to_i64_special_f64(uint64_t b) {
    int64_t v;
    memcpy(&v, &b, sizeof v);
    return v;
}

static uint64_t fold64_special_f64(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

static uint64_t special_f64(uint64_t seed) {
    uint64_t h = fold_init();

    const double one  = opaque_f64_special_f64(UINT64_C(0x3FF0000000000000), seed);
    const double mone = opaque_f64_special_f64(UINT64_C(0xBFF0000000000000), seed);
    const double pz   = opaque_f64_special_f64(UINT64_C(0x0000000000000000), seed);
    const double nz   = opaque_f64_special_f64(UINT64_C(0x8000000000000000), seed);
    const double pinf = opaque_f64_special_f64(UINT64_C(0x7FF0000000000000), seed);
    const double ninf = opaque_f64_special_f64(UINT64_C(0xFFF0000000000000), seed);
    const double nan_ = opaque_f64_special_f64(UINT64_C(0x7FF8000000000000), seed);
    const double dmax = opaque_f64_special_f64(UINT64_C(0x7FEFFFFFFFFFFFFF), seed);
    const double dmin = opaque_f64_special_f64(UINT64_C(0x0010000000000000), seed);
    const double denm = opaque_f64_special_f64(UINT64_C(0x000FFFFFFFFFFFFF), seed);
    const double tiny = opaque_f64_special_f64(UINT64_C(0x0000000000000001), seed);

    h = fold64_special_f64(h, pz);
    h = fold64_special_f64(h, nz);
    h = fold64_special_f64(h, pz + pz);
    h = fold64_special_f64(h, pz + nz);
    h = fold64_special_f64(h, nz + pz);
    h = fold64_special_f64(h, nz + nz);
    h = fold64_special_f64(h, pz - pz);
    h = fold64_special_f64(h, pz - nz);
    h = fold64_special_f64(h, nz - pz);
    h = fold64_special_f64(h, nz - nz);
    h = fold64_special_f64(h, pz * one);
    h = fold64_special_f64(h, pz * mone);
    h = fold64_special_f64(h, nz * one);
    h = fold64_special_f64(h, nz * mone);
    h = fold64_special_f64(h, nz * nz);
    h = fold64_special_f64(h, pz * nz);
    h = fold64_special_f64(h, -pz);
    h = fold64_special_f64(h, -nz);
    h = fold64_special_f64(h, pz / one);
    h = fold64_special_f64(h, pz / mone);
    h = fold64_special_f64(h, nz / one);
    h = fold64_special_f64(h, nz / mone);
    h = mix_u8(h, (uint8_t)(pz == nz));
    h = mix_u8(h, (uint8_t)(pz < nz));
    h = mix_u8(h, (uint8_t)(nz < pz));

    h = fold64_special_f64(h, pinf);
    h = fold64_special_f64(h, ninf);
    h = fold64_special_f64(h, -pinf);
    h = fold64_special_f64(h, one / pz);
    h = fold64_special_f64(h, one / nz);
    h = fold64_special_f64(h, mone / pz);
    h = fold64_special_f64(h, mone / nz);
    h = fold64_special_f64(h, pinf + one);
    h = fold64_special_f64(h, pinf + pinf);
    h = fold64_special_f64(h, pinf - ninf);
    h = fold64_special_f64(h, pinf * pinf);
    h = fold64_special_f64(h, pinf * ninf);
    h = fold64_special_f64(h, pinf * mone);
    h = fold64_special_f64(h, one / pinf);
    h = fold64_special_f64(h, mone / pinf);
    h = fold64_special_f64(h, one / ninf);
    h = fold64_special_f64(h, pinf / one);
    h = fold64_special_f64(h, dmax + dmax);
    h = mix_u8(h, (uint8_t)(pinf > dmax));
    h = mix_u8(h, (uint8_t)(ninf < (double)(0.0 - dmax)));

    h = fold64_special_f64(h, pinf - pinf);
    h = fold64_special_f64(h, ninf + pinf);
    h = fold64_special_f64(h, pinf * pz);
    h = fold64_special_f64(h, ninf * nz);
    h = fold64_special_f64(h, pinf / pinf);
    h = fold64_special_f64(h, pz / pz);
    h = fold64_special_f64(h, nz / pz);

    h = mix_u64(h, f64_to_bits_special_f64(nan_));
    h = mix_u8(h, (uint8_t)(nan_ != nan_));
    h = mix_u8(h, (uint8_t)(nan_ == nan_));
    h = mix_u8(h, (uint8_t)(nan_ < nan_));
    h = mix_u8(h, (uint8_t)(nan_ >= nan_));
    h = mix_u64(h, f64_to_bits_special_f64(-nan_));
    h = mix_u64(h, f64_to_bits_special_f64(-(double)(-nan_)));
    h = fold64_special_f64(h, nan_ + one);
    h = fold64_special_f64(h, one + nan_);
    h = fold64_special_f64(h, nan_ * pz);
    h = fold64_special_f64(h, nan_ - nan_);
    h = fold64_special_f64(h, nan_ / nan_);
    h = fold64_special_f64(h, nan_ / pinf);

    h = fold64_special_f64(h, tiny);
    h = fold64_special_f64(h, denm);
    h = fold64_special_f64(h, dmin);
    h = fold64_special_f64(h, denm + tiny);
    h = fold64_special_f64(h, dmin - tiny);
    h = fold64_special_f64(h, tiny + tiny);
    h = fold64_special_f64(h, tiny * 1.5);
    h = fold64_special_f64(h, tiny * 0.5);
    h = fold64_special_f64(h, -tiny);
    h = fold64_special_f64(h, tiny * 4503599627370496.0);
    h = fold64_special_f64(h, denm / dmin);
    h = mix_u8(h, (uint8_t)(tiny > pz));
    h = mix_u8(h, (uint8_t)(tiny == pz));
    h = mix_u8(h, (uint8_t)((double)(0.0 - tiny) < nz));

    double d = dmin;
    for (uint64_t i = 0; i < UINT64_C(60); i = (uint64_t)(i + UINT64_C(1))) {
        d = d * 0.5;
        h = fold64_special_f64(h, d);
    }

    const uint64_t rt[8] = {
        UINT64_C(0x0000000000000000), UINT64_C(0x8000000000000000),
        UINT64_C(0x0000000000000001), UINT64_C(0x7FF0000000000000),
        UINT64_C(0xFFF0000000000000), UINT64_C(0x7FF8000000000000),
        UINT64_C(0xFFF8000000C0DEAD), UINT64_C(0x7FF0000000000001)
    };
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        const double f = opaque_f64_special_f64(rt[k], seed);
        h = mix_u64(h, f64_to_bits_special_f64(f));
    }

    const float n32a = (float)dmax;
    const float n32b = (float)tiny;
    const float n32c = (float)opaque_f64_special_f64(UINT64_C(0x36A0000000000000), seed);
    const float n32d = (float)opaque_f64_special_f64(UINT64_C(0x3FF0000010000000), seed);
    const float n32e = (float)opaque_f64_special_f64(UINT64_C(0x3FF0000030000000), seed);
    const float n32f = (float)nz;
    const float n32g = (float)ninf;
    h = mix_f32(h, n32a);
    h = mix_f32(h, n32b);
    h = mix_f32(h, n32c);
    h = mix_f32(h, n32d);
    h = mix_f32(h, n32e);
    h = mix_f32(h, n32f);
    h = mix_f32(h, n32g);
    h = fold64_special_f64(h, (double)n32c);
    h = fold64_special_f64(h, (double)n32f);
    h = fold64_special_f64(h, (double)n32g);

    const uint64_t ua = (uint64_t)(UINT64_C(9007199254740992) + seed);
    const uint64_t ub = (uint64_t)(UINT64_C(9007199254740993) + seed);
    const uint64_t uc = (uint64_t)(UINT64_C(9007199254740995) + seed);
    const uint64_t ud = (uint64_t)(UINT64_C(18446744073709551615) + seed);
    const int64_t ia = bits_to_i64_special_f64((uint64_t)(UINT64_C(0xFFDFFFFFFFFFFFFF) + seed));
    const int64_t ib = bits_to_i64_special_f64((uint64_t)(UINT64_C(0x8000000000000000) + seed));
    h = fold64_special_f64(h, (double)ua);
    h = fold64_special_f64(h, (double)ub);
    h = fold64_special_f64(h, (double)uc);
    h = fold64_special_f64(h, (double)ud);
    h = fold64_special_f64(h, (double)seed);
    h = fold64_special_f64(h, (double)ia);
    h = fold64_special_f64(h, (double)ib);

    const double ca = 1.5 * one;
    const double cb = 9007199254740992.0 * one;
    const double cc = 4611686018427387904.0 * one;
    const double cd = 0.0 - 1.5 * one;
    const double ce = 0.5 * one;
    h = mix_u64(h, (uint64_t)ca);
    h = mix_u64(h, (uint64_t)cb);
    h = mix_u64(h, (uint64_t)cc);
    h = mix_u64(h, (uint64_t)ce);
    h = mix_u64(h, (uint64_t)pz);
    h = mix_i64(h, (int64_t)cd);
    h = mix_i64(h, (int64_t)cc);
    h = mix_i64(h, (int64_t)(double)(0.0 - ce));
    h = mix_i32(h, (int32_t)cd);
    h = mix_u32(h, (uint32_t)ca);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, special_f32(seed));
    h = mix_u64(h, special_f64(seed));
    return h;
}
