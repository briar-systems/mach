#include "corpus.h"

/* the mach side dispatches on the policy type at compile time; here each
 * concrete instance is written out as the arm the source names */

static uint64_t natural_arm(int64_t key) {
    return (uint64_t)((uint64_t)key * UINT64_C(3) + UINT64_C(1));
}

static uint64_t custom_arm(uint64_t k, uint64_t v) {
    return (uint64_t)(k * UINT64_C(5) + v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    int64_t a_k = (int64_t)(seed + UINT64_C(11));
    uint8_t a_v = (uint8_t)(seed + UINT64_C(2));
    int64_t b_k = (int64_t)(seed + UINT64_C(7));
    uint8_t b_v = (uint8_t)(seed + UINT64_C(3));
    int64_t key = (int64_t)(seed + UINT64_C(40));
    (void)a_k;
    (void)a_v;

    h = mix_u64(h, natural_arm(key));
    h = mix_u64(h, natural_arm(key));
    h = mix_u64(h, custom_arm((uint64_t)b_k, (uint64_t)b_v));
    h = mix_u64(h, natural_arm(key));

    uint32_t c_k = (uint32_t)(seed + UINT64_C(9));
    uint16_t c_v = (uint16_t)(seed + UINT64_C(4));
    h = mix_u64(h, custom_arm((uint64_t)c_k, (uint64_t)c_v));

    return h;
}
