#include "corpus.h"

/* C leaves argument order unspecified, so each capture is spelled out before the
   call whose later argument writes the source; that is mach's left-to-right rule */

typedef struct { uint64_t a; uint64_t b; uint64_t c; uint64_t d; uint64_t e; } R;
typedef struct { uint32_t a; uint32_t b; } S;
typedef struct { uint64_t tag; R inner; S small; } W;

static R gr;
static S gs;

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static R make_r(uint64_t s) {
    R r = { gen(s, 1), gen(s, 2), gen(s, 3), gen(s, 4), gen(s, 5) };
    return r;
}

static uint64_t fold_r(uint64_t h, R r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    h = mix_u64(h, r.c);
    h = mix_u64(h, r.d);
    return mix_u64(h, r.e);
}

static uint64_t fold_s(uint64_t h, S r) {
    return mix_u32(mix_u32(h, r.a), r.b);
}

static uint64_t take_r(R r, uint64_t k) {
    return fold_r(mix_u64(fold_init(), k), r);
}

static uint64_t take_s(S r, uint64_t k) {
    return fold_s(mix_u64(fold_init(), k), r);
}

static uint64_t take_rs(R r, S s, uint64_t k) {
    return fold_s(fold_r(mix_u64(fold_init(), k), r), s);
}

static uint64_t write_globals(uint64_t k) {
    gr = make_r(k);
    gs.a = (uint32_t)k;
    gs.b = (uint32_t)(k >> 32);
    return k;
}

static uint64_t poke_r(R *p, uint64_t k) {
    p->a = k;
    p->c = ~k;
    p->e = k + 9;
    return k;
}

static uint64_t poke_s(S *p, uint64_t k) {
    p->a = (uint32_t)k;
    p->b = ~(uint32_t)k;
    return k;
}

static uint64_t poke_w(W *p, uint64_t k) {
    p->tag = k;
    p->inner.b = k ^ UINT64_C(0x5555555555555555);
    p->small.a = (uint32_t)(k + 1);
    return k;
}

static uint64_t clobber_r(R v, uint64_t k) {
    v.a = k;
    v.b = k + 1;
    v.c = k + 2;
    v.d = k + 3;
    v.e = k + 4;
    return fold_r(fold_init(), v);
}

static uint64_t clobber_s(S v, uint64_t k) {
    v.a = (uint32_t)k;
    v.b = (uint32_t)(k >> 16);
    return fold_s(fold_init(), v);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    for (uint64_t pass = 0; pass < 3; pass++) {
        uint64_t k = gen(seed, pass + 11);

        gr = make_r(k);
        gs.a = (uint32_t)(k >> 3);
        gs.b = (uint32_t)(k >> 35);
        {
            R cr = gr;
            uint64_t kk = write_globals(k + 1);
            h = mix_u64(h, take_r(cr, kk));
        }
        h = fold_r(h, gr);
        {
            S cs = gs;
            uint64_t kk = write_globals(k + 2);
            h = mix_u64(h, take_s(cs, kk));
        }
        h = fold_s(h, gs);
        {
            R cr = gr;
            S cs = gs;
            uint64_t kk = write_globals(k + 3);
            h = mix_u64(h, take_rs(cr, cs, kk));
        }
        h = fold_r(h, gr);

        R l = make_r(k + 4);
        S ls = { (uint32_t)(k >> 5), (uint32_t)(k >> 37) };
        R *lp = &l;
        S *lsp = &ls;
        {
            R cr = l;
            uint64_t kk = poke_r(lp, k + 5);
            h = mix_u64(h, take_r(cr, kk));
        }
        h = fold_r(h, l);
        {
            S cs = ls;
            uint64_t kk = poke_s(lsp, k + 6);
            h = mix_u64(h, take_s(cs, kk));
        }
        h = fold_s(h, ls);

        {
            R cr = *lp;
            uint64_t kk = poke_r(lp, k + 7);
            h = mix_u64(h, take_r(cr, kk));
        }
        h = fold_r(h, l);
        {
            S cs = *lsp;
            uint64_t kk = poke_s(lsp, k + 8);
            h = mix_u64(h, take_s(cs, kk));
        }
        h = fold_s(h, ls);

        W w;
        w.tag = k;
        w.inner = make_r(k + 9);
        w.small.a = (uint32_t)k;
        w.small.b = 7;
        W *wp = &w;
        {
            R cr = w.inner;
            S cs = w.small;
            uint64_t kk = poke_w(wp, k + 10);
            h = mix_u64(h, take_rs(cr, cs, kk));
        }
        h = fold_r(h, w.inner);
        h = fold_s(h, w.small);
        h = mix_u64(h, w.tag);

        h = mix_u64(h, clobber_r(l, k + 12));
        h = fold_r(h, l);
        h = mix_u64(h, clobber_s(ls, k + 13));
        h = fold_s(h, ls);
        h = mix_u64(h, clobber_r(gr, k + 14));
        h = fold_r(h, gr);
        h = mix_u64(h, clobber_r(w.inner, k + 15));
        h = fold_r(h, w.inner);
        h = mix_u64(h, clobber_r(*lp, k + 16));
        h = fold_r(h, l);
    }

    return h;
}
