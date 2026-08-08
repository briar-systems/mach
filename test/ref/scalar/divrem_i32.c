/* addition, subtraction and multiplication run in the uint32_t bit-pattern
 * domain to stay clear of signed-overflow UB (per the reference rule); the
 * divisor and dividend are only ever read as int32_t at the point of / and %,
 * which is genuinely signed C division, well-defined because every divisor
 * here is nonzero and never -1, and every dividend stays clear of INT32_MIN. */
#include "corpus.h"

static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static inline uint32_t as_u32(int32_t v) { uint32_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t d = (uint32_t)(UINT32_C(2000000000) - s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(131) + UINT32_C(3) + s);
        const int32_t qi = (int32_t)(as_i32(d) / as_i32(v));
        const int32_t ri = (int32_t)(as_i32(d) % as_i32(v));
        h = mix_i32(h, qi);
        h = mix_i32(h, ri);
        h = mix_i32(h, as_i32((uint32_t)((uint32_t)(v * as_u32(qi)) + as_u32(ri))));
        d = (uint32_t)(d - v);
    }

    uint32_t e = (uint32_t)(UINT32_C(0) - (uint32_t)(UINT32_C(2000000000) + s));
    for (uint32_t i = 0; i < UINT32_C(8); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(197) + UINT32_C(5) + s);
        const int32_t qi = (int32_t)(as_i32(e) / as_i32(v));
        const int32_t ri = (int32_t)(as_i32(e) % as_i32(v));
        h = mix_i32(h, qi);
        h = mix_i32(h, ri);
        h = mix_i32(h, as_i32((uint32_t)((uint32_t)(v * as_u32(qi)) + as_u32(ri))));
        e = (uint32_t)(e + v);
    }

    const uint32_t big = (uint32_t)(UINT32_C(2147483644) + s);
    const uint32_t small = (uint32_t)(UINT32_C(3) + s);
    const int32_t q0i = (int32_t)(as_i32(big) / as_i32(small));
    const int32_t r0i = (int32_t)(as_i32(big) % as_i32(small));
    h = mix_i32(h, q0i);
    h = mix_i32(h, r0i);
    h = mix_i32(h, as_i32((uint32_t)((uint32_t)(small * as_u32(q0i)) + as_u32(r0i))));

    const int32_t q1i = (int32_t)(as_i32(small) / as_i32(big));
    const int32_t r1i = (int32_t)(as_i32(small) % as_i32(big));
    h = mix_i32(h, q1i);
    h = mix_i32(h, r1i);
    h = mix_i32(h, as_i32((uint32_t)((uint32_t)(big * as_u32(q1i)) + as_u32(r1i))));
    return h;
}
