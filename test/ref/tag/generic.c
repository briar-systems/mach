#include "corpus.h"

typedef struct { uint32_t a; uint16_t b; } Pair;
typedef struct { uint8_t d; union { uint8_t some; } p; } BoxU8;
typedef struct { uint8_t d; union { int64_t some; } p; } BoxI64;
typedef struct { uint8_t d; union { double some; } p; } BoxF64;
typedef struct { uint8_t d; union { Pair some; } p; } BoxPair;
typedef struct { uint8_t d; union { BoxI64 some; } p; } BoxBox;

static BoxU8 u8_none(void) { BoxU8 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static BoxU8 u8_some(uint8_t x) { BoxU8 v; memset(&v, 0, sizeof v); v.p.some = x; v.d = 1; return v; }
static BoxI64 i64_none(void) { BoxI64 v; memset(&v, 0, sizeof v); v.d = 0; return v; }
static BoxI64 i64_some(int64_t x) { BoxI64 v; memset(&v, 0, sizeof v); v.p.some = x; v.d = 1; return v; }
static BoxF64 f64_some(double x) { BoxF64 v; memset(&v, 0, sizeof v); v.p.some = x; v.d = 1; return v; }
static BoxPair pair_some(Pair x) { BoxPair v; memset(&v, 0, sizeof v); v.p.some = x; v.d = 1; return v; }
static BoxBox box_some(BoxI64 x) { BoxBox v; memset(&v, 0, sizeof v); v.p.some = x; v.d = 1; return v; }

static uint8_t byte_of(BoxU8 b) {
    if (b.d == 1) { return b.p.some; }
    return 255;
}

static int64_t long_of(BoxI64 b) {
    if (b.d == 1) { return b.p.some; }
    return -1;
}

static uint64_t real_of(uint64_t h, BoxF64 b) {
    if (b.d == 1) { return mix_f64(h, b.p.some); }
    return mix_u8(h, 0);
}

static uint32_t pair_of(BoxPair b) {
    if (b.d == 1) { return (uint32_t)(b.p.some.a * UINT32_C(1000) + (uint32_t)b.p.some.b); }
    return 0;
}

static int64_t deep_of(BoxBox b) {
    if (b.d == 1) { return long_of(b.p.some) + 1000000; }
    return -2;
}

static BoxI64 bump_long(BoxI64 b) {
    if (b.d == 1) { return i64_some(b.p.some + 1); }
    return i64_some(0);
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const int64_t s = (int64_t)seed;

    BoxU8 bb;
    memset(&bb, 0, sizeof bb);
    h = mix_u8(h, byte_of(bb));
    bb = u8_some((uint8_t)(UINT64_C(7) + seed));
    h = mix_u8(h, byte_of(bb));
    if (bb.d == 1) { bb.p.some = (uint8_t)(bb.p.some * 2); }
    h = mix_u8(h, byte_of(bb));
    bb = u8_none();
    h = mix_u8(h, byte_of(bb));

    BoxI64 bl;
    memset(&bl, 0, sizeof bl);
    h = mix_i64(h, long_of(bl));
    bl = i64_some(1234567890123 + s);
    h = mix_i64(h, long_of(bl));
    bl = bump_long(bl);
    h = mix_i64(h, long_of(bl));
    h = mix_i64(h, long_of(bump_long(i64_none())));

    BoxF64 br;
    memset(&br, 0, sizeof br);
    h = real_of(h, br);
    br = f64_some(2.5 + (double)s);
    h = real_of(h, br);
    if (br.d == 1) { br.p.some = br.p.some * -3.0; }
    h = real_of(h, br);

    BoxPair bp;
    memset(&bp, 0, sizeof bp);
    h = mix_u32(h, pair_of(bp));
    Pair pr;
    memset(&pr, 0, sizeof pr);
    pr.a = (uint32_t)(UINT32_C(4) + (uint32_t)seed); pr.b = 9;
    bp = pair_some(pr);
    h = mix_u32(h, pair_of(bp));
    if (bp.d == 1) { bp.p.some.b = (uint16_t)(bp.p.some.b + 1); }
    h = mix_u32(h, pair_of(bp));

    BoxBox bd;
    memset(&bd, 0, sizeof bd);
    h = mix_i64(h, deep_of(bd));
    bd = box_some(bl);
    h = mix_i64(h, deep_of(bd));
    bd = box_some(i64_none());
    h = mix_i64(h, deep_of(bd));
    if (bd.d == 1) { bd.p.some = i64_some(42 + s); }
    h = mix_i64(h, deep_of(bd));

    h = mix_u64(h, (uint64_t)sizeof(BoxU8));
    h = mix_u64(h, (uint64_t)sizeof(BoxI64));
    h = mix_u64(h, (uint64_t)sizeof(BoxF64));
    h = mix_u64(h, (uint64_t)sizeof(BoxPair));
    h = mix_u64(h, (uint64_t)sizeof(BoxBox));
    h = mix_u64(h, (uint64_t)_Alignof(BoxPair));
    return h;
}
