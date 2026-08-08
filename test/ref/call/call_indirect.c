#include "corpus.h"

typedef struct { uint64_t a; uint64_t b; uint64_t c; } B24;

typedef uint64_t (*BinOp)(uint64_t, uint64_t);
typedef uint64_t (*RecOp)(B24);
typedef B24 (*MakeOp)(uint64_t);

static inline int32_t as_i32(uint32_t v) { int32_t r; memcpy(&r, &v, sizeof r); return r; }

static uint64_t gen(uint64_t s, uint64_t k) {
    return (uint64_t)(k * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407) + s);
}

static uint64_t op_add(uint64_t a, uint64_t b) { return (uint64_t)(a + b); }
static uint64_t op_sub(uint64_t a, uint64_t b) { return (uint64_t)(a - b); }
static uint64_t op_mul(uint64_t a, uint64_t b) { return (uint64_t)(a * b); }
static uint64_t op_xor(uint64_t a, uint64_t b) { return (uint64_t)(a ^ (b * UINT64_C(3))); }

static uint64_t op_rot(uint64_t a, uint64_t b) {
    const uint64_t r = (uint64_t)((b & UINT64_C(63)) | UINT64_C(1));
    return (uint64_t)((a << r) | (a >> (UINT64_C(64) - r)));
}

static uint64_t op_mix(uint64_t a, uint64_t b) {
    uint64_t x = (uint64_t)(a ^ (b >> 7));
    x = (uint64_t)(x * UINT64_C(1099511628211));
    return (uint64_t)(x ^ (x >> 31));
}

static uint64_t fold24(uint64_t h, B24 r) {
    h = mix_u64(h, r.a);
    h = mix_u64(h, r.b);
    return mix_u64(h, r.c);
}

static uint64_t sum24(B24 r) { return (uint64_t)(r.a + r.b * UINT64_C(3) + r.c); }
static uint64_t xor24(B24 r) { return (uint64_t)(r.a ^ r.b ^ (r.c * UINT64_C(5))); }

static B24 mk_lin(uint64_t v) {
    B24 r = {0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v + UINT64_C(1));
    r.c = (uint64_t)(v + UINT64_C(2));
    return r;
}

static B24 mk_pow(uint64_t v) {
    B24 r = {0, 0, 0};
    r.a = v;
    r.b = (uint64_t)(v * v);
    r.c = (uint64_t)(~v);
    return r;
}

static uint64_t wide(uint64_t a0, uint32_t a1, double a2, uint8_t a3, float a4,
                     uint64_t a5, int32_t a6, double a7, uint16_t a8,
                     uint64_t a9, float a10, uint64_t a11) {
    uint64_t h = fold_init();
    h = mix_u64(h, a0);
    h = mix_u32(h, a1);
    h = mix_f64(h, a2);
    h = mix_u8(h, a3);
    h = mix_f32(h, a4);
    h = mix_u64(h, a5);
    h = mix_i32(h, a6);
    h = mix_f64(h, a7);
    h = mix_u16(h, a8);
    h = mix_u64(h, a9);
    h = mix_f32(h, a10);
    return mix_u64(h, a11);
}

static uint64_t apply(BinOp f, uint64_t a, uint64_t b) { return f(a, b); }

static uint64_t apply_twice(BinOp f, uint64_t a, uint64_t b) { return f(f(a, b), b); }

static BinOp pick(uint64_t k) {
    if (k == 0) { return op_add; }
    else if (k == 1) { return op_sub; }
    else if (k == 2) { return op_mul; }
    else if (k == 3) { return op_xor; }
    else if (k == 4) { return op_rot; }
    return op_mix;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();

    uint64_t v[12];
    for (uint64_t i = 0; i < UINT64_C(12); i = (uint64_t)(i + UINT64_C(1))) {
        v[i] = gen(seed, i);
    }

    BinOp tbl[6];
    tbl[0] = op_add;
    tbl[1] = op_sub;
    tbl[2] = op_mul;
    tbl[3] = op_xor;
    tbl[4] = op_rot;
    tbl[5] = op_mix;

    RecOp rt[2];
    rt[0] = sum24;
    rt[1] = xor24;

    MakeOp mt[2];
    mt[0] = mk_lin;
    mt[1] = mk_pow;

    uint64_t acc = (uint64_t)(seed + UINT64_C(1));
    for (uint64_t k = 0; k < UINT64_C(12); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t idx = (uint64_t)((v[k] + seed) % UINT64_C(6));
        acc = tbl[idx](acc, v[k]);
        h = mix_u64(h, acc);
        h = mix_u64(h, apply(tbl[idx], acc, v[k]));
        h = mix_u64(h, apply_twice(tbl[(idx + UINT64_C(1)) % UINT64_C(6)], acc, v[k]));
        h = mix_u64(h, pick((uint64_t)((v[k] + seed) % UINT64_C(6)))(acc, v[k]));
    }

    BinOp f = tbl[(v[0] + seed) % UINT64_C(6)];
    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        BinOp g = tbl[(v[k] + seed + k) % UINT64_C(6)];
        acc = f(acc, g(acc, v[k]));
        h = mix_u64(h, acc);
        f = g;
    }

    for (uint64_t k = 0; k < UINT64_C(8); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t mi = (uint64_t)((v[k] + seed) % UINT64_C(2));
        const uint64_t ri = (uint64_t)((v[k] + seed + UINT64_C(1)) % UINT64_C(2));
        const B24 r = mt[mi]((uint64_t)(v[k] + acc));
        h = fold24(h, r);
        h = mix_u64(h, rt[ri](r));
        h = mix_u64(h, rt[ri](mt[mi](v[k])));
    }

    uint64_t (*w)(uint64_t, uint32_t, double, uint8_t, float, uint64_t, int32_t,
                  double, uint16_t, uint64_t, float, uint64_t) = wide;
    for (uint64_t k = 0; k < UINT64_C(4); k = (uint64_t)(k + UINT64_C(1))) {
        const uint64_t d = (uint64_t)(v[k] + seed);
        h = mix_u64(h, w(d, (uint32_t)d, (double)(d & UINT64_C(4095)) / 8.0, (uint8_t)d,
                         (float)(d & UINT64_C(255)), (uint64_t)(d * UINT64_C(3)), as_i32((uint32_t)d),
                         (double)(d & UINT64_C(65535)) / 64.0, (uint16_t)d, (uint64_t)(~d),
                         (float)(d & UINT64_C(1023)) - 512.0f, (uint64_t)(d ^ acc)));
        h = mix_u64(h, wide(d, (uint32_t)d, (double)(d & UINT64_C(4095)) / 8.0, (uint8_t)d,
                            (float)(d & UINT64_C(255)), (uint64_t)(d * UINT64_C(3)), as_i32((uint32_t)d),
                            (double)(d & UINT64_C(65535)) / 64.0, (uint16_t)d, (uint64_t)(~d),
                            (float)(d & UINT64_C(1023)) - 512.0f, (uint64_t)(d ^ acc)));
    }

    return h;
}
