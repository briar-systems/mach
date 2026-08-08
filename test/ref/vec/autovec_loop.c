#include "corpus.h"

/* no vector type appears here: the loops are scalar and the question is whether
 * mach's -O2 vectorizes them into the same answer its -O0 produces. every integer
 * operation is unsigned, so every wrap is defined, and the float reduction is
 * written in element order so -ffp-contract=off and the absence of -ffast-math keep
 * the reference strictly ordered. */

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t n = (uint64_t)(UINT64_C(67) - (seed & UINT64_C(1)));
    const uint64_t m = (uint64_t)(UINT64_C(37) - (seed & UINT64_C(1)));
    const uint32_t s = (uint32_t)seed;
    const float z = (float)seed;
    uint64_t i;

    uint32_t a[67];
    uint32_t b[67];
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        a[i] = (uint32_t)((uint32_t)((uint32_t)i * UINT32_C(2654435761)) + UINT32_C(12345));
        b[i] = (uint32_t)(UINT32_C(4294967295) - (uint32_t)((uint32_t)i * UINT32_C(40503)));
    }
    a[0] = (uint32_t)(a[0] + s);
    b[1] = (uint32_t)(b[1] + s);

    uint32_t c[67];
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        c[i] = (uint32_t)((uint32_t)(a[i] * UINT32_C(3)) + b[i]);
    }
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, c[i]);
    }

    uint32_t sum = 0;
    uint32_t xacc = 0;
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        sum = (uint32_t)(sum + c[i]);
        xacc = (uint32_t)(xacc ^ a[i]);
    }
    h = mix_u32(h, sum);
    h = mix_u32(h, xacc);

    uint32_t d[67];
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        if (a[i] > b[i]) { d[i] = (uint32_t)(a[i] - b[i]); } else { d[i] = (uint32_t)(b[i] - a[i]); }
    }
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, d[i]);
    }

    uint32_t e[67];
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        e[i] = UINT32_C(2863311530);
    }
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(2))) {
        e[i] = (uint32_t)(c[i] ^ UINT32_C(1));
    }
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, e[i]);
    }

    uint32_t g[67];
    g[0] = a[0];
    for (i = 1; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        g[i] = (uint32_t)((uint32_t)(g[i - 1] * UINT32_C(31)) + a[i]);
    }
    for (i = 0; i < n; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_u32(h, g[i]);
    }

    float fa[37];
    float fb[37];
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        fa[i] = (float)i + z;
        fb[i] = 1.25f - (float)i;
    }

    float fc[37];
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        fc[i] = fa[i] * fb[i];
    }
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_f32(h, fc[i]);
    }

    float fd[37];
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        fd[i] = fa[i] / fb[i];
    }
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        h = mix_f32(h, fd[i]);
    }

    float fs = 0.0f;
    for (i = 0; i < m; i = (uint64_t)(i + UINT64_C(1))) {
        fs = fs + fd[i];
    }
    h = mix_f32(h, fs);
    return h;
}
