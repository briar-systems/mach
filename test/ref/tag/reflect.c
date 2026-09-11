#include "corpus.h"
#include <stddef.h>

typedef struct { uint8_t d; union { uint32_t a; uint32_t b; uint32_t c; } p; } Tri;
typedef struct { uint16_t d; union { int64_t value; } p; } Reply;

static Tri tri_case(uint64_t code, uint32_t v) {
    Tri t;
    memset(&t, 0, sizeof t);
    if (code == 0) { t.p.a = v; }
    else if (code == 1) { t.p.b = v; }
    else { t.p.c = v; }
    t.d = (uint8_t)code;
    return t;
}

static uint64_t code_of(Tri t) {
    uint64_t out = 99;
    if (t.d == 0) { out = 0; }
    if (t.d == 1) { out = 1; }
    if (t.d == 2) { out = 2; }
    return out;
}

static uint32_t payload_of(Tri t) {
    uint32_t out = 0;
    if (t.d == 0) { out = t.p.a; }
    if (t.d == 1) { out = t.p.b; }
    if (t.d == 2) { out = t.p.c; }
    return out;
}

static uint64_t reply_walk(uint64_t h, Reply r) {
    if (r.d == 0) {
        h = mix_u64(h, 0);
    }
    if (r.d == 1) {
        h = mix_u64(h, 1);
        h = mix_u64(h, (uint64_t)offsetof(Reply, p));
        h = mix_i64(h, r.p.value);
    }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    h = mix_u8(h, 1);
    h = mix_u8(h, 0);
    h = mix_u64(h, (uint64_t)sizeof(uint8_t));
    h = mix_u64(h, (uint64_t)sizeof(uint16_t));

    uint64_t code = 0;
    while (code < 3) {
        const Tri t = tri_case(code, (uint32_t)((uint32_t)code * UINT32_C(100) + s));
        h = mix_u64(h, code_of(t));
        h = mix_u32(h, payload_of(t));
        h = mix_u64(h, (uint64_t)offsetof(Tri, p));
        code = code + 1;
    }

    Reply r;
    memset(&r, 0, sizeof r);
    h = reply_walk(h, r);
    memset(&r, 0, sizeof r);
    r.p.value = 77 + (int64_t)seed; r.d = 1;
    h = reply_walk(h, r);
    memset(&r, 0, sizeof r);
    r.d = 0;
    h = reply_walk(h, r);
    return h;
}
