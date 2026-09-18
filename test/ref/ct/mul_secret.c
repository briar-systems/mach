#include "corpus.h"

static uint64_t fold8(uint64_t h, uint64_t seed) {
    const uint8_t s = (uint8_t)seed;
    const uint8_t ops[8] = {UINT8_C(0), UINT8_C(1), UINT8_C(255), UINT8_C(128), (uint8_t)(UINT8_C(90) ^ s), (uint8_t)(UINT8_C(195) ^ s), (uint8_t)(UINT8_C(7) ^ s), (uint8_t)(UINT8_C(254) ^ s)};
    uint64_t r = h;
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            const uint8_t a = ops[i];
            const uint8_t b = ops[j];
            r = mix_u8(r, (uint8_t)((uint32_t)a * (uint32_t)b));
            r = mix_u8(r, (uint8_t)((uint32_t)a * (uint32_t)b));
        }
    }
    return r;
}

static uint64_t fold16(uint64_t h, uint64_t seed) {
    const uint16_t s = (uint16_t)seed;
    const uint16_t ops[8] = {UINT16_C(0), UINT16_C(1), UINT16_C(65535), UINT16_C(32768), (uint16_t)(UINT16_C(23130) ^ s), (uint16_t)(UINT16_C(50115) ^ s), (uint16_t)(UINT16_C(7) ^ s), (uint16_t)(UINT16_C(65534) ^ s)};
    uint64_t r = h;
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            const uint16_t a = ops[i];
            const uint16_t b = ops[j];
            r = mix_u16(r, (uint16_t)((uint32_t)a * (uint32_t)b));
            r = mix_u16(r, (uint16_t)((uint32_t)a * (uint32_t)b));
        }
    }
    return r;
}

static uint64_t fold32(uint64_t h, uint64_t seed) {
    const uint32_t s = (uint32_t)seed;
    const uint32_t ops[8] = {UINT32_C(0), UINT32_C(1), UINT32_C(4294967295), UINT32_C(2147483648), (uint32_t)(UINT32_C(1515870810) ^ s), (uint32_t)(UINT32_C(3284386755) ^ s), (uint32_t)(UINT32_C(7) ^ s), (uint32_t)(UINT32_C(4294967294) ^ s)};
    uint64_t r = h;
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            const uint32_t a = ops[i];
            const uint32_t b = ops[j];
            r = mix_u32(r, (uint32_t)((uint32_t)a * (uint32_t)b));
            r = mix_u32(r, (uint32_t)((uint32_t)a * (uint32_t)b));
        }
    }
    return r;
}

static uint64_t fold64(uint64_t h, uint64_t seed) {
    const uint64_t ops[8] = {UINT64_C(0), UINT64_C(1), UINT64_C(18446744073709551615), UINT64_C(9223372036854775808), (uint64_t)(UINT64_C(6510615555426900570) ^ seed), (uint64_t)(UINT64_C(14106333703424951235) ^ seed), (uint64_t)(UINT64_C(7) ^ seed), (uint64_t)(UINT64_C(18446744073709551614) ^ seed)};
    uint64_t r = h;
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            const uint64_t a = ops[i];
            const uint64_t b = ops[j];
            r = mix_u64(r, (uint64_t)((uint64_t)a * (uint64_t)b));
            r = mix_u64(r, (uint64_t)((uint64_t)a * (uint64_t)b));
        }
    }
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = fold8(h, seed);
    h = fold16(h, seed);
    h = fold32(h, seed);
    h = fold64(h, seed);
    return h;
}
