#include "corpus.h"

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint32_t s = (uint32_t)seed;

    for (uint32_t k = 0; k < UINT32_C(20); k = (uint32_t)(k + UINT32_C(1))) {
        const uint32_t v = (uint32_t)(k + s);
        uint32_t r = 0;

        if (v < UINT32_C(4)) {
            if (v == UINT32_C(0)) { r = UINT32_C(100); }
            else if (v == UINT32_C(1)) { r = UINT32_C(101); }
            else if (v == UINT32_C(2)) { r = UINT32_C(102); }
            else { r = UINT32_C(103); }
        } else if (v < UINT32_C(8)) {
            if (v < UINT32_C(6)) {
                if (v == UINT32_C(4)) { r = UINT32_C(200); } else { r = UINT32_C(201); }
            } else {
                if (v == UINT32_C(6)) { r = UINT32_C(202); } else { r = UINT32_C(203); }
            }
        } else if (v < UINT32_C(12)) {
            if (v == UINT32_C(8)) { r = UINT32_C(300); }
            else if (v == UINT32_C(9)) { r = UINT32_C(301); }
            else if (v == UINT32_C(10)) { r = UINT32_C(302); }
            else { r = UINT32_C(303); }
        } else if (v < UINT32_C(16)) {
            if (v < UINT32_C(14)) {
                if (v == UINT32_C(12)) { r = UINT32_C(400); } else { r = UINT32_C(401); }
            } else {
                if (v == UINT32_C(14)) { r = UINT32_C(402); } else { r = UINT32_C(403); }
            }
        } else {
            if (v == UINT32_C(16)) { r = UINT32_C(500); }
            else if (v == UINT32_C(17)) { r = UINT32_C(501); }
            else if (v == UINT32_C(18)) { r = UINT32_C(502); }
            else { r = UINT32_C(503); }
        }

        h = mix_u32(h, r);
        h = mix_u32(h, v);
    }

    return h;
}
