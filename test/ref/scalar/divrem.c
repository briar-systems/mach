/* division and remainder at 32 and 64 bits, signed and unsigned, folding q, r and d*q + r.
 * one noinline part per type. */
#include "corpus.h"

static uint64_t divrem_u32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t d = (uint32_t)(UINT32_C(4294967295) - s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(131) + UINT32_C(3) + s);
        const uint32_t q = (uint32_t)(d / v);
        const uint32_t r = (uint32_t)(d % v);
        h = mix_u32(h, q);
        h = mix_u32(h, r);
        h = mix_u32(h, (uint32_t)((uint32_t)(v * q) + r));
        d = (uint32_t)(d - v);
    }

    const uint32_t big = (uint32_t)(UINT32_C(4294967293) + s);
    const uint32_t small = (uint32_t)(UINT32_C(3) + s);
    const uint32_t q0 = (uint32_t)(big / small);
    const uint32_t r0 = (uint32_t)(big % small);
    h = mix_u32(h, q0);
    h = mix_u32(h, r0);
    h = mix_u32(h, (uint32_t)((uint32_t)(small * q0) + r0));

    const uint32_t q1 = (uint32_t)(small / big);
    const uint32_t r1 = (uint32_t)(small % big);
    h = mix_u32(h, q1);
    h = mix_u32(h, r1);
    h = mix_u32(h, (uint32_t)((uint32_t)(big * q1) + r1));
    return h;
}

/* addition, subtraction and multiplication run in the uint32_t bit-pattern
 * domain to stay clear of signed-overflow UB (per the reference rule); the
 * divisor and dividend are only ever read as int32_t at the point of / and %,
 * which is genuinely signed C division, well-defined because every divisor
 * here is nonzero and never -1, and every dividend stays clear of INT32_MIN. */
static inline int32_t as_i32_divrem_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }
static inline uint32_t as_u32_divrem_i32(int32_t v) { uint32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t divrem_i32(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t d = (uint32_t)(UINT32_C(2000000000) - s);
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(131) + UINT32_C(3) + s);
        const int32_t qi = (int32_t)(as_i32_divrem_i32(d) / as_i32_divrem_i32(v));
        const int32_t ri = (int32_t)(as_i32_divrem_i32(d) % as_i32_divrem_i32(v));
        h = mix_i32(h, qi);
        h = mix_i32(h, ri);
        h = mix_i32(h, as_i32_divrem_i32((uint32_t)((uint32_t)(v * as_u32_divrem_i32(qi)) + as_u32_divrem_i32(ri))));
        d = (uint32_t)(d - v);
    }

    uint32_t e = (uint32_t)(UINT32_C(0) - (uint32_t)(UINT32_C(2000000000) + s));
    for (uint32_t i = 0; i < UINT32_C(8); i = (uint32_t)(i + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(i * UINT32_C(197) + UINT32_C(5) + s);
        const int32_t qi = (int32_t)(as_i32_divrem_i32(e) / as_i32_divrem_i32(v));
        const int32_t ri = (int32_t)(as_i32_divrem_i32(e) % as_i32_divrem_i32(v));
        h = mix_i32(h, qi);
        h = mix_i32(h, ri);
        h = mix_i32(h, as_i32_divrem_i32((uint32_t)((uint32_t)(v * as_u32_divrem_i32(qi)) + as_u32_divrem_i32(ri))));
        e = (uint32_t)(e + v);
    }

    const uint32_t big = (uint32_t)(UINT32_C(2147483644) + s);
    const uint32_t small = (uint32_t)(UINT32_C(3) + s);
    const int32_t q0i = (int32_t)(as_i32_divrem_i32(big) / as_i32_divrem_i32(small));
    const int32_t r0i = (int32_t)(as_i32_divrem_i32(big) % as_i32_divrem_i32(small));
    h = mix_i32(h, q0i);
    h = mix_i32(h, r0i);
    h = mix_i32(h, as_i32_divrem_i32((uint32_t)((uint32_t)(small * as_u32_divrem_i32(q0i)) + as_u32_divrem_i32(r0i))));

    const int32_t q1i = (int32_t)(as_i32_divrem_i32(small) / as_i32_divrem_i32(big));
    const int32_t r1i = (int32_t)(as_i32_divrem_i32(small) % as_i32_divrem_i32(big));
    h = mix_i32(h, q1i);
    h = mix_i32(h, r1i);
    h = mix_i32(h, as_i32_divrem_i32((uint32_t)((uint32_t)(big * as_u32_divrem_i32(q1i)) + as_u32_divrem_i32(r1i))));
    return h;
}

