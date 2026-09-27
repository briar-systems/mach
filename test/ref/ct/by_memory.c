#include "corpus.h"

typedef struct { uint64_t a, b, c, d, e; } Big;
typedef struct { uint64_t a, b; } Two;
typedef struct { uint8_t k[40]; uint64_t tag; } Keyed;

static void vx(uint32_t r[8], const uint32_t a[8], const uint32_t b[8]) {
    for (size_t i = 0; i < 8; i++) { r[i] = (uint32_t)((uint32_t)(a[i] ^ b[i]) + a[i]); }
}

static Big bx(Big a, Big b) {
    Big r = a;
    r.a = a.a ^ b.e;
    r.c = (uint64_t)(a.c + b.b);
    r.e = (uint64_t)(a.e + b.a);
    return r;
}

static Two tx(Two a, Two b) {
    Two r = a;
    r.a = a.a ^ b.b;
    r.b = (uint64_t)(a.b + b.a);
    return r;
}

static void ax(uint64_t r[5], const uint64_t a[5], const uint64_t b[5]) {
    for (size_t i = 0; i < 5; i++) { r[i] = a[i] ^ b[4 - i]; }
}

static Keyed kx(Keyed a, Keyed b) {
    Keyed r = a;
    r.k[0] = (uint8_t)(a.k[0] ^ b.k[39]);
    r.k[39] = (uint8_t)(a.k[39] + b.k[0]);
    r.tag = (uint64_t)(a.tag + b.tag);
    return r;
}

static Big chain(const uint32_t v[8], Big g, Two t, const uint64_t q[5]) {
    uint32_t w1[8], w[8];
    vx(w1, v, v);
    vx(w, w1, v);
    const Big h = bx(bx(g, g), g);
    const Two u = tx(tx(t, t), t);
    uint64_t z1[5], z[5];
    ax(z1, q, q);
    ax(z, z1, q);
    Big r = h;
    r.b = h.b ^ (uint64_t)w[5];
    r.d = h.d ^ u.b ^ z[3];
    return r;
}

static uint64_t fold_vec(uint64_t h, const uint32_t v[8]) {
    uint64_t r = h;
    for (size_t i = 0; i < 8; i++) { r = mix_u32(r, v[i]); }
    return r;
}

static uint64_t fold_big(uint64_t h, Big g) {
    uint64_t r = mix_u64(h, g.a);
    r = mix_u64(r, g.b);
    r = mix_u64(r, g.c);
    r = mix_u64(r, g.d);
    return mix_u64(r, g.e);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    const uint32_t v[8] = {UINT32_C(2863311530) ^ s, 1, UINT32_C(4294967295), UINT32_C(2147483648), UINT32_C(7) ^ s, UINT32_C(305419896), UINT32_C(65535) ^ s, UINT32_C(3735928559)};
    const uint32_t u[8] = {UINT32_C(1431655765), UINT32_C(2) ^ s, 0, UINT32_C(4294967294), UINT32_C(3) ^ s, UINT32_C(2271560481), UINT32_C(16777215), UINT32_C(4276215469) ^ s};
    uint32_t r[8];
    vx(r, v, u);
    h = fold_vec(h, r);
    vx(r, u, v);
    h = fold_vec(h, r);

    const Big g = {UINT64_C(6510615555426900570) ^ seed, 1, UINT64_C(18446744073709551615), UINT64_C(9223372036854775808), UINT64_C(7) ^ seed};
    const Big k = {3, UINT64_C(14106333703424951235) ^ seed, UINT64_C(18446744073709551614), UINT64_C(5) ^ seed, UINT64_C(81985529216486895)};
    h = fold_big(h, bx(g, k));
    h = fold_big(h, bx(k, g));

    const Two t = {UINT64_C(12297829382473034410) ^ seed, UINT64_C(18446744073709551615)};
    const Two o = {2, UINT64_C(6148914691236517205) ^ seed};
    const Two tr = tx(t, o);
    h = mix_u64(h, tr.a);
    h = mix_u64(h, tr.b);

    const uint64_t q[5] = {UINT64_C(1) ^ seed, UINT64_C(18446744073709551615), UINT64_C(9223372036854775808), UINT64_C(12345678901234567) ^ seed, 42};
    const uint64_t p[5] = {7, UINT64_C(11) ^ seed, 13, 17, UINT64_C(18446744073709551557) ^ seed};
    uint64_t ar[5];
    ax(ar, q, p);
    for (size_t i = 0; i < 5; i++) { h = mix_u64(h, ar[i]); }

    Keyed ka, kb;
    for (uint64_t j = 0; j < 40; j++) {
        ka.k[j] = (uint8_t)(seed + j * UINT64_C(37));
        kb.k[j] = (uint8_t)(seed ^ (j * UINT64_C(91)));
    }
    ka.tag = seed;
    kb.tag = UINT64_C(18446744073709551615);
    const Keyed kr = kx(ka, kb);
    for (size_t j = 0; j < 40; j++) { h = mix_u8(h, kr.k[j]); }
    h = mix_u64(h, kr.tag);

    h = fold_big(h, chain(v, g, t, q));
    return h;
}
