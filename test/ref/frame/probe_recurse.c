#include "corpus.h"

/* the same recursions without the frames: each answer depends only on the
 * depth and the values the frames carry, so the arrays are left out. the cold
 * branch of the first two never runs. */

static int32_t bare_page_recurse(int32_t n) {
    if (n <= 0) { return 7; }
    return (int32_t)(bare_page_recurse((int32_t)(n - 1)) + 1);
}

static int32_t page_multiple_recurse(int32_t n) {
    if (n <= 0) { return 11; }
    return (int32_t)(page_multiple_recurse((int32_t)(n - 1)) + 1);
}

static uint8_t deep_wide_recurse(int32_t n, uint8_t seed) {
    const uint8_t first = (uint8_t)(uint32_t)n;
    if (n <= 0) { return (uint8_t)(first + seed); }
    const uint8_t r = deep_wide_recurse((int32_t)(n - 1), (uint8_t)(seed + 1u));
    return (uint8_t)(r + first);
}

static int64_t saved_frame_recurse(int32_t n, int64_t acc) {
    const int64_t last = (int64_t)(uint8_t)(uint32_t)n;
    if (n <= 0) { return acc + 77 + last; }
    const int64_t r = saved_frame_recurse((int32_t)(n - 1), acc + 3);
    return r + last;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int32_t s = (int32_t)(uint32_t)seed;
    h = mix_i32(h, bare_page_recurse(300 + s));
    h = mix_i32(h, page_multiple_recurse(300 + s));
    h = mix_u8(h, deep_wide_recurse(64 + s, (uint8_t)(1u + (uint8_t)seed)));
    h = mix_i64(h, saved_frame_recurse(300 + s, (int64_t)seed));
    return h;
}