static uint64_t divrem_u64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t d = (uint64_t)(UINT64_C(18446744073709551615) - s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(131) + UINT64_C(3) + s);
        const uint64_t q = (uint64_t)(d / v);
        const uint64_t r = (uint64_t)(d % v);
        h = mix_u64(h, q);
        h = mix_u64(h, r);
        h = mix_u64(h, (uint64_t)((uint64_t)(v * q) + r));
        d = (uint64_t)(d - v);
    }

    const uint64_t big = (uint64_t)(UINT64_C(18446744073709551613) + s);
    const uint64_t small = (uint64_t)(UINT64_C(3) + s);
    const uint64_t q0 = (uint64_t)(big / small);
    const uint64_t r0 = (uint64_t)(big % small);
    h = mix_u64(h, q0);
    h = mix_u64(h, r0);
    h = mix_u64(h, (uint64_t)((uint64_t)(small * q0) + r0));

    const uint64_t q1 = (uint64_t)(small / big);
    const uint64_t r1 = (uint64_t)(small % big);
    h = mix_u64(h, q1);
    h = mix_u64(h, r1);
    h = mix_u64(h, (uint64_t)((uint64_t)(big * q1) + r1));
    return h;
}

/* addition, subtraction and multiplication run in the uint64_t bit-pattern
 * domain to stay clear of signed-overflow UB (per the reference rule); the
 * divisor and dividend are only ever read as int64_t at the point of / and %,
 * which is genuinely signed C division, well-defined because every divisor
 * here is nonzero and never -1, and every dividend stays clear of INT64_MIN. */
static inline int64_t as_i64_divrem_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }
static inline uint64_t as_u64_divrem_i64(int64_t v) { uint64_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t divrem_i64(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t d = (uint64_t)(UINT64_C(9000000000000000000) - s);
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(131) + UINT64_C(3) + s);
        const int64_t qi = (int64_t)(as_i64_divrem_i64(d) / as_i64_divrem_i64(v));
        const int64_t ri = (int64_t)(as_i64_divrem_i64(d) % as_i64_divrem_i64(v));
        h = mix_i64(h, qi);
        h = mix_i64(h, ri);
        h = mix_i64(h, as_i64_divrem_i64((uint64_t)((uint64_t)(v * as_u64_divrem_i64(qi)) + as_u64_divrem_i64(ri))));
        d = (uint64_t)(d - v);
    }

    uint64_t e = (uint64_t)(UINT64_C(0) - (uint64_t)(UINT64_C(9000000000000000000) + s));
    for (uint64_t i = 0; i < UINT64_C(8); i = (uint64_t)(i + UINT64_C(1))) {
        const uint64_t v = (uint64_t)(i * UINT64_C(197) + UINT64_C(5) + s);
        const int64_t qi = (int64_t)(as_i64_divrem_i64(e) / as_i64_divrem_i64(v));
        const int64_t ri = (int64_t)(as_i64_divrem_i64(e) % as_i64_divrem_i64(v));
        h = mix_i64(h, qi);
        h = mix_i64(h, ri);
        h = mix_i64(h, as_i64_divrem_i64((uint64_t)((uint64_t)(v * as_u64_divrem_i64(qi)) + as_u64_divrem_i64(ri))));
        e = (uint64_t)(e + v);
    }

    const uint64_t big = (uint64_t)(UINT64_C(9223372036854775804) + s);
    const uint64_t small = (uint64_t)(UINT64_C(3) + s);
    const int64_t q0i = (int64_t)(as_i64_divrem_i64(big) / as_i64_divrem_i64(small));
    const int64_t r0i = (int64_t)(as_i64_divrem_i64(big) % as_i64_divrem_i64(small));
    h = mix_i64(h, q0i);
    h = mix_i64(h, r0i);
    h = mix_i64(h, as_i64_divrem_i64((uint64_t)((uint64_t)(small * as_u64_divrem_i64(q0i)) + as_u64_divrem_i64(r0i))));

    const int64_t q1i = (int64_t)(as_i64_divrem_i64(small) / as_i64_divrem_i64(big));
    const int64_t r1i = (int64_t)(as_i64_divrem_i64(small) % as_i64_divrem_i64(big));
    h = mix_i64(h, q1i);
    h = mix_i64(h, r1i);
    h = mix_i64(h, as_i64_divrem_i64((uint64_t)((uint64_t)(big * as_u64_divrem_i64(q1i)) + as_u64_divrem_i64(r1i))));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    h = mix_u64(h, divrem_u32(seed));
    h = mix_u64(h, divrem_i32(seed));
    h = mix_u64(h, divrem_u64(seed));
    h = mix_u64(h, divrem_i64(seed));
    return h;
}
