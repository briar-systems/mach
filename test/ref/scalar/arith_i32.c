/* signed add/sub/negate wraps in two's complement, identical to arith_u32 at the
 * bit level, so every step is done on the uint32_t bit pattern and only bitcast
 * to int32_t at the fold call, per the reference rule against signed overflow UB. */
#include "corpus.h"

static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(4294967280) + s);
    for (uint32_t i = 0; i < UINT32_C(32); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a + (uint32_t)(i * UINT32_C(7) + UINT32_C(1)));
        h = mix_i32(h, as_i32(a));
        a = (uint32_t)(a - (uint32_t)(i * UINT32_C(3) + UINT32_C(2)));
        h = mix_i32(h, as_i32(a));
    }

    uint32_t b = s;
    for (uint32_t j = 0; j < UINT32_C(8); j = (uint32_t)(j + UINT32_C(1))) {
        b = (uint32_t)(b - (uint32_t)(j * UINT32_C(305419896) + UINT32_C(1)));
        h = mix_i32(h, as_i32(b));
    }

    const uint32_t n = (uint32_t)(UINT32_C(0) - a);
    h = mix_i32(h, as_i32(n));
    h = mix_i32(h, as_i32((uint32_t)(UINT32_C(0) - n)));
    h = mix_i32(h, as_i32((uint32_t)(UINT32_C(0) - s)));
    h = mix_i32(h, as_i32((uint32_t)(UINT32_C(2147483647) + a)));
    h = mix_i32(h, as_i32((uint32_t)(UINT32_C(2147483648) - a)));
    h = mix_i32(h, as_i32((uint32_t)(UINT32_C(4294967295) + a)));
    h = mix_i32(h, as_i32((uint32_t)(a + b)));
    h = mix_i32(h, as_i32((uint32_t)(a - b)));
    h = mix_i32(h, as_i32((uint32_t)(b - a)));
    return h;
}
