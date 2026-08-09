#include "corpus.h"

typedef struct { uint32_t a; uint32_t b; uint64_t c; uint32_t d; } Consts;

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;

    uint32_t w32;
    if (s == UINT64_C(0)) {
        w32 = UINT32_C(2047);                /* 2^11 - 1 */
    } else {
        w32 = UINT32_C(2147483648);          /* 2^31 */
    }
    h = mix_u32(h, w32);

    uint64_t w64;
    if (s == UINT64_C(1)) {
        w64 = UINT64_C(4294967295);          /* 2^32 - 1 */
    } else {
        w64 = UINT64_C(9223372036854775807); /* 2^63 - 1 */
    }
    h = mix_u64(h, w64);

    uint32_t wb1;
    if (s == UINT64_C(2)) {
        wb1 = UINT32_C(1431655765);          /* 0x55555555 */
    } else {
        wb1 = UINT32_C(252645135);           /* 0x0f0f0f0f */
    }
    h = mix_u32(h, wb1);

    uint64_t wb2;
    if (s == UINT64_C(3)) {
        wb2 = UINT64_C(6148914691236517205); /* 0x5555555555555555 */
    } else {
        wb2 = UINT64_C(18446462603027742720); /* 0xffff0000ffff0000 */
    }
    h = mix_u64(h, wb2);

    const uint64_t arr[3] = {
        UINT64_C(4294967296),             /* 2^32 */
        UINT64_C(9223372036854775808),    /* 2^63 */
        UINT64_C(4294901760)              /* 0xffff0000 */
    };
    const uint64_t ai = s % UINT64_C(3);
    h = mix_u64(h, arr[ai]);
    h = mix_u64(h, arr[(ai + UINT64_C(1)) % UINT64_C(3)]);
    h = mix_u64(h, arr[(ai + UINT64_C(2)) % UINT64_C(3)]);

    Consts rc;
    rc.a = UINT32_C(2047);
    rc.b = UINT32_C(2147483648);
    rc.c = UINT64_C(9223372036854775808);
    rc.d = UINT32_C(1431655765);
    rc.a = (uint32_t)(rc.a + (uint32_t)s);
    h = mix_u32(h, rc.a);
    h = mix_u32(h, rc.b);
    h = mix_u64(h, rc.c);
    h = mix_u32(h, rc.d);

    return h;
}
