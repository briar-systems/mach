#include "corpus.h"

typedef struct { uint8_t d; union { int64_t value; } p; } Reply;
typedef struct { uint8_t d; union { Reply inner; } p; } Outer;
typedef struct { uint8_t n; Reply r; uint32_t k; } Box;
typedef struct { uint8_t d; union { Box box; Outer deep; } p; } Holder;

static Reply reply_empty(void) { Reply r; memset(&r, 0, sizeof r); r.d = 0; return r; }
static Reply reply_value(int64_t v) { Reply r; memset(&r, 0, sizeof r); r.p.value = v; r.d = 1; return r; }
static Outer outer_none(void) { Outer o; memset(&o, 0, sizeof o); o.d = 0; return o; }
static Outer outer_inner(Reply r) { Outer o; memset(&o, 0, sizeof o); o.p.inner = r; o.d = 1; return o; }
static Holder holder_nothing(void) { Holder h; memset(&h, 0, sizeof h); h.d = 0; return h; }
static Holder holder_box(Box b) { Holder h; memset(&h, 0, sizeof h); h.p.box = b; h.d = 1; return h; }
static Holder holder_deep(Outer o) { Holder h; memset(&h, 0, sizeof h); h.p.deep = o; h.d = 2; return h; }

static int64_t reply_of(Reply r) {
    if (r.d == 1) { return r.p.value; }
    return -1;
}

static int64_t outer_of(Outer o) {
    if (o.d == 1) { return reply_of(o.p.inner) + 1000; }
    return -2;
}

static uint64_t fold_box(uint64_t h, Box b) {
    h = mix_u8(h, b.n);
    h = mix_i64(h, reply_of(b.r));
    h = mix_u32(h, b.k);
    return h;
}

static uint64_t fold_holder(uint64_t h, Holder hd) {
    if (hd.d == 1) { return fold_box(mix_u8(h, 1), hd.p.box); }
    else if (hd.d == 2) { return mix_i64(mix_u8(h, 2), outer_of(hd.p.deep)); }
    return mix_u8(h, 0);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed;

    Outer o;
    memset(&o, 0, sizeof o);
    h = mix_i64(h, outer_of(o));
    o = outer_inner(reply_empty());
    h = mix_i64(h, outer_of(o));
    o = outer_inner(reply_value(7 + s));
    h = mix_i64(h, outer_of(o));

    if (o.d == 1) {
        if (o.p.inner.d == 1) { o.p.inner.p.value = o.p.inner.p.value * 3; }
        h = mix_i64(h, outer_of(o));
        o.p.inner = reply_empty();
        h = mix_i64(h, outer_of(o));
        o.p.inner = reply_value(11 + s);
    }
    h = mix_i64(h, outer_of(o));

    Box b;
    memset(&b, 0, sizeof b);
    b.n = 3; b.k = 9;
    h = fold_box(h, b);
    b.r = reply_value(20 + s);
    h = fold_box(h, b);
    const Box b2 = b;
    b.r = reply_empty();
    h = fold_box(h, b);
    h = fold_box(h, b2);

    Holder hd;
    memset(&hd, 0, sizeof hd);
    h = fold_holder(h, hd);
    hd = holder_box(b2);
    h = fold_holder(h, hd);
    if (hd.d == 1) {
        hd.p.box.k = (uint32_t)(hd.p.box.k + UINT32_C(1));
        if (hd.p.box.r.d == 1) { hd.p.box.r.p.value = hd.p.box.r.p.value + 1; }
    }
    h = fold_holder(h, hd);
    hd = holder_deep(o);
    h = fold_holder(h, hd);
    if (hd.d == 2) {
        if (hd.p.deep.d == 1) {
            if (hd.p.deep.p.inner.d == 1) { hd.p.deep.p.inner.p.value = hd.p.deep.p.inner.p.value + 5; }
        }
    }
    h = fold_holder(h, hd);
    hd = holder_deep(outer_none());
    h = fold_holder(h, hd);
    hd = holder_nothing();
    h = fold_holder(h, hd);
    Box inner;
    memset(&inner, 0, sizeof inner);
    inner.n = 1; inner.r = reply_value(s); inner.k = 2;
    const Holder hc = holder_box(inner);
    h = fold_holder(h, hc);
    return h;
}
