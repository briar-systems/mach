#include "corpus.h"

typedef struct { uint8_t d; union { int32_t failed; uint64_t value; } p; } SecretReply;

static SecretReply make(uint64_t v) { SecretReply r; memset(&r, 0, sizeof r); r.p.value = v; r.d = 1; return r; }
static SecretReply failed(int32_t v) { SecretReply r; memset(&r, 0, sizeof r); r.p.failed = v; r.d = 0; return r; }

static SecretReply bump(SecretReply r) {
    if (r.d == 1) { return make((uint64_t)(r.p.value + UINT64_C(1))); }
    return failed(-1);
}

static uint64_t public_of(SecretReply r) {
    if (r.d == 0) { return (uint64_t)(uint32_t)r.p.failed; }
    return r.p.value;
}

static SecretReply copy_whole(SecretReply v) {
    const SecretReply u = v;
    return u;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    SecretReply r;
    memset(&r, 0, sizeof r);
    h = mix_u64(h, public_of(r));
    r = make((uint64_t)(UINT64_C(1000) + seed));
    h = mix_u64(h, public_of(r));
    r = bump(r);
    h = mix_u64(h, public_of(r));
    if (r.d == 1) {
        const uint64_t open = (uint64_t)(r.p.value * UINT64_C(3));
        r.p.value = open;
    }
    h = mix_u64(h, public_of(r));
    r = failed((int32_t)(-7 - (int32_t)seed));
    h = mix_u64(h, public_of(r));
    h = mix_u64(h, public_of(bump(r)));

    const SecretReply hidden = make((uint64_t)(UINT64_C(5) + seed));
    const SecretReply back = copy_whole(hidden);
    h = mix_u64(h, public_of(back));
    return h;
}
