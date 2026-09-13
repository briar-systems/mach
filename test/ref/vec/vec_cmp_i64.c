#include "corpus.h"

/* mach's i64x2 and u64x2 are 2 lanes x 8 bytes, size 16, and fill the vector
 * register so $align_of is 16. a comparison yields an unsigned mask of the lane
 * width, all-ones for true and all-zeros for false, built explicitly here. every
 * lane is held as uint64_t: the signed pairs are written as their two's-complement
 * patterns and the signed orderings go through the sign-bit-flip identity, so no
 * signed type is involved and nothing here is UB. */
typedef struct { _Alignas(16) uint64_t l[2]; } u64x2;

#define M64(c) ((uint64_t)((c) ? UINT64_C(0xFFFFFFFFFFFFFFFF) : UINT64_C(0)))
#define S64(x) ((uint64_t)((x) ^ UINT64_C(0x8000000000000000)))

static uint64_t fold_u64x2(uint64_t h, u64x2 v) {
    for (unsigned i = 0; i < 2u; i++) { h = mix_u64(h, v.l[i]); }
    return h;
}

static u64x2 lt_s(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(S64(a.l[i]) <  S64(b.l[i])); } return r; }
static u64x2 le_s(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(S64(a.l[i]) <= S64(b.l[i])); } return r; }
static u64x2 gt_s(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(S64(a.l[i]) >  S64(b.l[i])); } return r; }
static u64x2 ge_s(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(S64(a.l[i]) >= S64(b.l[i])); } return r; }
static u64x2 lt_u(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] <  b.l[i]); } return r; }
static u64x2 le_u(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] <= b.l[i]); } return r; }
static u64x2 gt_u(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] >  b.l[i]); } return r; }
static u64x2 ge_u(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] >= b.l[i]); } return r; }
static u64x2 eq_v(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] == b.l[i]); } return r; }
static u64x2 ne_v(u64x2 a, u64x2 b) { u64x2 r; for (unsigned i = 0; i < 2u; i++) { r.l[i] = M64(a.l[i] != b.l[i]); } return r; }

static u64x2 select(u64x2 m, u64x2 a, u64x2 b) {
    u64x2 r;
    for (unsigned i = 0; i < 2u; i++) { r.l[i] = (uint64_t)((m.l[i] & a.l[i]) | (~m.l[i] & b.l[i])); }
    return r;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t imin = UINT64_C(0x8000000000000000);
    const uint64_t imax = UINT64_C(0x7FFFFFFFFFFFFFFF);
    const uint64_t neg1 = UINT64_C(0xFFFFFFFFFFFFFFFF);
    const uint64_t neg_2p32 = UINT64_C(0xFFFFFFFF00000000);

    u64x2 av[6];
    u64x2 bv[6];
    av[0].l[0] = UINT64_C(8589934591); av[0].l[1] = UINT64_C(4294967297);
    bv[0].l[0] = UINT64_C(4294967297); bv[0].l[1] = UINT64_C(8589934591);
    av[1].l[0] = neg1;                 av[1].l[1] = neg_2p32;
    bv[1].l[0] = neg_2p32;             bv[1].l[1] = neg1;
    av[2].l[0] = imin;                 av[2].l[1] = imax;
    bv[2].l[0] = imax;                 bv[2].l[1] = imin;
    av[3].l[0] = imin;                 av[3].l[1] = UINT64_C(0x123456789ABCDEF0);
    bv[3].l[0] = neg1;                 bv[3].l[1] = UINT64_C(0x123456789ABCDEF0);
    av[4].l[0] = UINT64_C(8589934592); av[4].l[1] = UINT64_C(0);
    bv[4].l[0] = UINT64_C(8589934591); bv[4].l[1] = neg1;
    av[5].l[0] = UINT64_C(266);        av[5].l[1] = neg1;
    bv[5].l[0] = UINT64_C(10);         bv[5].l[1] = UINT64_C(0);

    for (uint64_t k = 0; k < UINT64_C(6); k = (uint64_t)(k + UINT64_C(1))) {
        u64x2 a = av[k];
        u64x2 b = bv[k];
        a.l[0] = (uint64_t)(a.l[0] + seed);
        b.l[1] = (uint64_t)(b.l[1] + seed);
        h = fold_u64x2(h, lt_s(a, b));
        h = fold_u64x2(h, le_s(a, b));
        h = fold_u64x2(h, gt_s(a, b));
        h = fold_u64x2(h, ge_s(a, b));
        h = fold_u64x2(h, eq_v(a, b));
        h = fold_u64x2(h, ne_v(a, b));
        const u64x2 m = lt_s(a, b);
        h = fold_u64x2(h, select(m, a, b));
    }

    u64x2 cv[5];
    u64x2 dv[5];
    cv[0].l[0] = UINT64_C(9223372036854775808); cv[0].l[1] = UINT64_C(9223372036854775807);
    dv[0].l[0] = UINT64_C(9223372036854775807); dv[0].l[1] = UINT64_C(9223372036854775808);
    cv[1].l[0] = UINT64_C(8589934591);          cv[1].l[1] = UINT64_C(4294967297);
    dv[1].l[0] = UINT64_C(4294967297);          dv[1].l[1] = UINT64_C(8589934591);
    cv[2].l[0] = UINT64_C(18446744073709551615); cv[2].l[1] = UINT64_C(0);
    dv[2].l[0] = UINT64_C(0);                    dv[2].l[1] = UINT64_C(18446744073709551615);
    cv[3].l[0] = UINT64_C(9223372039002259456); cv[3].l[1] = UINT64_C(18446744069414584320);
    dv[3].l[0] = UINT64_C(9223372039002259456); dv[3].l[1] = UINT64_C(18446744069414584319);
    cv[4].l[0] = UINT64_C(266);                 cv[4].l[1] = UINT64_C(18446744073709551615);
    dv[4].l[0] = UINT64_C(10);                  dv[4].l[1] = UINT64_C(0);

    for (uint64_t j = 0; j < UINT64_C(5); j = (uint64_t)(j + UINT64_C(1))) {
        u64x2 c = cv[j];
        u64x2 d = dv[j];
        c.l[0] = (uint64_t)(c.l[0] + seed);
        d.l[1] = (uint64_t)(d.l[1] + seed);
        h = fold_u64x2(h, lt_u(c, d));
        h = fold_u64x2(h, le_u(c, d));
        h = fold_u64x2(h, gt_u(c, d));
        h = fold_u64x2(h, ge_u(c, d));
        h = fold_u64x2(h, eq_v(c, d));
        h = fold_u64x2(h, ne_v(c, d));
        const u64x2 m = lt_u(c, d);
        h = fold_u64x2(h, select(m, c, d));
    }
    return h;
}
