#include "corpus.h"

__attribute__((noinline)) static uint64_t opaque(uint64_t v) {
    return v ^ UINT64_C(0x5A);
}

__attribute__((noinline)) static uint64_t floats64(uint64_t seed) {
    uint64_t h = fold_init();
    double acc = 0.0;
    for (uint64_t i = 0; i < UINT64_C(24); i = (uint64_t)(i + UINT64_C(1))) {
        const double x = (double)((uint64_t)(i * UINT64_C(7) + seed) & UINT64_C(255)) * 0.00390625;
        acc = acc + x * 1.5 + x * 2.25 + x * 3.125 + x * 4.0625 + x * 5.5
            + x * 6.75 + x * 7.875 + x * 8.5 + x * 9.25 + x * 10.75
            + x * 11.5 + x * 12.125 + x * 13.25 + x * 14.5 + x * 15.75
            + x * 16.5 + x * 17.25 + x * 18.75 + x * 19.5 + x * 20.125;
        h = mix_f64(h, acc);
    }
    return h;
}

__attribute__((noinline)) static uint64_t floats32(uint64_t seed) {
    uint64_t h = fold_init();
    float acc = 0.0f;
    for (uint64_t i = 0; i < UINT64_C(20); i = (uint64_t)(i + UINT64_C(1))) {
        const float x = (float)((uint64_t)(i * UINT64_C(5) + seed) & UINT64_C(63)) * 0.0625f;
        acc = acc * 0.5f + x * 1.25f + x * 2.5f + x * 3.75f + x * 4.5f + x * 5.25f
            + x * 6.5f + x * 7.75f + x * 8.25f + x * 9.5f + x * 10.25f
            + x * 11.75f + x * 12.5f + x * 13.25f + x * 14.75f + x * 15.5f
            + x * 16.25f + x * 17.5f;
        h = mix_f32(h, acc);
    }
    return h;
}

__attribute__((noinline)) static uint64_t wide(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t a = seed;
    uint64_t b = (uint64_t)(seed + UINT64_C(1));
    uint64_t c = (uint64_t)(seed + UINT64_C(2));
    uint64_t d = (uint64_t)(seed + UINT64_C(3));
    uint64_t e = (uint64_t)(seed + UINT64_C(4));
    uint64_t f = (uint64_t)(seed + UINT64_C(5));
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)((a ^ i) * UINT64_C(1099511628211));
        b = (uint64_t)((uint64_t)(b + a) * UINT64_C(11400714819323198485));
        c = (uint64_t)((c ^ b) * UINT64_C(14029467366897019727));
        d = (uint64_t)((uint64_t)(d + c) * UINT64_C(1609587929392839161));
        e = (uint64_t)((e ^ d) * UINT64_C(9650029242287828579));
        f = (uint64_t)((uint64_t)(f + e) * UINT64_C(2870177450012600261));
        a = a ^ UINT64_C(3337190745834213367);
        b = (uint64_t)(b + UINT64_C(5754853343627963903));
        c = c ^ UINT64_C(12297829382473034410);
        d = (uint64_t)(d + UINT64_C(6148914691236517205));
        e = e & UINT64_C(18446744069414584320);
        f = f | UINT64_C(1229782938247303441);
        h = mix_u64(h, a ^ b ^ c ^ d ^ e ^ f);
    }
    return h;
}

__attribute__((noinline)) static uint64_t nested(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t r = 0; r < UINT64_C(6); r = (uint64_t)(r + UINT64_C(1))) {
        uint64_t x = (uint64_t)((uint64_t)(r + seed) * UINT64_C(6364136223846793005));
        x = opaque(x) ^ UINT64_C(6364136223846793005);
        for (uint32_t c = 0; c < UINT32_C(5); c = (uint32_t)(c + UINT32_C(1))) {
            const uint32_t w = (uint32_t)((uint32_t)((uint32_t)x + c) * UINT32_C(305419896));
            h = mix_u32(h, (uint32_t)((w ^ UINT32_C(2596069104)) | UINT32_C(16777473)));
            h = mix_u64(h, (uint64_t)((x & UINT64_C(4294967295)) * UINT64_C(1099511627776)));
        }
        h = mix_u64(h, x);
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, floats64(seed));
    h = mix_u64(h, floats32(seed));
    h = mix_u64(h, wide(seed));
    h = mix_u64(h, nested(seed));
    return h;
}
