#include "corpus.h"

typedef struct { uint8_t d; union { int64_t value; } p; } Reply;
typedef struct { uint8_t d; union { int64_t b; uint8_t c; } p; } Three;
typedef struct { uint8_t d; union { Reply inner; } p; } Outer;

static Reply reply_empty(void) { Reply r; memset(&r, 0, sizeof r); r.d = 0; return r; }
static Reply make(int64_t v) { Reply r; memset(&r, 0, sizeof r); r.p.value = v; r.d = 1; return r; }
static Three three_a(void) { Three t; memset(&t, 0, sizeof t); t.d = 0; return t; }
static Three three_b(int64_t v) { Three t; memset(&t, 0, sizeof t); t.p.b = v; t.d = 1; return t; }
static Three three_c(uint8_t v) { Three t; memset(&t, 0, sizeof t); t.p.c = v; t.d = 2; return t; }
static Outer outer_none(void) { Outer o; memset(&o, 0, sizeof o); o.d = 0; return o; }
static Outer outer_inner(Reply r) { Outer o; memset(&o, 0, sizeof o); o.p.inner = r; o.d = 1; return o; }

static int64_t arm(Reply r) {
    Reply t = r;
    if (t.d == 1) {
        t.p.value = t.p.value + 1;
        int64_t *p = &t.p.value;
        *p = *p + 100;
        return t.p.value;
    }
    return -1;
}

static int64_t chain_ret(Reply r) {
    Reply t = r;
    if (t.d != 1) { return -1; }
    t.p.value = t.p.value + 1;
    int64_t *p = &t.p.value;
    *p = *p + 100;
    return t.p.value;
}

static int64_t chain_brk(Reply r) {
    Reply t = r;
    int64_t out = -1;
    uint8_t once = 0;
    while (once == 0) {
        once = 1;
        if (t.d != 1) { break; }
        t.p.value = t.p.value + 1;
        int64_t *p = &t.p.value;
        *p = *p + 100;
        out = t.p.value;
    }
    return out;
}

typedef struct { Reply items[4]; } Arr4;

static int64_t chain_cnt(Arr4 a) {
    Arr4 arr = a;
    int64_t sum = 0;
    uint64_t i = 0;
    while (i < 4) {
        const uint64_t j = i;
        i = i + 1;
        if (arr.items[j].d != 1) { continue; }
        arr.items[j].p.value = arr.items[j].p.value + 1;
        int64_t *p = &arr.items[j].p.value;
        *p = *p + 100;
        sum = sum + arr.items[j].p.value;
    }
    return sum;
}

static int64_t andop(Reply r) {
    Reply t = r;
    int64_t out = 0;
    if (t.d == 1 && t.p.value > 3) { out = out + 1; }
    if (t.d == 1 && *(&t.p.value) > 3) { out = out + 10; }
    if (t.d == 0 || (t.d == 1 && t.p.value == 1)) { out = out + 100; }
    return out;
}

static int64_t three(Three t) {
    Three v = t;
    if (v.d == 0) { return -1; }
    else if (v.d == 2) { return (int64_t)v.p.c; }
    v.p.b = v.p.b + 1;
    int64_t *p = &v.p.b;
    return *p;
}

static int64_t nested(Outer o) {
    Outer v = o;
    if (v.d == 1) {
        if (v.p.inner.d != 1) { return -1; }
        v.p.inner.p.value = v.p.inner.p.value + 1;
        int64_t *p = &v.p.inner.p.value;
        *p = *p + 100;
        return v.p.inner.p.value;
    }
    return -2;
}

static int64_t through(Reply r) {
    Reply t = r;
    Reply *p = &t;
    int64_t out = 0;
    if (p->d == 1) { p->p.value = p->p.value + 1; out = out + p->p.value; }
    if ((*p).d == 1) { out = out + (*p).p.value; }
    return out;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed + 41;

    h = mix_i64(h, arm(make(s)));
    h = mix_i64(h, arm(reply_empty()));
    h = mix_i64(h, chain_ret(make(s)));
    h = mix_i64(h, chain_ret(reply_empty()));
    h = mix_i64(h, chain_brk(make(s)));
    h = mix_i64(h, chain_brk(reply_empty()));

    Arr4 a;
    memset(&a, 0, sizeof a);
    a.items[0] = make(1);
    a.items[2] = make(2 + s);
    h = mix_i64(h, chain_cnt(a));

    h = mix_i64(h, andop(make(s)));
    h = mix_i64(h, andop(make(1)));
    h = mix_i64(h, andop(reply_empty()));

    h = mix_i64(h, three(three_b(5 + s)));
    h = mix_i64(h, three(three_c(9)));
    h = mix_i64(h, three(three_a()));

    h = mix_i64(h, nested(outer_inner(make(s))));
    h = mix_i64(h, nested(outer_inner(reply_empty())));
    h = mix_i64(h, nested(outer_none()));

    h = mix_i64(h, through(make(s)));
    h = mix_i64(h, through(reply_empty()));

    const uint8_t done = a.items[2].d == 1;
    if (done) { h = mix_u8(h, 1); } else { h = mix_u8(h, 0); }
    return h;
}
