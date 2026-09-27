/* division and remainder by constants at 8, 16, 32 and 64 bits, signed and
 * unsigned (#3350). generated: one noinline part per type. */
#include "corpus.h"

static uint64_t divconst_u8(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t i = 0; i < UINT64_C(256); i = i + 1) {
        const uint8_t xu = (uint8_t)(i + seed);
        const uint8_t x = xu;
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(2)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(2)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(3)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(3)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(5)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(5)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(7)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(7)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(10)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(10)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(16)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(16)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(25)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(25)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(127)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(127)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(128)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(128)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(129)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(129)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(255)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(255)));
        h = mix_u8(h, (uint8_t)(x / (uint8_t)UINT64_C(85)));
        h = mix_u8(h, (uint8_t)(x % (uint8_t)UINT64_C(85)));
    }
    return h;
}

static uint64_t divconst_i8(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t i = 0; i < UINT64_C(256); i = i + 1) {
        const uint8_t xu = (uint8_t)(i + seed);
        int8_t x; memcpy(&x, &xu, sizeof x);
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(2)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(2)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(3)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(3)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(7)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(7)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(10)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(10)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(64)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(64)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-3)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-3)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-7)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-7)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-10)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-10)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-2)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-2)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-16)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-16)));
        h = mix_i8(h, (int8_t)(x / (int8_t)(-127 - 1)));
        h = mix_i8(h, (int8_t)(x % (int8_t)(-127 - 1)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-127)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-127)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(127)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(127)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(126)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(126)));
        h = mix_i8(h, (int8_t)(x / (int8_t)INT64_C(-65)));
        h = mix_i8(h, (int8_t)(x % (int8_t)INT64_C(-65)));
    }
    return h;
}

static uint64_t divconst_u16(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t i = 0; i < UINT64_C(65536); i = i + 1) {
        const uint16_t xu = (uint16_t)(i + seed);
        const uint16_t x = xu;
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(2)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(2)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(3)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(3)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(5)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(5)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(7)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(7)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(10)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(10)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(16)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(16)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(25)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(25)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(641)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(641)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(32767)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(32767)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(32768)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(32768)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(32769)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(32769)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(65535)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(65535)));
        h = mix_u16(h, (uint16_t)(x / (uint16_t)UINT64_C(21845)));
        h = mix_u16(h, (uint16_t)(x % (uint16_t)UINT64_C(21845)));
    }
    return h;
}

static uint64_t divconst_i16(uint64_t seed) {
    uint64_t h = fold_init();
    for (uint64_t i = 0; i < UINT64_C(65536); i = i + 1) {
        const uint16_t xu = (uint16_t)(i + seed);
        int16_t x; memcpy(&x, &xu, sizeof x);
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(2)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(2)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(3)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(3)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(7)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(7)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(10)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(10)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(64)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(64)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-3)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-3)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-7)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-7)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-10)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-10)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-641)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-641)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-2)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-2)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-16)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-16)));
        h = mix_i16(h, (int16_t)(x / (int16_t)(-32767 - 1)));
        h = mix_i16(h, (int16_t)(x % (int16_t)(-32767 - 1)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-32767)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-32767)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(32767)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(32767)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(32766)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(32766)));
        h = mix_i16(h, (int16_t)(x / (int16_t)INT64_C(-16385)));
        h = mix_i16(h, (int16_t)(x % (int16_t)INT64_C(-16385)));
    }
    return h;
}

static uint64_t divconst_u32(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t v = UINT64_C(0x9E3779B97F4A7C15) + seed;
    for (uint64_t i = 0; i < UINT64_C(2048); i = i + 1) {
        v = v * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
        uint64_t xu = v >> (i & 31);
        if (i < 8) { xu = (UINT64_C(0) - 4 + i) >> (64 - 32); }
        if (i >= 8 && i < 16) { xu = (i - 8) << (32 - 3); }
        const uint32_t xn = (uint32_t)xu;
        const uint32_t x = xn;
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(2)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(2)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(3)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(3)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(5)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(5)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(7)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(7)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(10)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(10)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(16)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(16)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(25)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(25)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(641)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(641)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(1000000007)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(1000000007)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(6700417)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(6700417)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(2147483647)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(2147483647)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(2147483648)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(2147483648)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(2147483649)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(2147483649)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(4294967295)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(4294967295)));
        h = mix_u32(h, (uint32_t)(x / (uint32_t)UINT64_C(1431655765)));
        h = mix_u32(h, (uint32_t)(x % (uint32_t)UINT64_C(1431655765)));
    }
    return h;
}

