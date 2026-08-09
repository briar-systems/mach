/* the C reference side of the corpus: the same FNV-1a fold as lib/fold.mach, plus
 * the entry that prints the checksum in the one shape the case contract allows.
 *
 * every operation here is defined in C. the fold is unsigned throughout, signed
 * inputs reach it through the two's-complement identity, and floats reach it
 * through memcpy rather than a type-punned load. a reference file that disagrees
 * with mach must indict mach, so nothing in this header may be UB. */

#ifndef CORPUS_H
#define CORPUS_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CORPUS_FNV_INIT  UINT64_C(14695981039346656037)
#define CORPUS_FNV_PRIME UINT64_C(1099511628211)

static inline uint64_t fold_init(void) { return CORPUS_FNV_INIT; }

static inline uint64_t mix_u64(uint64_t h, uint64_t v) {
    for (unsigned i = 0; i < 8u; i++) {
        h = (uint64_t)(h ^ ((v >> (i * 8u)) & UINT64_C(255)));
        h = (uint64_t)(h * CORPUS_FNV_PRIME);
    }
    return h;
}

static inline uint64_t mix_u8(uint64_t h, uint8_t v)   { return mix_u64(h, (uint64_t)v); }
static inline uint64_t mix_u16(uint64_t h, uint16_t v) { return mix_u64(h, (uint64_t)v); }
static inline uint64_t mix_u32(uint64_t h, uint32_t v) { return mix_u64(h, (uint64_t)v); }

static inline uint64_t mix_i8(uint64_t h, int8_t v)    { return mix_u64(h, (uint64_t)(int64_t)v); }
static inline uint64_t mix_i16(uint64_t h, int16_t v)  { return mix_u64(h, (uint64_t)(int64_t)v); }
static inline uint64_t mix_i32(uint64_t h, int32_t v)  { return mix_u64(h, (uint64_t)(int64_t)v); }
static inline uint64_t mix_i64(uint64_t h, int64_t v)  { return mix_u64(h, (uint64_t)v); }

static inline uint64_t mix_f32(uint64_t h, float v) {
    uint32_t bits;
    memcpy(&bits, &v, sizeof bits);
    return mix_u64(h, (uint64_t)bits);
}

static inline uint64_t mix_f64(uint64_t h, double v) {
    uint64_t bits;
    memcpy(&bits, &v, sizeof bits);
    return mix_u64(h, bits);
}

uint64_t checksum(uint64_t seed);

int main(int argc, char **argv) {
    (void)argv;
    printf("%016" PRIx64 "\n", checksum((uint64_t)(argc - 1)));
    return 0;
}

#endif
