#include "corpus.h"

/* the values each capture must produce, computed in order: an argument or a
 * returned value is the source as it stood when it was read, the call after it
 * writes 99 and 100 (or 99, 100, 101 into the array), and `order` gains the
 * observing call's 1 before the writing call's 2. */

static uint64_t fold_pair(uint64_t h, int64_t first, int64_t second) {
    return mix_i64(mix_i64(h, first), second);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed;

    /* a later argument's call writes the source after the argument is read */
    int64_t order = s * 10 + 1;
    order = order * 10 + 2;
    h = mix_i64(h, order);
    h = fold_pair(h, 99, 100);
    h = mix_i64(h, 7 + s + 8);

    /* a local, then a global */
    h = mix_i64(h, 7 + s + 8);
    h = fold_pair(h, 99, 100);
    h = mix_i64(h, 7 + s + 8);
    h = fold_pair(h, 99, 100);

    /* a field, then an element */
    h = mix_i64(h, 7 + s + 8);
    h = fold_pair(h, 99, 100);
    h = mix_i64(h, 7 + s + 8);
    h = fold_pair(h, 99, 100);

    /* an array, then a union */
    h = mix_i64(h, 7 + s + 8 + 9);
    h = mix_i64(h, 99);
    h = mix_i64(h, 101);
    h = mix_i64(h, 7 + s + 8);
    h = fold_pair(h, 99, 100);

    /* the right side is read before the destination's call writes it */
    int64_t order_2 = s * 10 + 1;
    order_2 = order_2 * 10 + 2;
    h = mix_i64(h, order_2);
    h = fold_pair(h, 99, 100);
    h = fold_pair(h, 7 + s, 8);

    /* the returned value is read before the fin writes it */
    h = fold_pair(h, 99, 100);
    h = fold_pair(h, 7 + s, 8);
    return h;
}
