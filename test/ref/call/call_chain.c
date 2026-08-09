#include "corpus.h"

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static double bit(uint64_t x, uint64_t j) {
    return (double)((x >> j) & UINT64_C(1023)) / 8.0;
}

static float bit32(uint64_t x, uint64_t j) {
    return (float)((x >> j) & UINT64_C(255)) / 4.0f;
}

static uint64_t burn(uint64_t x) {
    uint64_t a0 = (uint64_t)(x ^ UINT64_C(1));
    uint64_t a1 = (uint64_t)(x * UINT64_C(3) + UINT64_C(2));
    uint64_t a2 = (uint64_t)(~x + UINT64_C(3));
    uint64_t a3 = (uint64_t)(x * UINT64_C(5) + UINT64_C(4));
    uint64_t a4 = (uint64_t)(x ^ UINT64_C(2863311530));
    uint64_t a5 = (uint64_t)(x * UINT64_C(7) + UINT64_C(6));
    uint64_t a6 = (uint64_t)((~x) * UINT64_C(9));
    uint64_t a7 = (uint64_t)(x + UINT64_C(305419896));
    uint64_t a8 = (uint64_t)(x * UINT64_C(11) + UINT64_C(10));
    uint64_t a9 = (uint64_t)(x ^ UINT64_C(1431655765));
    uint64_t a10 = (uint64_t)(x * UINT64_C(13) + UINT64_C(12));
    uint64_t a11 = (uint64_t)((~x) ^ x);
    double b0 = bit(x, 0);
    double b1 = bit(x, 5);
    double b2 = bit(x, 11);
    float b3 = bit32(x, 17);
    float b4 = bit32(x, 23);
    float b5 = bit32(x, 29);

    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        a0 = (uint64_t)(a0 + a1);
        a1 = (uint64_t)(a1 ^ a2);
        a2 = (uint64_t)(a2 + a3);
        a3 = (uint64_t)(a3 ^ a4);
        a4 = (uint64_t)(a4 + a5);
        a5 = (uint64_t)(a5 ^ a6);
        a6 = (uint64_t)(a6 + a7);
        a7 = (uint64_t)(a7 ^ a8);
        a8 = (uint64_t)(a8 + a9);
        a9 = (uint64_t)(a9 ^ a10);
        a10 = (uint64_t)(a10 + a11);
        a11 = (uint64_t)(a11 ^ a0);
        b0 = b0 + b1;
        b1 = b1 - b2;
        b2 = b2 + b0;
        b3 = b3 + b4;
        b4 = b4 - b5;
        b5 = b5 + b3;
    }

    uint64_t h = fold_init();
    h = mix_u64(h, (uint64_t)(a0 ^ a1 ^ a2 ^ a3 ^ a4 ^ a5));
    h = mix_u64(h, (uint64_t)(a6 ^ a7 ^ a8 ^ a9 ^ a10 ^ a11));
    h = mix_f64(h, b0 + b1 + b2);
    h = mix_f32(h, b3 + b4 + b5);
    return h;
}

static uint64_t chain(uint64_t n, uint64_t x, uint64_t h) {
    const uint64_t c0 = (uint64_t)(x * UINT64_C(3) + UINT64_C(1));
    const uint64_t c1 = (uint64_t)(x ^ UINT64_C(305419896));
    const uint64_t c2 = (uint64_t)(~x);
    const uint64_t c3 = (uint64_t)(x * UINT64_C(5) + n);
    const uint64_t c4 = (uint64_t)(x + UINT64_C(2863311530));
    const uint64_t c5 = (uint64_t)(x * UINT64_C(7) + UINT64_C(3));
    const uint64_t c6 = (uint64_t)(x ^ UINT64_C(1431655765));
    const uint64_t c7 = (uint64_t)(x * UINT64_C(9) + n);
    const uint64_t c8 = (uint64_t)(~x + UINT64_C(11));
    const uint64_t c9 = (uint64_t)(x * UINT64_C(13) + UINT64_C(5));
    const uint64_t c10 = (uint64_t)(x ^ n);
    const uint64_t c11 = (uint64_t)(x * UINT64_C(17) + UINT64_C(7));
    const double d0 = bit(x, 1);
    const double d1 = bit(x, 9);
    const double d2 = bit(x, 19);
    const float d3 = bit32(x, 3);
    const float d4 = bit32(x, 13);
    const float d5 = bit32(x, 25);

    uint64_t a = h;
    if (n > 0) {
        a = chain((uint64_t)(n - UINT64_C(1)), (uint64_t)(x * UINT64_C(3) + n), a);
    } else {
        a = mix_u64(a, burn(x));
    }

    a = mix_u64(a, c0);
    a = mix_u64(a, c1);
    a = mix_u64(a, c2);
    a = mix_u64(a, c3);
    a = mix_u64(a, c4);
    a = mix_u64(a, c5);
    a = mix_u64(a, c6);
    a = mix_u64(a, c7);
    a = mix_u64(a, c8);
    a = mix_u64(a, c9);
    a = mix_u64(a, c10);
    a = mix_u64(a, c11);
    a = mix_f64(a, d0);
    a = mix_f64(a, d1);
    a = mix_f64(a, d2);
    a = mix_f32(a, d3);
    a = mix_f32(a, d4);
    a = mix_f32(a, d5);
    a = mix_f64(a, d0 + d1 - d2);
    a = mix_f32(a, d3 + d4 - d5);
    return mix_u64(a, (uint64_t)(c0 ^ c5 ^ c11));
}

