#include "corpus.h"

static double opaque_f64(uint64_t bits, uint64_t s) {
    uint64_t b = (uint64_t)(bits + s);
    double f;
    memcpy(&f, &b, sizeof f);
    return f;
}

static uint64_t fold64(uint64_t h, double v) {
    if (v != v) { return mix_u64(h, UINT64_C(0xFFFFFFFFFFFFFFFF)); }
    return mix_f64(h, v);
}

/* mach's float `%` is the truncated remainder rem = x - trunc(x / y) * y, with the
 * truncation taken through i64. every operand pair the case feeds it has an exact
 * quotient truncation and an exact product, so this agrees with the IEEE remainder
 * and the oracle carries no lowering detail of its own. the i64 conversion is in
 * range for every pair, so it is defined in C. */
static double frem64(double x, double y) {
    double q = x / y;
    int64_t t = (int64_t)q;
    double tf = (double)t;
    double p = tf * y;
    return x - p;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    const double one = opaque_f64(UINT64_C(0x3FF0000000000000), seed);

    const double a = 1.5 * one;
    const double b = 0.25 * one;
    h = fold64(h, a + b);
    h = fold64(h, a - b);
    h = fold64(h, b - a);
    h = fold64(h, a * b);
    h = fold64(h, a / b);
    h = fold64(h, b / a);
    h = fold64(h, -a);
    h = fold64(h, 0.0 - a);
    h = fold64(h, a - a);
    h = fold64(h, (double)(0.0 - a) + a);

    const double three = 3.0 * one;
    const double third = one / three;
    h = fold64(h, third);
    h = fold64(h, third * three);
    h = fold64(h, (double)((double)(one - third) - third) - third);
    h = fold64(h, one / 49.0);
    h = fold64(h, one / 1048576.5);
    h = fold64(h, 1048576.5 / three);

    const double p53 = 9007199254740992.0 * one;
    h = fold64(h, p53 + one);
    h = fold64(h, (double)(p53 + one) + one);
    h = fold64(h, p53 + (double)(one + one));
    h = fold64(h, p53 - one);
    h = fold64(h, (double)(p53 + one) - p53);

    double acc = 0.0 * one;
    for (uint64_t i = 0; i < UINT64_C(24); i = (uint64_t)(i + UINT64_C(1))) {
        acc = acc + third;
        acc = acc * 1.5;
        h = fold64(h, acc);
    }

    const double dmax = opaque_f64(UINT64_C(0x7FEFFFFFFFFFFFFF), seed);
    h = fold64(h, dmax);
    h = fold64(h, dmax / 2.0);
    h = fold64(h, dmax + dmax);
    h = fold64(h, dmax * 2.0);
    h = fold64(h, 0.0 - (double)(dmax * 2.0));
    h = fold64(h, dmax * dmax);
    h = fold64(h, dmax - dmax);

    const double dmin = opaque_f64(UINT64_C(0x0010000000000000), seed);
    const double tiny = opaque_f64(UINT64_C(0x0000000000000001), seed);
    h = fold64(h, dmin * 0.5);
    h = fold64(h, dmin / 2.0);
    h = fold64(h, dmin - tiny);
    h = fold64(h, dmin * one);
    h = fold64(h, tiny + tiny);
    h = fold64(h, tiny * 0.5);
    h = fold64(h, tiny * 0.75);
    h = fold64(h, tiny * 9007199254740992.0);
    h = fold64(h, tiny / dmin);

    const double r = 5.5 * one;
    const double q = 3.0 * one;
    h = fold64(h, frem64(r, q));
    h = fold64(h, frem64(0.0 - r, q));
    h = fold64(h, frem64(r, 0.0 - q));
    h = fold64(h, frem64(0.0 - r, 0.0 - q));
    h = fold64(h, frem64(7.0 * one, 0.5));
    h = fold64(h, frem64(one, q));
    h = fold64(h, frem64(q, q));
    h = fold64(h, frem64(1048576.0 * one, q));
    return h;
}
