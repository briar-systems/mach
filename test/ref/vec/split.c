#include "corpus.h"

/* every probe lane by lane: wrapping integer arithmetic, a shift that
 * saturates at the lane width, a comparison that yields an all-ones or
 * all-zeros mask, and conversions that extend, truncate or round each lane as
 * the scalar rule does. every result folds as its bytes in order. */

static uint64_t next(uint64_t *s) {
    *s = *s * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return *s >> 29;
}

static void fill(uint64_t *s, uint8_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { p[k] = (uint8_t)next(s); }
}

static double fval(uint64_t *s) {
    return (double)((int64_t)(next(s) % 20001u) - 10000) / 8.0;
}

static uint64_t fold_bytes(uint64_t h, const uint8_t *p, unsigned n) {
    for (unsigned k = 0; k < n; k++) { h = mix_u8(h, p[k]); }
    return h;
}

#define LOAD(T, N, name, src) T name[N]; memcpy(name, (src), sizeof name)
#define STORE(dst, name) memcpy((dst), name, sizeof name)

static int16_t shl16(int16_t a, int16_t c) {
    return (c >= 16) ? 0 : (int16_t)(uint16_t)((uint32_t)(uint16_t)a << (unsigned)c);
}

static int16_t shr16(int16_t a, int16_t c) {
    uint16_t x = (uint16_t)a;
    if (c >= 16) { return (x >> 15u) ? (int16_t)-1 : 0; }
    int neg = (x >> 15u) != 0;
    return (int16_t)(neg ? (uint16_t)~(uint16_t)(((uint16_t)~x) >> (unsigned)c) : (uint16_t)(x >> (unsigned)c));
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    uint64_t s = seed + UINT64_C(0x9E3779B97F4A7C15);
    uint8_t buf[128];
    uint8_t out[128];

    fill(&s, buf, 128);
    {
        LOAD(int32_t, 5, a, buf); LOAD(int32_t, 5, b, buf + 20); int32_t r[5];
        for (unsigned i = 0; i < 5; i++) { r[i] = (int32_t)((uint32_t)a[i] + (uint32_t)b[i] * (uint32_t)a[i]); }
        STORE(out, r); h = fold_bytes(h, out, 20);
    }
    {
        LOAD(int64_t, 3, a, buf); LOAD(int64_t, 3, b, buf + 24); int64_t r[3];
        for (unsigned i = 0; i < 3; i++) { r[i] = (int64_t)((uint64_t)a[i] * (uint64_t)b[i] - ((uint64_t)b[i] ^ (uint64_t)a[i])); }
        STORE(out, r); h = fold_bytes(h, out, 24);
    }
    {
        LOAD(uint8_t, 17, a, buf); LOAD(uint8_t, 17, b, buf + 17); uint8_t r[17];
        for (unsigned i = 0; i < 17; i++) { r[i] = (uint8_t)((a[i] & b[i]) | (a[i] ^ (uint8_t)~b[i])); }
        STORE(out, r); h = fold_bytes(h, out, 17);
    }
    {
        int16_t c[9];
        for (unsigned k = 0; k < 9; k++) { c[k] = (int16_t)(next(&s) % 20u); }
        LOAD(int16_t, 9, a, buf); int16_t r[9];
        for (unsigned i = 0; i < 9; i++) { r[i] = (int16_t)(shl16(a[i], c[i]) ^ shr16(a[i], c[i])); }
        STORE(out, r); h = fold_bytes(h, out, 18);
    }
    {
        LOAD(int32_t, 9, a, buf); LOAD(int32_t, 9, b, buf + 36); int32_t r[9];
        for (unsigned i = 0; i < 9; i++) { r[i] = a[i] > b[i] ? a[i] : b[i]; }
        STORE(out, r); h = fold_bytes(h, out, 36);
    }
    {
        int32_t d[6];
        for (unsigned k = 0; k < 6; k++) { d[k] = (int32_t)(next(&s) % 100u) + 1; }
        LOAD(int32_t, 6, a, buf); int32_t r[6];
        for (unsigned i = 0; i < 6; i++) { r[i] = a[i] / d[i]; }
        STORE(out, r); h = fold_bytes(h, out, 24);
    }
    {
        LOAD(int64_t, 3, a, buf); LOAD(int64_t, 3, b, buf + 8); uint64_t r[3];
        for (unsigned i = 0; i < 3; i++) { r[i] = a[i] <= b[i] ? UINT64_MAX : 0; }
        STORE(out, r); h = fold_bytes(h, out, 24);
    }

    fill(&s, buf, 128);
    {
        double x[3], y[3], r[3];
        for (unsigned k = 0; k < 3; k++) {
            x[k] = fval(&s);
            y[k] = fval(&s);
            if (y[k] < 0.0) { y[k] = 0.0 - y[k]; }
            y[k] = y[k] + 1.0;
        }
        for (unsigned i = 0; i < 3; i++) { r[i] = (x[i] + y[i]) * x[i] / y[i]; }
        STORE(out, r); h = fold_bytes(h, out, 24);
        h = mix_f64(h, x[0] + x[1] + x[2]);
    }
    {
        float x[7], y[7]; uint32_t r[7];
        for (unsigned k = 0; k < 7; k++) {
            x[k] = (float)fval(&s);
            y[k] = (float)fval(&s);
        }
        y[2] = x[2];
        for (unsigned i = 0; i < 7; i++) { r[i] = x[i] < y[i] ? UINT32_MAX : 0; }
        STORE(out, r); h = fold_bytes(h, out, 28);
    }
    {
        LOAD(uint8_t, 16, a, buf); LOAD(uint8_t, 16, b, buf + 16); uint16_t r[16];
        for (unsigned i = 0; i < 16; i++) { r[i] = (uint16_t)((uint32_t)a[i] * (uint32_t)b[i]); }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    {
        LOAD(int16_t, 8, a, buf); LOAD(int16_t, 8, b, buf + 16); int32_t r[8];
        for (unsigned i = 0; i < 8; i++) { r[i] = (int32_t)a[i] * (int32_t)b[i]; }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    {
        LOAD(int32_t, 4, a, buf); LOAD(int32_t, 4, b, buf + 16); int64_t r[4];
        for (unsigned i = 0; i < 4; i++) { r[i] = (int64_t)a[i] * (int64_t)b[i]; }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    {
        LOAD(int8_t, 16, a, buf); int32_t r[16];
        for (unsigned i = 0; i < 16; i++) { r[i] = (int32_t)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 64);
    }
    {
        LOAD(uint16_t, 10, a, buf); uint32_t r[10];
        for (unsigned i = 0; i < 10; i++) { r[i] = (uint32_t)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 40);
    }
    {
        LOAD(int32_t, 8, a, buf); double r[8];
        for (unsigned i = 0; i < 8; i++) { r[i] = (double)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 64);
    }
    {
        LOAD(int32_t, 8, a, buf); int16_t r[8];
        for (unsigned i = 0; i < 8; i++) { r[i] = (int16_t)(uint16_t)(uint32_t)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 16);
    }
    {
        LOAD(int64_t, 4, a, buf); int16_t r[4];
        for (unsigned i = 0; i < 4; i++) { r[i] = (int16_t)(uint16_t)(uint64_t)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 8);
    }
    {
        LOAD(int32_t, 10, a, buf); int8_t r[10];
        for (unsigned i = 0; i < 10; i++) { r[i] = (int8_t)(uint8_t)(uint32_t)a[i]; }
        STORE(out, r); h = fold_bytes(h, out, 10);
    }
    {
        LOAD(int8_t, 40, a, buf); int8_t r[40];
        for (unsigned i = 0; i < 40; i++) { r[i] = (int8_t)(uint8_t)(uint32_t)((int32_t)a[i] * (int32_t)a[i]); }
        STORE(out, r); h = fold_bytes(h, out, 40);
    }

    fill(&s, buf, 128);
    {
        double x[8];
        for (unsigned k = 0; k < 8; k++) { x[k] = fval(&s) * 1.25; }
        float p[8]; double e[8]; int32_t t[6]; float q[8];
        for (unsigned i = 0; i < 8; i++) { p[i] = (float)x[i]; }
        STORE(out, p); h = fold_bytes(h, out, 32);
        for (unsigned i = 0; i < 8; i++) { e[i] = (double)p[i]; }
        STORE(out, e); h = fold_bytes(h, out, 64);
        for (unsigned i = 0; i < 6; i++) { t[i] = (int32_t)x[1 + i]; }
        STORE(out, t); h = fold_bytes(h, out, 24);
        static const unsigned perm[8] = {7, 0, 5, 2, 6, 1, 4, 3};
        for (unsigned i = 0; i < 8; i++) { q[i] = p[perm[i]]; }
        STORE(out, q); h = fold_bytes(h, out, 32);
    }
    { LOAD(int32_t, 8, v, buf); STORE(out, v); memmove(out, out + 12, 20); h = fold_bytes(h, out, 20); }
    { LOAD(int32_t, 8, v, buf); STORE(out, v); memmove(out, out + 8, 16); h = fold_bytes(h, out, 16); }
    { memcpy(out, buf + 16, 16); h = fold_bytes(h, out, 16); }
    { memcpy(out, buf + 7, 20); h = fold_bytes(h, out, 20); }
    {
        LOAD(int16_t, 16, v, buf); int32_t r[8];
        for (unsigned i = 0; i < 8; i++) { r[i] = (int32_t)v[8 + i]; }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    {
        LOAD(int64_t, 5, w, buf);
        int64_t x = (int64_t)(seed - 5u);
        w[3] = x;
        w[4] = (int64_t)((uint64_t)w[0] + (uint64_t)x);
        STORE(out, w); h = fold_bytes(h, out, 40);
    }
    {
        LOAD(int32_t, 8, v, buf); uint32_t sum = 0;
        for (unsigned i = 0; i < 8; i++) { sum += (uint32_t)v[i]; }
        h = mix_i32(h, (int32_t)sum);
    }
    {
        uint8_t acc = 0;
        for (unsigned i = 0; i < 32; i++) { acc = (uint8_t)(acc ^ (uint8_t)(buf[i] + i)); }
        h = mix_u8(h, acc);
    }
    {
        LOAD(int32_t, 32, p, buf); uint32_t acc[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        for (unsigned k = 0; k < 4; k++) {
            for (unsigned i = 0; i < 8; i++) { acc[i] = acc[i] * 3u + (uint32_t)p[k * 8 + i]; }
        }
        STORE(out, acc); h = fold_bytes(h, out, 32);
    }
    {
        LOAD(uint16_t, 12, v, buf); uint16_t r[12];
        for (unsigned i = 0; i < 12; i++) { r[i] = (uint16_t)(v[i] + v[i]); }
        STORE(out, r); h = fold_bytes(h, out, 24);
    }
    { memcpy(out, buf, 32); h = fold_bytes(h, out, 32); }
    { memcpy(out, buf, 32); h = fold_bytes(h, out, 32); }
    for (unsigned pick = 0; pick < 2; pick++) {
        LOAD(int32_t, 8, a, buf); LOAD(int32_t, 8, b, buf + 32); int32_t r[8];
        const int32_t *c = ((seed + pick) & 1u) ? b : a;
        for (unsigned i = 0; i < 8; i++) { r[i] = (int32_t)((uint32_t)c[i] + (uint32_t)a[i]); }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    {
        static const uint32_t k[8] = {1, 20, 300, 4000, 50000, 600000, 7000000, 80000000};
        LOAD(uint32_t, 8, v, buf); uint32_t r[8];
        for (unsigned i = 0; i < 8; i++) { r[i] = v[i] + k[i]; }
        STORE(out, r); h = fold_bytes(h, out, 32);
    }
    return h;
}
