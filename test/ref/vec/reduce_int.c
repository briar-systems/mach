#include "corpus.h"

/* the loops of cases/vec/reduce_int.mach, scalar and in element order. signed
 * wrapping sums and products are computed unsigned and reinterpreted, so no
 * overflow here is undefined. */

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t n = (uint64_t)(UINT64_C(67) - (seed & UINT64_C(1)));
    const uint32_t s = (uint32_t)seed;
    uint64_t i;

    uint8_t b[67];
    int32_t x[67];
    int32_t y[67];
    uint32_t u[67];
    for (i = 0; i < n; i++) {
        const uint32_t r = (uint32_t)((uint32_t)((uint32_t)i * UINT32_C(2654435761)) + s);
        b[i] = (uint8_t)((r >> 13) & UINT32_C(15));
        x[i] = (int32_t)(uint32_t)((uint32_t)(r >> 7) - UINT32_C(16777216));
        y[i] = (int32_t)(uint32_t)((uint32_t)((uint32_t)(r * UINT32_C(40503)) >> 9) - UINT32_C(4194304));
        u[i] = (uint32_t)((r >> 20) | UINT32_C(1));
    }

    const uint8_t k = (uint8_t)((seed & UINT64_C(7)) + UINT64_C(4));
    uint64_t cnt = 0;
    uint32_t big = 0;
    for (i = 0; i < n; i++) {
        if (b[i] == k) { cnt = cnt + 1; }
    }
    for (i = 0; i < n; i++) {
        if (b[i] > 7) { big = big + 1; }
    }
    h = mix_u64(h, cnt);
    h = mix_u32(h, big);

    const int32_t t = (int32_t)(s & UINT32_C(1023));
    uint32_t cs = 0;
    uint32_t skip = 0;
    uint64_t ws = 0;
    for (i = 0; i < n; i++) {
        if (x[i] > t) { cs = (uint32_t)(cs + (uint32_t)x[i]); }
    }
    for (i = 0; i < n; i++) {
        const int32_t v = x[i];
        if (!(v > y[i])) { skip = (uint32_t)(skip + (uint32_t)v); }
    }
    for (i = 0; i < n; i++) {
        if (x[i] > y[i]) { ws = (uint64_t)(ws + (uint64_t)(int64_t)x[i]); }
    }
    h = mix_i32(h, (int32_t)cs);
    h = mix_i32(h, (int32_t)skip);
    h = mix_i64(h, (int64_t)ws);

    uint32_t ma = 0;
    uint64_t mw = 0;
    for (i = 0; i < n; i++) {
        ma = (uint32_t)(ma + (uint32_t)((uint32_t)x[i] * (uint32_t)y[i]));
    }
    for (i = 0; i < n; i++) {
        mw = (uint64_t)(mw + (uint64_t)((int64_t)x[i] * (int64_t)y[i]));
    }
    h = mix_i32(h, (int32_t)ma);
    h = mix_i64(h, (int64_t)mw);

    uint32_t p = 1;
    uint32_t pp = 3;
    uint32_t an = UINT32_C(4294967295);
    uint32_t ac = UINT32_C(4294967295);
    uint32_t o = 0;
    uint32_t xr = 0;
    for (i = 0; i < n; i++) {
        p = (uint32_t)(p * u[i]);
    }
    for (i = 0; i < n; i++) {
        const uint32_t w = u[i];
        if (w > 2000) { pp = (uint32_t)(pp * w); }
    }
    for (i = 0; i < n; i++) {
        an = an & (u[i] | UINT32_C(3072));
    }
    for (i = 0; i < n; i++) {
        const uint32_t w = u[i];
        if (w < 3000) { ac = ac & w; }
    }
    for (i = 0; i < n; i++) {
        o = o | u[i];
        xr = xr ^ u[i];
    }
    h = mix_u32(h, p);
    h = mix_u32(h, pp);
    h = mix_u32(h, an);
    h = mix_u32(h, ac);
    h = mix_u32(h, o);
    h = mix_u32(h, xr);
    return h;
}
