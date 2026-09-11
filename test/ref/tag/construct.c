#include "corpus.h"

typedef struct { uint8_t d; union { int64_t value; } p; } Reply;
typedef struct { uint8_t d; } Flag;
typedef Reply R;

static Reply reply_empty(void) { Reply r; memset(&r, 0, sizeof r); r.d = 0; return r; }
static Reply reply_value(int64_t v) { Reply r; memset(&r, 0, sizeof r); r.p.value = v; r.d = 1; return r; }
static Flag flag_off(void) { Flag f; memset(&f, 0, sizeof f); f.d = 0; return f; }
static Flag flag_on(void) { Flag f; memset(&f, 0, sizeof f); f.d = 1; return f; }

static int64_t value_of(Reply r) {
    if (r.d == 1) { return r.p.value; }
    return -1;
}

static uint8_t is_empty(Reply r) {
    if (r.d == 0) { return 1; }
    return 0;
}

static uint8_t flag_code(Flag f) {
    if (f.d == 1) { return 1; }
    return 0;
}

static uint64_t fold_reply(uint64_t h, Reply r) {
    h = mix_u8(h, is_empty(r));
    h = mix_i64(h, value_of(r));
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed;

    Reply r;
    memset(&r, 0, sizeof r);
    h = fold_reply(h, r);

    r = reply_value(40 + s);
    h = fold_reply(h, r);

    if (r.d == 1) { r.p.value = r.p.value * 2; }
    h = fold_reply(h, r);

    r = reply_value(value_of(r) + 1);
    h = fold_reply(h, r);

    r = reply_empty();
    h = fold_reply(h, r);

    R a = reply_value(7 + s);
    h = fold_reply(h, a);
    const Reply c = a;
    h = fold_reply(h, c);
    a = reply_empty();
    h = fold_reply(h, a);

    Flag f;
    memset(&f, 0, sizeof f);
    h = mix_u8(h, flag_code(f));
    f = flag_on();
    h = mix_u8(h, flag_code(f));
    const Flag g = f;
    f = flag_off();
    h = mix_u8(h, flag_code(f));
    h = mix_u8(h, flag_code(g));

    Reply d = reply_value(100 + s);
    Reply e = d;
    if (e.d == 1) { e.p.value = e.p.value + 5; }
    h = fold_reply(h, d);
    h = fold_reply(h, e);
    return h;
}
