/* signed add/sub/negate wraps in two's complement, identical to arith_u16 at the
 * bit level, so every step is done on the uint16_t bit pattern and only bitcast
 * to int16_t at the fold call, per the reference rule against signed overflow UB. */
#include "corpus.h"

static inline int16_t as_i16(uint16_t v) { int16_t r; memcpy(&r, &v, sizeof r); return r; }

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint16_t s = (uint16_t)seed;

    uint16_t a = (uint16_t)(UINT16_C(65520) + s);
    for (uint16_t i = 0; i < UINT16_C(32); i = (uint16_t)(i + UINT16_C(1))) {
        a = (uint16_t)(a + (uint16_t)(i * UINT16_C(7) + UINT16_C(1)));
        h = mix_i16(h, as_i16(a));
        a = (uint16_t)(a - (uint16_t)(i * UINT16_C(3) + UINT16_C(2)));
        h = mix_i16(h, as_i16(a));
    }

    uint16_t b = s;
    for (uint16_t j = 0; j < UINT16_C(8); j = (uint16_t)(j + UINT16_C(1))) {
        b = (uint16_t)(b - (uint16_t)(j * UINT16_C(12345) + UINT16_C(1)));
        h = mix_i16(h, as_i16(b));
    }

    const uint16_t n = (uint16_t)(UINT16_C(0) - a);
    h = mix_i16(h, as_i16(n));
    h = mix_i16(h, as_i16((uint16_t)(UINT16_C(0) - n)));
    h = mix_i16(h, as_i16((uint16_t)(UINT16_C(0) - s)));
    h = mix_i16(h, as_i16((uint16_t)(UINT16_C(32767) + a)));
    h = mix_i16(h, as_i16((uint16_t)(UINT16_C(32768) - a)));
    h = mix_i16(h, as_i16((uint16_t)(UINT16_C(65535) + a)));
    h = mix_i16(h, as_i16((uint16_t)(a + b)));
    h = mix_i16(h, as_i16((uint16_t)(a - b)));
    h = mix_i16(h, as_i16((uint16_t)(b - a)));
    return h;
}