static uint64_t stage_d(uint64_t x, uint64_t h) {
    const uint64_t e0 = (uint64_t)(x + UINT64_C(1));
    const uint64_t e1 = (uint64_t)(x * UINT64_C(3));
    const uint64_t e2 = (uint64_t)(~x);
    const double e3 = bit(x, 2);
    const float e4 = bit32(x, 7);
    uint64_t a = mix_u64(h, burn((uint64_t)(x ^ UINT64_C(12345))));
    a = mix_u64(a, e0);
    a = mix_u64(a, e1);
    a = mix_u64(a, e2);
    a = mix_f64(a, e3);
    return mix_f32(a, e4);
}

static uint64_t stage_c(uint64_t x, uint64_t h) {
    const uint64_t e0 = (uint64_t)(x ^ UINT64_C(11));
    const uint64_t e1 = (uint64_t)(x * UINT64_C(5) + UINT64_C(2));
    const uint64_t e2 = (uint64_t)(~x + UINT64_C(7));
    const uint64_t e3 = (uint64_t)(x + UINT64_C(2863311530));
    const uint64_t e4 = (uint64_t)(x * UINT64_C(9));
    const double e5 = bit(x, 4);
    const double e6 = bit(x, 14);
    const float e7 = bit32(x, 21);
    uint64_t a = stage_d((uint64_t)(x * UINT64_C(7) + UINT64_C(1)), h);
    a = mix_u64(a, e0);
    a = mix_u64(a, e1);
    a = mix_u64(a, e2);
    a = mix_u64(a, e3);
    a = mix_u64(a, e4);
    a = mix_f64(a, e5);
    a = mix_f64(a, e6);
    a = mix_f32(a, e7);
    return mix_u64(a, (uint64_t)(e0 ^ e4));
}

static uint64_t stage_b(uint64_t x, uint64_t h) {
    const uint64_t e0 = (uint64_t)(x * UINT64_C(13) + UINT64_C(3));
    const uint64_t e1 = (uint64_t)(x ^ UINT64_C(1431655765));
    const uint64_t e2 = (uint64_t)((~x) * UINT64_C(3));
    const uint64_t e3 = (uint64_t)(x + UINT64_C(17));
    const uint64_t e4 = (uint64_t)(x * UINT64_C(19));
    const uint64_t e5 = (uint64_t)(x ^ UINT64_C(305419896));
    const uint64_t e6 = (uint64_t)(x + UINT64_C(23));
    const double e7 = bit(x, 6);
    const double e8 = bit(x, 16);
    const double e9 = bit(x, 26);
    const float e10 = bit32(x, 9);
    const float e11 = bit32(x, 27);
    uint64_t a = stage_c((uint64_t)(x + UINT64_C(29)), h);
    a = mix_u64(a, e0);
    a = mix_u64(a, e1);
    a = mix_u64(a, e2);
    a = mix_u64(a, e3);
    a = mix_u64(a, e4);
    a = mix_u64(a, e5);
    a = mix_u64(a, e6);
    a = mix_f64(a, e7);
    a = mix_f64(a, e8);
    a = mix_f64(a, e9);
    a = mix_f32(a, e10);
    a = mix_f32(a, e11);
    return mix_u64(a, (uint64_t)(e0 + e3 + e6));
}

static uint64_t stage_a(uint64_t x, uint64_t h) {
    const uint64_t e0 = (uint64_t)(x + UINT64_C(31));
    const uint64_t e1 = (uint64_t)(x * UINT64_C(37));
    const uint64_t e2 = (uint64_t)((~x) ^ UINT64_C(41));
    const uint64_t e3 = (uint64_t)(x * UINT64_C(43) + UINT64_C(2));
    const uint64_t e4 = (uint64_t)(x + UINT64_C(47));
    const uint64_t e5 = (uint64_t)(x ^ UINT64_C(2863311530));
    const uint64_t e6 = (uint64_t)(x * UINT64_C(53));
    const uint64_t e7 = (uint64_t)(~x + UINT64_C(59));
    const uint64_t e8 = (uint64_t)(x * UINT64_C(61) + UINT64_C(4));
    const double e9 = bit(x, 8);
    const double e10 = bit(x, 18);
    const float e11 = bit32(x, 11);
    const float e12 = bit32(x, 19);
    uint64_t a = stage_b((uint64_t)(x * UINT64_C(3) + UINT64_C(5)), h);
    a = mix_u64(a, e0);
    a = mix_u64(a, e1);
    a = mix_u64(a, e2);
    a = mix_u64(a, e3);
    a = mix_u64(a, e4);
    a = mix_u64(a, e5);
    a = mix_u64(a, e6);
    a = mix_u64(a, e7);
    a = mix_u64(a, e8);
    a = mix_f64(a, e9);
    a = mix_f64(a, e10);
    a = mix_f32(a, e11);
    a = mix_f32(a, e12);
    return mix_u64(a, (uint64_t)(e0 ^ e4 ^ e8));
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[6];
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    h = chain((uint64_t)(UINT64_C(16) + seed), v[0], h);
    h = chain((uint64_t)(UINT64_C(1) + seed), v[1], h);

    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        h = stage_a((uint64_t)(v[k] + seed), h);
    }

    h = mix_u64(h, burn((uint64_t)(v[5] + seed)));
    return h;
}
