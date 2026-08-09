/* addition, subtraction and multiplication run in the uint64_t bit-pattern
 * domain to stay clear of signed-overflow UB (per the reference rule); the
 * divisor and dividend are only ever read as int64_t at the point of / and %,
 * which is genuinely signed C division, well-defined because every divisor
 * here is nonzero and never -1, and every dividend stays clear of INT64_MIN. */
#include "corpus.h"

static inline int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }
static inline uint64_t as_u64(int64_t v) { uint64_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t d = (uint64_t)(UINT64_C(9000000000000000000) - s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(131) + UINT64_C(3) + s);
        const int64_t qi = (int64_t)(as_i64(d) / as_i64(v));
        const int64_t ri = (int64_t)(as_i64(d) % as_i64(v));
        h = mix_i64(h, qi);
        h = mix_i64(h, ri);
        h = mix_i64(h, as_i64((uint64_t)((uint64_t)(v * as_u64(qi)) + as_u64(ri))));
        d = (uint64_t)(d - v);
    }

    uint64_t e = (uint64_t)(UINT64_C(0) - (uint64_t)(UINT64_C(9000000000000000000) + s));
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(197) + UINT64_C(5) + s);
        const int64_t qi = (int64_t)(as_i64(e) / as_i64(v));
        const int64_t ri = (int64_t)(as_i64(e) % as_i64(v));
        h = mix_i64(h, qi);
        h = mix_i64(h, ri);
        h = mix_i64(h, as_i64((uint64_t)((uint64_t)(v * as_u64(qi)) + as_u64(ri))));
        e = (uint64_t)(e + v);
    }

    const uint64_t big = (uint64_t)(UINT64_C(9223372036854775804) + s);
    const uint64_t small = (uint64_t)(UINT64_C(3) + s);
    const int64_t q0i = (int64_t)(as_i64(big) / as_i64(small));
    const int64_t r0i = (int64_t)(as_i64(big) % as_i64(small));
    h = mix_i64(h, q0i);
    h = mix_i64(h, r0i);
    h = mix_i64(h, as_i64((uint64_t)((uint64_t)(small * as_u64(q0i)) + as_u64(r0i))));

    const int64_t q1i = (int64_t)(as_i64(small) / as_i64(big));
    const int64_t r1i = (int64_t)(as_i64(small) % as_i64(big));
    h = mix_i64(h, q1i);
    h = mix_i64(h, r1i);
    h = mix_i64(h, as_i64((uint64_t)((uint64_t)(big * as_u64(q1i)) + as_u64(r1i))));
    return h;
}
