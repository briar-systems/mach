#include "corpus.h"

static double opaque_f64(uint64_t bits, uint64_t s) {
    uint64_t b = (uint64_t)(bits + s);
    double f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t f64_to_bits(double v) {
    uint64_t b;
    memcpy(&b, &v, sizeof b);
    return b;
}

static int64_t bits_to_i64(uint64_t b) {
    int64_t v;
    memcpy(&v, &b, sizeof v);
    return v;
}

static uint64_t fold64(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const double one  = opaque_f64(UINT64_C(0x3FF0000000000000), seed);
    const double mone = opaque_f64(UINT64_C(0xBFF0000000000000), seed);
    const double pz   = opaque_f64(UINT64_C(0x0000000000000000), seed);
    const double nz   = opaque_f64(UINT64_C(0x8000000000000000), seed);
    const double pinf = opaque_f64(UINT64_C(0x7FF0000000000000), seed);
    const double ninf = opaque_f64(UINT64_C(0xFFF0000000000000), seed);
    const double nan_ = opaque_f64(UINT64_C(0x7FF8000000000000), seed);
    const double dmax = opaque_f64(UINT64_C(0x7FEFFFFFFFFFFFFF), seed);
    const double dmin = opaque_f64(UINT64_C(0x0010000000000000), seed);
    const double denm = opaque_f64(UINT64_C(0x000FFFFFFFFFFFFF), seed);
    const double tiny = opaque_f64(UINT64_C(0x0000000000000001), seed);

    h = fold64(h, pz);
    h = fold64(h, nz);
    h = fold64(h, pz + pz);
    h = fold64(h, pz + nz);
    h = fold64(h, nz + pz);
    h = fold64(h, nz + nz);
    h = fold64(h, pz - pz);
    h = fold64(h, pz - nz);
    h = fold64(h, nz - pz);
    h = fold64(h, nz - nz);
    h = fold64(h, pz * one);
    h = fold64(h, pz * mone);
    h = fold64(h, nz * one);
    h = fold64(h, nz * mone);
    h = fold64(h, nz * nz);
    h = fold64(h, pz * nz);
    h = fold64(h, -pz);
    h = fold64(h, -nz);
    h = fold64(h, pz / one);
    h = fold64(h, pz / mone);
    h = fold64(h, nz / one);
    h = fold64(h, nz / mone);
    h = mix_u8(h, (uint8_t)(pz == nz));
    h = mix_u8(h, (uint8_t)(pz < nz));
    h = mix_u8(h, (uint8_t)(nz < pz));

    h = fold64(h, pinf);
    h = fold64(h, ninf);
    h = fold64(h, -pinf);
    h = fold64(h, one / pz);
    h = fold64(h, one / nz);
    h = fold64(h, mone / pz);
    h = fold64(h, mone / nz);
    h = fold64(h, pinf + one);
    h = fold64(h, pinf + pinf);
    h = fold64(h, pinf - ninf);
    h = fold64(h, pinf * pinf);
    h = fold64(h, pinf * ninf);
    h = fold64(h, pinf * mone);
    h = fold64(h, one / pinf);
    h = fold64(h, mone / pinf);
    h = fold64(h, one / ninf);
    h = fold64(h, pinf / one);
    h = fold64(h, dmax + dmax);
    h = mix_u8(h, (uint8_t)(pinf > dmax));
    h = mix_u8(h, (uint8_t)(ninf < (double)(0.0 - dmax)));

    h = fold64(h, pinf - pinf);
    h = fold64(h, ninf + pinf);
    h = fold64(h, pinf * pz);
    h = fold64(h, ninf * nz);
    h = fold64(h, pinf / pinf);
    h = fold64(h, pz / pz);
    h = fold64(h, nz / pz);

    h = mix_u64(h, f64_to_bits(nan_));
    h = mix_u8(h, (uint8_t)(nan_ != nan_));
    h = mix_u8(h, (uint8_t)(nan_ == nan_));
    h = mix_u8(h, (uint8_t)(nan_ < nan_));
    h = mix_u8(h, (uint8_t)(nan_ >= nan_));
    h = mix_u64(h, f64_to_bits(-nan_));
    h = mix_u64(h, f64_to_bits(-(double)(-nan_)));
    h = fold64(h, nan_ + one);
    h = fold64(h, one + nan_);
    h = fold64(h, nan_ * pz);
    h = fold64(h, nan_ - nan_);
    h = fold64(h, nan_ / nan_);
    h = fold64(h, nan_ / pinf);

    h = fold64(h, tiny);
    h = fold64(h, denm);
    h = fold64(h, dmin);
    h = fold64(h, denm + tiny);
    h = fold64(h, dmin - tiny);
    h = fold64(h, tiny + tiny);
    h = fold64(h, tiny * 1.5);
    h = fold64(h, tiny * 0.5);
    h = fold64(h, -tiny);
    h = fold64(h, tiny * 4503599627370496.0);
    h = fold64(h, denm / dmin);
    h = mix_u8(h, (uint8_t)(tiny > pz));
    h = mix_u8(h, (uint8_t)(tiny == pz));
    h = mix_u8(h, (uint8_t)((double)(0.0 - tiny) < nz));

    double d = dmin;
    for (uint64_t i = 0; i < UINT64_C(60); i = (uint64_t)(i + UINT64_C(1))) {
        d = d * 0.5;
        h = fold64(h, d);
    }

    const uint64_t rt[8] = {
        UINT64_C(0x0000000000000000), UINT64_C(0x8000000000000000),
        UINT64_C(0x0000000000000001), UINT64_C(0x7FF0000000000000),
        UINT64_C(0xFFF0000000000000), UINT64_C(0x7FF8000000000000),
        UINT64_C(0xFFF8000000C0DEAD), UINT64_C(0x7FF0000000000001)
    };
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        const double f = opaque_f64(rt[k], seed);
        h = mix_u64(h, f64_to_bits(f));
    }

    const float n32a = (float)dmax;
    const float n32b = (float)tiny;
    const float n32c = (float)opaque_f64(UINT64_C(0x36A0000000000000), seed);
    const float n32d = (float)opaque_f64(UINT64_C(0x3FF0000010000000), seed);
    const float n32e = (float)opaque_f64(UINT64_C(0x3FF0000030000000), seed);
    const float n32f = (float)nz;
    const float n32g = (float)ninf;
    h = mix_f32(h, n32a);
    h = mix_f32(h, n32b);
    h = mix_f32(h, n32c);
    h = mix_f32(h, n32d);
    h = mix_f32(h, n32e);
    h = mix_f32(h, n32f);
    h = mix_f32(h, n32g);
    h = fold64(h, (double)n32c);
    h = fold64(h, (double)n32f);
    h = fold64(h, (double)n32g);

    const uint64_t ua = (uint64_t)(UINT64_C(9007199254740992) + seed);
    const uint64_t ub = (uint64_t)(UINT64_C(9007199254740993) + seed);
    const uint64_t uc = (uint64_t)(UINT64_C(9007199254740995) + seed);
    const uint64_t ud = (uint64_t)(UINT64_C(18446744073709551615) + seed);
    const int64_t ia = bits_to_i64((uint64_t)(UINT64_C(0xFFDFFFFFFFFFFFFF) + seed));
    const int64_t ib = bits_to_i64((uint64_t)(UINT64_C(0x8000000000000000) + seed));
    h = fold64(h, (double)ua);
    h = fold64(h, (double)ub);
    h = fold64(h, (double)uc);
    h = fold64(h, (double)ud);
    h = fold64(h, (double)seed);
    h = fold64(h, (double)ia);
    h = fold64(h, (double)ib);

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
