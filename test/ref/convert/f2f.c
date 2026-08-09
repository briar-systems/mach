#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const double z = (double)seed;
    const float zf = (float)seed;

    const double d1 = 1.0000000596046448 + z;
    const float f1 = (float)d1;
    const double d1b = (double)f1;
    h = mix_f64(h, d1);
    h = mix_f32(h, f1);
    h = mix_f64(h, d1b);

    const double d2 = 0.1 + z;
    const float f2 = (float)d2;
    const double d2b = (double)f2;
    h = mix_f64(h, d2);
    h = mix_f32(h, f2);
    h = mix_f64(h, d2b);

    const double d3 = 16777217.0 + z;
    const float f3 = (float)d3;
    h = mix_f64(h, d3);
    h = mix_f32(h, f3);

    const float f4 = 3.4028235e38f + zf;
    const double d4 = (double)f4;
    h = mix_f32(h, f4);
    h = mix_f64(h, d4);

    return h;
}
