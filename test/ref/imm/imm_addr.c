#include "corpus.h"

typedef struct { uint32_t lead; uint64_t v[3]; uint32_t trail; } Node;

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    /* zero-initialized locals, matching mach's default-initialized locals:
     * the case keeps this storage function-local rather than module-scope,
     * so the reference does too. */
    uint64_t TABLE[4224] = {0};
    Node NODES[96] = {0};

    TABLE[0] = (uint64_t)(s + UINT64_C(1));
    TABLE[255] = (uint64_t)(s + UINT64_C(2));
    TABLE[256] = (uint64_t)(s + UINT64_C(3));
    TABLE[4095] = (uint64_t)(s + UINT64_C(4));
    TABLE[4096] = (uint64_t)(s + UINT64_C(5));
    TABLE[4223] = (uint64_t)(s + UINT64_C(6));
    h = mix_u64(h, TABLE[0]);
    h = mix_u64(h, TABLE[255]);
    h = mix_u64(h, TABLE[256]);
    h = mix_u64(h, TABLE[4095]);
    h = mix_u64(h, TABLE[4096]);
    h = mix_u64(h, TABLE[4223]);

    const uint64_t i0 = s % UINT64_C(4224);
    const uint64_t i1 = (s + UINT64_C(4095)) % UINT64_C(4224);
    const uint64_t i2 = (s + UINT64_C(2048)) % UINT64_C(4224);
    TABLE[i0] = (uint64_t)(TABLE[i0] + UINT64_C(7));
    TABLE[i1] = (uint64_t)(TABLE[i1] + UINT64_C(8));
    TABLE[i2] = (uint64_t)(TABLE[i2] + UINT64_C(9));
    h = mix_u64(h, TABLE[i0]);
    h = mix_u64(h, TABLE[i1]);
    h = mix_u64(h, TABLE[i2]);

    const uint64_t n0 = s % UINT64_C(96);
    const uint64_t n1 = (s + UINT64_C(63)) % UINT64_C(96);
    NODES[n0].lead = UINT32_C(4294967295);
    NODES[n0].v[0] = (uint64_t)(s + UINT64_C(10));
    NODES[n0].v[1] = (uint64_t)(s + UINT64_C(11));
    NODES[n0].v[2] = (uint64_t)(s + UINT64_C(12));
    NODES[n0].trail = UINT32_C(305419896);
    NODES[n1].lead = UINT32_C(2882400001);
    NODES[n1].v[0] = (uint64_t)(s + UINT64_C(13));
    NODES[n1].v[1] = (uint64_t)(s + UINT64_C(14));
    NODES[n1].v[2] = (uint64_t)(s + UINT64_C(15));
    NODES[n1].trail = UINT32_C(19088743);
    h = mix_u32(h, NODES[n0].lead);
    h = mix_u64(h, NODES[n0].v[0]);
    h = mix_u64(h, NODES[n0].v[1]);
    h = mix_u64(h, NODES[n0].v[2]);
    h = mix_u32(h, NODES[n0].trail);
    h = mix_u32(h, NODES[n1].lead);
    h = mix_u64(h, NODES[n1].v[0]);
    h = mix_u64(h, NODES[n1].v[1]);
    h = mix_u64(h, NODES[n1].v[2]);
    h = mix_u32(h, NODES[n1].trail);

    uint64_t *p = &TABLE[4095];
    *p = (uint64_t)(*p + UINT64_C(16));
    h = mix_u64(h, *p);

    return h;
}
