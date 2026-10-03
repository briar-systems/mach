#include "corpus.h"

/* the plain layouts the generic ones must match. a write through one union
 * member is read through the other in the low bytes, so the 16-bit write over a
 * 32-bit member is read back through a union in C as well, and the u64 member of
 * the second instance overlaps the u32 at its low half on a little-endian target
 * and its high half on a big-endian one; every corpus target is little-endian. */

typedef struct { uint8_t tag; union { uint32_t a; uint32_t b; } v; } AnonInPlain;
typedef struct { uint8_t tag; union { uint16_t a; uint32_t b; } v; } NestedPlain;
typedef union { uint32_t a; uint32_t b; } NamedPlain;
typedef union { uint64_t a; uint32_t b; } NamedU64;

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    /* AnonInGeneric[u32, u32] and AnonInPlain */
    for (int k = 0; k < 2; k++) {
        AnonInPlain p;
        p.v.b = (uint32_t)(2u + s);
        p.v.a = (uint32_t)(1u + s);
        h = mix_u8(h, 1);
        h = mix_u32(h, p.v.b);
    }
    h = mix_u64(h, (uint64_t)sizeof(AnonInPlain));
    h = mix_u64(h, (uint64_t)sizeof(AnonInPlain));

    /* NamedGeneric[u32, u32] and NamedPlain */
    for (int k = 0; k < 2; k++) {
        NamedPlain p;
        p.b = (uint32_t)(2u + s);
        p.a = (uint32_t)(1u + s);
        h = mix_u8(h, 1);
        h = mix_u32(h, p.b);
    }
    h = mix_u64(h, (uint64_t)sizeof(NamedPlain));
    h = mix_u64(h, (uint64_t)sizeof(NamedPlain));

    /* NamedGeneric[u64, u32] */
    NamedU64 w;
    w.b = (uint32_t)(2u + s);
    w.a = (uint64_t)(uint32_t)(1u + s);
    h = mix_u8(h, 1);
    h = mix_u32(h, w.b);
    h = mix_u64(h, (uint64_t)sizeof(NamedU64));

    /* NamedInGeneric[u16] and NestedPlain */
    NestedPlain n;
    n.v.b = (uint32_t)(2u + s);
    n.v.a = (uint16_t)(1u + s);
    h = mix_u8(h, 1);
    h = mix_u32(h, n.v.b);
    h = mix_u64(h, (uint64_t)sizeof(NestedPlain));
    h = mix_u64(h, (uint64_t)sizeof(NestedPlain));

    /* RecGeneric[u32, u32]: separate fields */
    h = mix_u8(h, 0);
    h = mix_u32(h, (uint32_t)(2u + s));
    h = mix_u32(h, (uint32_t)(1u + s));
    return h;
}
