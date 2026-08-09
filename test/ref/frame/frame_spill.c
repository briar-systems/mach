#include "corpus.h"

/* every rotate amount is in [1, 63], so neither shift is by the width and the
 * rotate is defined rather than merely usual. */
static uint64_t rol(uint64_t x, unsigned n) {
    return (uint64_t)((uint64_t)(x << n) | (uint64_t)(x >> (64u - n)));
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a0 = (uint64_t)(UINT64_C(6364136223846793005) + s);
    uint64_t a1 = (uint64_t)(UINT64_C(1442695040888963407) ^ s);
    uint64_t a2 = (uint64_t)(UINT64_C(11400714819323198485) + s);
    uint64_t a3 = (uint64_t)(UINT64_C(14029467366897019727) ^ s);
    uint64_t a4 = (uint64_t)(UINT64_C(1609587929392839161) + s);
    uint64_t a5 = (uint64_t)(UINT64_C(9650029242287828579) ^ s);
    uint64_t a6 = (uint64_t)(UINT64_C(2870177450012600261) + s);
    uint64_t a7 = (uint64_t)(UINT64_C(8261357461348097633) ^ s);
    uint64_t a8 = (uint64_t)(UINT64_C(4823894509314446453) + s);
    uint64_t a9 = (uint64_t)(UINT64_C(3512524072905137951) ^ s);
    uint64_t a10 = (uint64_t)(UINT64_C(7046029254386353131) + s);
    uint64_t a11 = (uint64_t)(UINT64_C(5545126536417089969) ^ s);
    uint64_t a12 = (uint64_t)(UINT64_C(12297829382473034411) + s);
    uint64_t a13 = (uint64_t)(UINT64_C(15726070495360670683) ^ s);
    uint64_t a14 = (uint64_t)(UINT64_C(2246822519342508753) + s);
    uint64_t a15 = (uint64_t)(UINT64_C(3266489917871134591) ^ s);

    uint32_t w0 = (uint32_t)(UINT64_C(2654435761) + s);
    uint32_t w1 = (uint32_t)(UINT64_C(40503) ^ s);
    uint32_t w2 = (uint32_t)(UINT64_C(2246822519) + s);
    uint32_t w3 = (uint32_t)(UINT64_C(3266489917) ^ s);

    const double z = (double)seed;
    double f0 = 1.5 + z;
    double f1 = -2.25 + z;
    double f2 = 3.125 + z;
    double f3 = -0.75 + z;
    double f4 = 5.625 + z;
    double f5 = -1.375 + z;
    double f6 = 0.875 + z;
    double f7 = -4.5 + z;

    for (uint64_t i = 0; i < UINT64_C(24); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t t = (uint64_t)(a0 ^ rol(a15, 29u));
        a0 = (uint64_t)(a1 + rol(a0, 7u));
        a1 = (uint64_t)(a2 ^ rol(a1, 11u));
        a2 = (uint64_t)(a3 + rol(a2, 13u));
        a3 = (uint64_t)(a4 ^ rol(a3, 17u));
        a4 = (uint64_t)(a5 + rol(a4, 19u));
        a5 = (uint64_t)(a6 ^ rol(a5, 23u));
        a6 = (uint64_t)(a7 + rol(a6, 31u));
        a7 = (uint64_t)(a8 ^ rol(a7, 37u));
        a8 = (uint64_t)(a9 + rol(a8, 41u));
        a9 = (uint64_t)(a10 ^ rol(a9, 43u));
        a10 = (uint64_t)(a11 + rol(a10, 47u));
        a11 = (uint64_t)(a12 ^ rol(a11, 53u));
        a12 = (uint64_t)(a13 + rol(a12, 59u));
        a13 = (uint64_t)(a14 ^ rol(a13, 61u));
        a14 = (uint64_t)(a15 + rol(a14, 3u));
        a15 = (uint64_t)(t + i);

        const uint32_t u = (uint32_t)(w0 ^ (uint32_t)a12);
        w0 = (uint32_t)(w1 + (uint32_t)a0);
        w1 = (uint32_t)(w2 ^ (uint32_t)a4);
        w2 = (uint32_t)(w3 + (uint32_t)a8);
        w3 = u;

        const double g = f0;
        f0 = f0 * 1.5 + f1;
        f1 = f1 * 0.75 + f2;
        f2 = f2 * 1.25 + f3;
        f3 = f3 * 0.5 + f4;
        f4 = f4 * 1.75 + f5;
        f5 = f5 * 0.625 + f6;
        f6 = f6 * 1.125 + f7;
        f7 = f7 * 0.875 + g;

        h = mix_u64(h, (uint64_t)(a0 ^ a8));
        h = mix_u32(h, w0);
        h = mix_f64(h, f0 + f4);
    }

    h = mix_u64(h, a0);
    h = mix_u64(h, a1);
    h = mix_u64(h, a2);
    h = mix_u64(h, a3);
    h = mix_u64(h, a4);
    h = mix_u64(h, a5);
    h = mix_u64(h, a6);
    h = mix_u64(h, a7);
    h = mix_u64(h, a8);
    h = mix_u64(h, a9);
    h = mix_u64(h, a10);
    h = mix_u64(h, a11);
    h = mix_u64(h, a12);
    h = mix_u64(h, a13);
    h = mix_u64(h, a14);
    h = mix_u64(h, a15);
    h = mix_u32(h, w0);
    h = mix_u32(h, w1);
    h = mix_u32(h, w2);
    h = mix_u32(h, w3);
    h = mix_f64(h, f0);
    h = mix_f64(h, f1);
    h = mix_f64(h, f2);
    h = mix_f64(h, f3);
    h = mix_f64(h, f4);
    h = mix_f64(h, f5);
    h = mix_f64(h, f6);
    h = mix_f64(h, f7);
    return h;
}
