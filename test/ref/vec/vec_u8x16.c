#include "corpus.h"

/* mach's u8x16 is 16 lanes x 1 byte, size 16, and it fills the vector register so
 * $align_of is 16. _Alignas gives the C struct the same size and alignment.
 *
 * every lane operation widens to uint32_t before it computes and casts back, so no
 * operand ever reaches the integer promotion to signed int where a product could
 * overflow. */
typedef struct { _Alignas(16) uint8_t l[16]; } u8x16;

typedef struct { uint32_t lead; u8x16 v; uint32_t trail; } Pair;

static u8x16 vadd(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] + (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vsub(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] - (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vmul(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] * (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vand(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] & (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vor(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] | (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vxor(u8x16 x, u8x16 y) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)((uint32_t)x.l[i] ^ (uint32_t)y.l[i]); }
    return r;
}

static u8x16 vnot(u8x16 x) {
    u8x16 r;
    for (unsigned i = 0; i < 16u; i++) { r.l[i] = (uint8_t)(~(uint32_t)x.l[i]); }
    return r;
}

static uint64_t fold_vec(uint64_t h, u8x16 v) {
    for (unsigned i = 0; i < 16u; i++) { h = mix_u8(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint8_t s = (uint8_t)seed;

    u8x16 a = { { 240, 1, 128, 255, 17, 2, 200, 99, 0, 7, 250, 33, 128, 64, 5, 199 } };
    u8x16 b = { { 17, 255, 129, 1, 200, 100, 57, 21, 255, 3, 6, 222, 128, 192, 251, 57 } };
    const u8x16 ramp = { { 1, 3, 9, 27, 2, 6, 18, 54, 4, 12, 36, 108, 8, 24, 72, 216 } };
    const u8x16 zero = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
    a.l[0] = (uint8_t)((uint32_t)a.l[0] + (uint32_t)s);
    a.l[7] = (uint8_t)((uint32_t)a.l[7] + (uint32_t)s);
    b.l[3] = (uint8_t)((uint32_t)b.l[3] + (uint32_t)s);
    b.l[12] = (uint8_t)((uint32_t)b.l[12] + (uint32_t)s);

    h = fold_vec(h, a);
    h = fold_vec(h, b);
    h = fold_vec(h, vadd(a, b));
    h = fold_vec(h, vsub(a, b));
    h = fold_vec(h, vsub(b, a));
    h = fold_vec(h, vmul(a, b));
    h = fold_vec(h, vmul(a, ramp));
    h = fold_vec(h, vmul(b, ramp));
    h = fold_vec(h, vmul(vadd(a, b), ramp));
    h = fold_vec(h, vsub(zero, a));
    h = fold_vec(h, vsub(zero, b));
    h = fold_vec(h, vand(a, b));
    h = fold_vec(h, vor(a, b));
    h = fold_vec(h, vxor(a, b));
    h = fold_vec(h, vnot(a));
    h = fold_vec(h, vnot(b));
    h = fold_vec(h, vxor(vand(a, b), vor(a, b)));

    u8x16 acc = zero;
    u8x16 sum = zero;
    for (uint64_t i = 0; i < UINT64_C(6); i = (uint64_t)(i + UINT64_C(1))) {
        const u8x16 t = vmul(a, ramp);
        h = fold_vec(h, t);
        acc = vxor(acc, t);
        h = fold_vec(h, acc);
        sum = vadd(sum, a);
        h = fold_vec(h, sum);
        a = vadd(a, b);
        h = fold_vec(h, a);
    }

    u8x16 arr[4];
    arr[0] = a;
    arr[1] = vadd(a, b);
    arr[2] = vmul(a, ramp);
    arr[3] = acc;
    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        h = fold_vec(h, arr[k]);
    }

    Pair r;
    r.lead = UINT32_C(4294967295);
    r.v = arr[2];
    r.trail = UINT32_C(305419896);
    h = mix_u32(h, r.lead);
    h = fold_vec(h, r.v);
    h = mix_u32(h, r.trail);
    return h;
}
