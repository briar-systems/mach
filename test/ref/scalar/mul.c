/* products that exceed the width at 32 and 64 bits, signed and unsigned, seeded so the
 * wrap cannot fold. one noinline part per type. */
#include "corpus.h"

static uint64_t mul_u32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(2654435761) + s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a * (uint32_t)(UINT32_C(2) * i + UINT32_C(3)));
        h = mix_u32(h, a);
    }

    uint32_t b = (uint32_t)(UINT32_C(4294967231) + s);
    h = mix_u32(h, (uint32_t)(b * b));
    h = mix_u32(h, (uint32_t)(b * UINT32_C(4294967295)));
    h = mix_u32(h, (uint32_t)(a * b));
    h = mix_u32(h, (uint32_t)(b * a));

    uint32_t c = (uint32_t)(UINT32_C(65537) + s);
    h = mix_u32(h, (uint32_t)(c * c));
    h = mix_u32(h, (uint32_t)(c * UINT32_C(65535)));
    return h;
}

/* a signed product's wrap is bit-identical to the unsigned product of the same
 * bit patterns, so the multiply and add steps run in the uint32_t domain and
 * only bitcast to int32_t at the fold call, per the reference rule against
 * signed overflow UB. */
static inline int32_t as_i32_mul_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t mul_i32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(2654435769) + s); /* bits of -1640531527 */
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a * (uint32_t)(UINT32_C(2) * i + UINT32_C(3)));
        h = mix_i32(h, as_i32_mul_i32(a));
    }

    uint32_t b = (uint32_t)(UINT32_C(4294967231) + s); /* bits of -65 */
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(b * b)));
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(b * UINT32_C(4294967295))));
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(a * b)));
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(b * a)));

    uint32_t c = (uint32_t)(UINT32_C(65537) + s);
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(c * c)));
    h = mix_i32(h, as_i32_mul_i32((uint32_t)(c * UINT32_C(4294967295))));
    return h;
}

static uint64_t mul_u64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(11400714819323198485) + s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a * (uint64_t)(UINT64_C(2) * i + UINT64_C(3)));
        h = mix_u64(h, a);
    }

    uint64_t b = (uint64_t)(UINT64_C(18446744073709551553) + s);
    h = mix_u64(h, (uint64_t)(b * b));
    h = mix_u64(h, (uint64_t)(b * UINT64_C(18446744073709551615)));
    h = mix_u64(h, (uint64_t)(a * b));
    h = mix_u64(h, (uint64_t)(b * a));

    uint64_t c = (uint64_t)(UINT64_C(4294967311) + s);
    h = mix_u64(h, (uint64_t)(c * c));
    h = mix_u64(h, (uint64_t)(c * UINT64_C(4294967295)));
    return h;
}

/* a signed product's wrap is bit-identical to the unsigned product of the same
 * bit patterns, so the multiply and add steps run in the uint64_t domain and
 * only bitcast to int64_t at the fold call, per the reference rule against
 * signed overflow UB. */
static inline int64_t as_i64_mul_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t mul_i64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(11400714819323198485) + s); /* bits of -7046029254386353131 */
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a * (uint64_t)(UINT64_C(2) * i + UINT64_C(3)));
        h = mix_i64(h, as_i64_mul_i64(a));
    }

    uint64_t b = (uint64_t)(UINT64_C(18446744073709551553) + s); /* bits of -63 */
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(b * b)));
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(b * UINT64_C(18446744073709551615))));
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(a * b)));
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(b * a)));

    uint64_t c = (uint64_t)(UINT64_C(4294967311) + s);
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(c * c)));
    h = mix_i64(h, as_i64_mul_i64((uint64_t)(c * UINT64_C(18446744073709551615))));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, mul_u32(seed));
    h = mix_u64(h, mul_i32(seed));
    h = mix_u64(h, mul_u64(seed));
    h = mix_u64(h, mul_i64(seed));
    return h;
}
