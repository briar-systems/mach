#include "corpus.h"

typedef struct { uint64_t w[96]; } Wide;
typedef struct { uint8_t b[40]; } Small;
typedef struct { uint8_t b[203]; } Odd;

static uint64_t fold_words(uint64_t h, const uint64_t *p, uint64_t n) {
    for (uint64_t i = 0; i < n; i++) h = mix_u64(h, p[i]);
    return h;
}

static uint64_t fold_bytes(uint64_t h, const uint8_t *p, uint64_t n) {
    for (uint64_t i = 0; i < n; i++) h = mix_u8(h, p[i]);
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const uint64_t s = seed;
    const uint64_t k = s & 3;

    uint64_t words[256];
    for (uint64_t i = 0; i < 256; i++) words[i] = (uint64_t)(i * UINT64_C(6364136223846793005)) ^ s;

    memmove(&words[10], &words[13 + k], sizeof(Wide));
    h = fold_words(h, words, 256);
    memmove(&words[40 + k], &words[1], sizeof(Wide));
    h = fold_words(h, words, 256);
    memmove(&words[7], &words[7], sizeof(Wide));
    h = fold_words(h, words, 256);

    uint8_t bytes[512];
    for (uint64_t i = 0; i < 512; i++) bytes[i] = (uint8_t)((i * 131) + s);

    memmove(&bytes[3], &bytes[11 + k], sizeof(Small));
    h = fold_bytes(h, bytes, 512);
    memmove(&bytes[29 + k], &bytes[5], sizeof(Small));
    h = fold_bytes(h, bytes, 512);
    memmove(&bytes[101], &bytes[150 + k], sizeof(Odd));
    h = fold_bytes(h, bytes, 512);
    memmove(&bytes[200 + k], &bytes[97], sizeof(Odd));
    h = fold_bytes(h, bytes, 512);
    memmove(&bytes[61], &bytes[61], sizeof(Odd));
    h = fold_bytes(h, bytes, 512);
    return h;
}
