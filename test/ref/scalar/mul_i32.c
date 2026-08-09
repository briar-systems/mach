/* a signed product's wrap is bit-identical to the unsigned product of the same
 * bit patterns, so the multiply and add steps run in the uint32_t domain and
 * only bitcast to int32_t at the fold call, per the reference rule against
 * signed overflow UB. */
#include "corpus.h"

static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    uint32_t a = (uint32_t)(UINT32_C(2654435769) + s); /* bits of -1640531527 */
    for (uint32_t i = 0; i < UINT32_C(16); i = (uint32_t)(i + UINT32_C(1))) {
        a = (uint32_t)(a * (uint32_t)(UINT32_C(2) * i + UINT32_C(3)));
        h = mix_i32(h, as_i32(a));
    }

    uint32_t b = (uint32_t)(UINT32_C(4294967231) + s); /* bits of -65 */
    h = mix_i32(h, as_i32((uint32_t)(b * b)));
    h = mix_i32(h, as_i32((uint32_t)(b * UINT32_C(4294967295))));
    h = mix_i32(h, as_i32((uint32_t)(a * b)));
    h = mix_i32(h, as_i32((uint32_t)(b * a)));

    uint32_t c = (uint32_t)(UINT32_C(65537) + s);
    h = mix_i32(h, as_i32((uint32_t)(c * c)));
    h = mix_i32(h, as_i32((uint32_t)(c * UINT32_C(4294967295))));
    return h;
}