static uint64_t divconst_i32(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t v = UINT64_C(0x9E3779B97F4A7C15) + seed;
    for (uint64_t i = 0; i < UINT64_C(2048); i = i + 1) {
        v = v * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
        uint64_t xu = v >> (i & 31);
        if (i < 8) { xu = (UINT64_C(0) - 4 + i) >> (64 - 32); }
        if (i >= 8 && i < 16) { xu = (i - 8) << (32 - 3); }
        const uint32_t xn = (uint32_t)xu;
        int32_t x; memcpy(&x, &xn, sizeof x);
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(2)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(2)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(3)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(3)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(7)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(7)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(10)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(10)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(64)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(64)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(1000000007)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(1000000007)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-3)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-3)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-7)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-7)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-10)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-10)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-641)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-641)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-2)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-2)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-16)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-16)));
        h = mix_i32(h, (int32_t)(x / (int32_t)(-2147483647 - 1)));
        h = mix_i32(h, (int32_t)(x % (int32_t)(-2147483647 - 1)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-2147483647)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-2147483647)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(2147483647)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(2147483647)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(2147483646)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(2147483646)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(-1073741825)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(-1073741825)));
        h = mix_i32(h, (int32_t)(x / (int32_t)INT64_C(6700417)));
        h = mix_i32(h, (int32_t)(x % (int32_t)INT64_C(6700417)));
    }
    return h;
}

static uint64_t divconst_u64(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t v = UINT64_C(0x9E3779B97F4A7C15) + seed;
    for (uint64_t i = 0; i < UINT64_C(2048); i = i + 1) {
        v = v * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
        uint64_t xu = v >> (i & 31);
        if (i < 8) { xu = (UINT64_C(0) - 4 + i) >> (64 - 64); }
        if (i >= 8 && i < 16) { xu = (i - 8) << (64 - 3); }
        const uint64_t xn = (uint64_t)xu;
        const uint64_t x = xn;
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(2)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(2)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(3)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(3)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(5)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(5)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(7)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(7)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(10)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(10)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(16)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(16)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(25)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(25)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(641)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(641)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(1000000007)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(1000000007)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(6700417)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(6700417)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(9223372036854775807)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(9223372036854775807)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(9223372036854775808)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(9223372036854775808)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(9223372036854775809)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(9223372036854775809)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(18446744073709551615)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(18446744073709551615)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(6148914691236517205)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(6148914691236517205)));
        h = mix_u64(h, (uint64_t)(x / (uint64_t)UINT64_C(12297829382473034411)));
        h = mix_u64(h, (uint64_t)(x % (uint64_t)UINT64_C(12297829382473034411)));
    }
    return h;
}

static uint64_t divconst_i64(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t v = UINT64_C(0x9E3779B97F4A7C15) + seed;
    for (uint64_t i = 0; i < UINT64_C(2048); i = i + 1) {
        v = v * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
        uint64_t xu = v >> (i & 31);
        if (i < 8) { xu = (UINT64_C(0) - 4 + i) >> (64 - 64); }
        if (i >= 8 && i < 16) { xu = (i - 8) << (64 - 3); }
        const uint64_t xn = (uint64_t)xu;
        int64_t x; memcpy(&x, &xn, sizeof x);
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(2)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(2)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(3)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(3)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(7)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(7)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(10)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(10)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(64)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(64)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(1000000007)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(1000000007)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-3)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-3)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-7)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-7)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-10)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-10)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-641)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-641)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-2)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-2)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-16)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-16)));
        h = mix_i64(h, (int64_t)(x / (INT64_MIN)));
        h = mix_i64(h, (int64_t)(x % (INT64_MIN)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-9223372036854775807)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-9223372036854775807)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(9223372036854775807)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(9223372036854775807)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(9223372036854775806)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(9223372036854775806)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(-4611686018427387905)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(-4611686018427387905)));
        h = mix_i64(h, (int64_t)(x / (int64_t)INT64_C(6700417)));
        h = mix_i64(h, (int64_t)(x % (int64_t)INT64_C(6700417)));
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, divconst_u8(seed));
    h = mix_u64(h, divconst_i8(seed));
    h = mix_u64(h, divconst_u16(seed));
    h = mix_u64(h, divconst_i16(seed));
    h = mix_u64(h, divconst_u32(seed));
    h = mix_u64(h, divconst_i32(seed));
    h = mix_u64(h, divconst_u64(seed));
    h = mix_u64(h, divconst_i64(seed));
    return h;
}
