/* a signed product's wrap is bit-identical to the unsigned product of the same
 * bit patterns, so the multiply and add steps run in the uint64_t domain and
 * only bitcast to int64_t at the fold call, per the reference rule against
 * signed overflow UB. */
#include "corpus.h"

static inline int64_t as_i64(uint64_t v) { int64_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint64_t a = (uint64_t)(UINT64_C(11400714819323198485) + s); /* bits of -7046029254386353131 */
    for (uint64_t i = 0; i < UINT64_C(16); i = (uint64_t)(i + UINT64_C(1))) {
        a = (uint64_t)(a * (uint64_t)(UINT64_C(2) * i + UINT64_C(3)));
        h = mix_i64(h, as_i64(a));
    }

    uint64_t b = (uint64_t)(UINT64_C(18446744073709551553) + s); /* bits of -63 */
    h = mix_i64(h, as_i64((uint64_t)(b * b)));
    h = mix_i64(h, as_i64((uint64_t)(b * UINT64_C(18446744073709551615))));
    h = mix_i64(h, as_i64((uint64_t)(a * b)));
    h = mix_i64(h, as_i64((uint64_t)(b * a)));

    uint64_t c = (uint64_t)(UINT64_C(4294967311) + s);
    h = mix_i64(h, as_i64((uint64_t)(c * c)));
    h = mix_i64(h, as_i64((uint64_t)(c * UINT64_C(18446744073709551615))));
    return h;
}
