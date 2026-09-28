/* the reference of the f16 proof (#3804): reads what the subject
 * (src/bin/main.mach) writes for one slice of one mode on stdin and holds every
 * result against a correctly rounded binary16 computed here in integers alone.
 *
 * usage: check <mode> <lo> <hi> <x86|arm|riscv>
 *
 * every finite binary16 is M * 2^E with M < 2^11. a sum is exact in units of
 * 2^-24, a product is the exact integer product, and a quotient is an integer
 * quotient carrying enough bits past the result that its remainder is only a
 * sticky bit. round16 rounds an exact value (plus that sticky bit) once, to
 * nearest with ties to even, with subnormals and overflow to infinity. no
 * floating-point operation takes part, so the reference cannot share a
 * rounding with what it checks.
 *
 * the NaN a result carries is the target's, named by the last argument:
 * x86 returns the first NaN operand, quieted, and makes the negative default
 * NaN; arm gives a signaling operand priority, then the first NaN, and makes
 * the positive default NaN; riscv makes the canonical NaN for every NaN. a
 * converted NaN keeps its sign and the top of its payload, quieted, on x86 and
 * arm, and is canonical on riscv (#4125).
 *
 * the output is one line per variant, `result <mode> <variant> <role> <cases>
 * <mismatches>`, after the first mismatches of each proof variant as
 * `mismatch ...` lines.
 * a proof variant must not mismatch, a control (a mutation) must. the exit is
 * nonzero only when the stream is not the length the slice asks for: the
 * driver judges the counts. */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { RULE_X86, RULE_ARM, RULE_RISCV };
static int rule;

/* how a variant is compared: exact holds every bit, NaNs included; value
 * holds every bit of a non-NaN reference and only NaN-ness otherwise, for the
 * forms written through explicit conversions, which quiet an operand first */
enum { CMP_EXACT, CMP_VALUE };
enum { ROLE_PROOF, ROLE_CONTROL };

#define MAX_VARIANTS 16
#define SHOW 5

struct variant {
    const char *name;
    int cmp, role;
    unsigned long long cases, mismatches;
};

static const char *mode;
static struct variant var[MAX_VARIANTS];
static int nvar;

static void variant(const char *name, int cmp, int role)
{
    var[nvar].name = name;
    var[nvar].cmp = cmp;
    var[nvar].role = role;
    nvar++;
}

/* the stream, read in blocks of 16-bit little-endian words */
static uint16_t block[1 << 16];
static size_t have, at;

static int next16(uint16_t *w)
{
    if (at == have) {
        have = fread(block, 2, sizeof block / 2, stdin);
        at = 0;
        if (have == 0)
            return 0;
    }
    *w = block[at++];
    return 1;
}

static uint64_t word16(void)
{
    uint16_t w;
    if (!next16(&w)) {
        fprintf(stderr, "check: %s: the subject's stream ended early\n", mode);
        exit(3);
    }
    return w;
}

static uint64_t word64(void)
{
    uint64_t v = word16();
    v |= word16() << 16;
    v |= word16() << 32;
    v |= word16() << 48;
    return v;
}

static int is_nan16(uint64_t x) { return (x & 0x7C00) == 0x7C00 && (x & 0x3FF) != 0; }
static int is_snan16(uint64_t x) { return is_nan16(x) && !(x & 0x200); }

/* the input a mismatch line names: one operand, or the pair of an arithmetic mode */
static uint64_t in_a, in_b;
static int in_pair;

/* judge one result of variant v against the reference */
static void judge(int v, uint64_t got, uint64_t want, int nan_want, int nan_got)
{
    struct variant *p = &var[v];
    int ok;
    p->cases++;
    if (p->cmp == CMP_VALUE && nan_want)
        ok = nan_got;
    else
        ok = got == want;
    if (ok)
        return;
    /* a control's mismatches are expected, so only a proof's are shown */
    if (p->role == ROLE_PROOF && p->mismatches < SHOW) {
        if (in_pair)
            printf("mismatch %s %s in %#llx,%#llx got %#llx want %#llx\n", mode, p->name, (unsigned long long)in_a,
                   (unsigned long long)in_b, (unsigned long long)got, (unsigned long long)want);
        else
            printf("mismatch %s %s in %#llx got %#llx want %#llx\n", mode, p->name, (unsigned long long)in_a,
                   (unsigned long long)got, (unsigned long long)want);
    }
    p->mismatches++;
}

static void judge16(int v, uint64_t got, uint64_t want)
{
    judge(v, got, want, is_nan16(want), is_nan16(got));
}

/* a finite nonzero binary16 is M * 2^E */
struct half {
    int sign, kind; /* kind: 0 zero, 1 finite nonzero, 2 infinity, 3 NaN */
    uint64_t m;
    int e;
};
static struct half H[1 << 16];

static void decode_all(void)
{
    for (uint32_t x = 0; x < (1u << 16); x++) {
        struct half *h = &H[x];
        uint32_t ex = (x >> 10) & 31, mant = x & 0x3FF;
        h->sign = (int)(x >> 15);
        if (ex == 31) {
            h->kind = mant ? 3 : 2;
        } else if (ex == 0) {
            h->kind = mant ? 1 : 0;
            h->m = mant;
            h->e = -24;
        } else {
            h->kind = 1;
            h->m = 1024 | mant;
            h->e = (int)ex - 25;
        }
    }
}

static int bitlen(uint64_t n) { return 64 - __builtin_clzll(n); }

/* (-1)^sign * (n + s) * 2^e, s in (0, 1) when sticky and 0 otherwise, rounded
 * once to binary16, to nearest with ties to even. a caller passes a sticky
 * bit only with at least two bits of n below the result's last place */
static uint16_t round16(int sign, uint64_t n, int e, int sticky)
{
    uint16_t s = sign ? 0x8000 : 0;
    if (n == 0) {
        if (sticky) {
            fprintf(stderr, "check: harness defect: a sticky bit with no significand\n");
            exit(4);
        }
        return s;
    }
    int t = bitlen(n) - 1 + e; /* the exponent of the leading bit */
    int q = t - 10;            /* the exponent of the result's last place */
    if (q < -24)
        q = -24;
    int shift = q - e;
    uint64_t Q, R = 0, half = 1;
    if (shift <= 0) {
        if (sticky) {
            fprintf(stderr, "check: harness defect: a sticky bit at the last place\n");
            exit(4);
        }
        Q = n << -shift;
    } else if (shift < 64) {
        Q = n >> shift;
        R = n & ((1ull << shift) - 1);
        half = 1ull << (shift - 1);
    } else if (shift == 64) {
        Q = 0;
        R = n;
        half = 1ull << 63;
    } else {
        /* n < 2^64 <= half the last place, sticky or not */
        Q = 0;
    }
    if (shift > 0 && shift <= 64 && (R > half || (R == half && (sticky || (Q & 1)))))
        Q++;
    if (Q == 2048) {
        Q = 1024;
        q++;
    }
    if (Q < 1024)
        return s | (uint16_t)Q; /* subnormal, q is -24 */
    if (q + 25 >= 31)
        return s | 0x7C00;
    return s | (uint16_t)(((uint64_t)(q + 25) << 10) + (Q - 1024));
}

static uint16_t default_nan(void) { return rule == RULE_X86 ? 0xFE00 : 0x7E00; }

/* the NaN an operation with a NaN operand gives */
static uint16_t pick_nan(uint16_t a, uint16_t b)
{
    if (rule == RULE_RISCV)
        return 0x7E00;
    if (rule == RULE_X86)
        return is_nan16(a) ? (a | 0x200) : (b | 0x200);
    if (is_snan16(a))
        return a | 0x200;
    if (is_snan16(b))
        return b | 0x200;
    return is_nan16(a) ? a : b;
}

static uint16_t ref_add(uint16_t a, uint16_t b, int negate_b)
{
    if (is_nan16(a) || is_nan16(b))
        return pick_nan(a, b);
    struct half x = H[a], y = H[b];
    y.sign ^= negate_b;
    if (x.kind == 2 || y.kind == 2) {
        if (x.kind == 2 && y.kind == 2 && x.sign != y.sign)
            return default_nan();
        return x.kind == 2 ? (x.sign ? 0xFC00 : 0x7C00) : (y.sign ? 0xFC00 : 0x7C00);
    }
    /* exact, in units of 2^-24 */
    int64_t A = x.kind ? (int64_t)(x.m << (x.e + 24)) : 0;
    int64_t B = y.kind ? (int64_t)(y.m << (y.e + 24)) : 0;
    int64_t S = (x.sign ? -A : A) + (y.sign ? -B : B);
    if (S == 0)
        return (x.sign && y.sign && A == 0 && B == 0) ? 0x8000 : 0x0000;
    return round16(S < 0, S < 0 ? (uint64_t)-S : (uint64_t)S, -24, 0);
}

static uint16_t ref_mul(uint16_t a, uint16_t b)
{
    if (is_nan16(a) || is_nan16(b))
        return pick_nan(a, b);
    struct half x = H[a], y = H[b];
    int s = x.sign ^ y.sign;
    if (x.kind == 2 || y.kind == 2) {
        if (x.kind == 0 || y.kind == 0)
            return default_nan();
        return s ? 0xFC00 : 0x7C00;
    }
    if (x.kind == 0 || y.kind == 0)
        return s ? 0x8000 : 0;
    return round16(s, x.m * y.m, x.e + y.e, 0);
}

static uint16_t ref_div(uint16_t a, uint16_t b)
{
    if (is_nan16(a) || is_nan16(b))
        return pick_nan(a, b);
    struct half x = H[a], y = H[b];
    int s = x.sign ^ y.sign;
    if (x.kind == 2)
        return y.kind == 2 ? default_nan() : (s ? 0xFC00 : 0x7C00);
    if (y.kind == 2)
        return s ? 0x8000 : 0;
    if (y.kind == 0)
        return x.kind == 0 ? default_nan() : (s ? 0xFC00 : 0x7C00);
    if (x.kind == 0)
        return s ? 0x8000 : 0;
    /* 24 extra bits give a quotient of at least 14 bits, three past the
     * result's 11, so the remainder only decides the sticky bit */
    uint64_t num = x.m << 24;
    return round16(s, num / y.m, x.e - y.e - 24, num % y.m != 0);
}

static uint16_t ref_narrow32(uint32_t x)
{
    int s = (int)(x >> 31);
    uint32_t ex = (x >> 23) & 0xFF, mant = x & 0x7FFFFF;
    if (ex == 0xFF) {
        if (!mant)
            return s ? 0xFC00 : 0x7C00;
        if (rule == RULE_RISCV)
            return 0x7E00;
        return (uint16_t)((s ? 0x8000 : 0) | 0x7E00 | (mant >> 13));
    }
    if (ex == 0)
        return round16(s, mant, -149, 0);
    return round16(s, mant | 0x800000, (int)ex - 150, 0);
}

static uint16_t ref_narrow64(uint64_t x)
{
    int s = (int)(x >> 63);
    uint64_t ex = (x >> 52) & 0x7FF, mant = x & 0xFFFFFFFFFFFFFull;
    if (ex == 0x7FF) {
        if (!mant)
            return s ? 0xFC00 : 0x7C00;
        if (rule == RULE_RISCV)
            return 0x7E00;
        return (uint16_t)((s ? 0x8000 : 0) | 0x7E00 | (mant >> 42));
    }
    if (ex == 0)
        return round16(s, mant, -1074, 0);
    return round16(s, mant | (1ull << 52), (int)ex - 1075, 0);
}

static uint16_t ref_from_int(int negative, uint64_t magnitude)
{
    return round16(negative, magnitude, 0, 0);
}

static uint16_t ref_from_i64(int64_t v)
{
    return ref_from_int(v < 0, v < 0 ? 0 - (uint64_t)v : (uint64_t)v);
}

/* binary16 widened exactly to a format of `p` significand bits (with the
 * leading one) and exponent bias `bias` */
static uint64_t ref_widen(uint16_t a, int p, int bias, int width)
{
    struct half h = H[a];
    uint64_t sign = (uint64_t)h.sign << (width - 1);
    uint64_t exp_all = ((1ull << (width - p)) - 1) << (p - 1);
    if (h.kind == 0)
        return sign;
    if (h.kind == 2)
        return sign | exp_all;
    if (h.kind == 3) {
        uint64_t quiet = 1ull << (p - 2);
        if (rule == RULE_RISCV)
            return exp_all | quiet;
        return sign | exp_all | quiet | ((uint64_t)(a & 0x3FF) << (p - 11));
    }
    int top = bitlen(h.m) - 1;
    uint64_t frac = (h.m << (p - 1 - top)) & ((1ull << (p - 1)) - 1);
    return sign | ((uint64_t)(top + h.e + bias) << (p - 1)) | frac;
}

static void run_arith(int op, uint64_t lo, uint64_t hi)
{
    in_pair = 1;
    variant("f16", CMP_EXACT, ROLE_PROOF);
    variant("binary32", CMP_VALUE, ROLE_PROOF);
    variant("binary64", CMP_VALUE, ROLE_PROOF);
    variant("twice", CMP_VALUE, ROLE_CONTROL);
    variant("truncated", CMP_VALUE, ROLE_CONTROL);
    for (uint64_t a = lo; a < hi; a++) {
        for (uint64_t b = 0; b < 0x10000; b++) {
            uint16_t want;
            switch (op) {
            case 0: want = ref_add((uint16_t)a, (uint16_t)b, 0); break;
            case 1: want = ref_add((uint16_t)a, (uint16_t)b, 1); break;
            case 2: want = ref_mul((uint16_t)a, (uint16_t)b); break;
            default: want = ref_div((uint16_t)a, (uint16_t)b); break;
            }
            in_a = a;
            in_b = b;
            for (int v = 0; v < 5; v++)
                judge16(v, word16(), want);
        }
    }
}

static void run_conv(const char *m, uint64_t lo, uint64_t hi)
{
    if (!strcmp(m, "narrow32")) {
        variant("binary32", CMP_EXACT, ROLE_PROOF);
    } else if (!strcmp(m, "narrow64")) {
        variant("binary64", CMP_EXACT, ROLE_PROOF);
        variant("binary64+1", CMP_EXACT, ROLE_PROOF);
    } else if (!strcmp(m, "int32")) {
        variant("i32", CMP_EXACT, ROLE_PROOF);
        variant("u32", CMP_EXACT, ROLE_PROOF);
        variant("i64", CMP_EXACT, ROLE_PROOF);
        variant("u64", CMP_EXACT, ROLE_PROOF);
        variant("u64<<32", CMP_EXACT, ROLE_PROOF);
        variant("i64<<32", CMP_EXACT, ROLE_PROOF);
    } else if (!strcmp(m, "int16")) {
        variant("i16", CMP_EXACT, ROLE_PROOF);
        variant("u16", CMP_EXACT, ROLE_PROOF);
        variant("i8", CMP_EXACT, ROLE_PROOF);
        variant("u8", CMP_EXACT, ROLE_PROOF);
    } else if (!strcmp(m, "widen")) {
        variant("binary32", CMP_EXACT, ROLE_PROOF);
        variant("binary64", CMP_EXACT, ROLE_PROOF);
    } else {
        /* per width, the f16 conversion and the binary64 one: in range both
         * truncate exactly, out of range (NaN and infinity too) the f16 one
         * is the target's binary64 result, the rule every float width follows */
        static const char *names[] = {"i8", "i8/binary64", "i16", "i16/binary64", "i32", "i32/binary64",
                                      "i64", "i64/binary64", "u8", "u8/binary64", "u16", "u16/binary64",
                                      "u32", "u32/binary64", "u64", "u64/binary64"};
        for (int i = 0; i < 16; i++)
            variant(names[i], CMP_EXACT, ROLE_PROOF);
    }
    for (uint64_t x = lo; x < hi; x++) {
        in_a = x;
        if (!strcmp(m, "narrow32")) {
            judge16(0, word16(), ref_narrow32((uint32_t)x));
        } else if (!strcmp(m, "narrow64")) {
            judge16(0, word16(), ref_narrow64(x << 32));
            judge16(1, word16(), ref_narrow64((x << 32) | 1));
        } else if (!strcmp(m, "int32")) {
            uint32_t u = (uint32_t)x;
            int32_t i = (int32_t)u;
            uint64_t w = (uint64_t)u << 32;
            judge16(0, word16(), ref_from_i64(i));
            judge16(1, word16(), ref_from_int(0, u));
            judge16(2, word16(), ref_from_i64(i));
            judge16(3, word16(), ref_from_int(0, u));
            judge16(4, word16(), ref_from_int(0, w));
            judge16(5, word16(), ref_from_i64((int64_t)w));
        } else if (!strcmp(m, "int16")) {
            uint16_t u = (uint16_t)x;
            judge16(0, word16(), ref_from_i64((int16_t)u));
            judge16(1, word16(), ref_from_int(0, u));
            judge16(2, word16(), ref_from_i64((int8_t)(uint8_t)u));
            judge16(3, word16(), ref_from_int(0, (uint8_t)u));
        } else if (!strcmp(m, "widen")) {
            uint64_t w32 = ref_widen((uint16_t)x, 24, 127, 32);
            uint64_t w64 = ref_widen((uint16_t)x, 53, 1023, 64);
            uint64_t g32 = word64(), g64 = word64();
            judge(0, g32, w32, 0, 0);
            judge(1, g64, w64, 0, 0);
        } else {
            struct half h = H[x];
            /* the truncated value, when finite */
            uint64_t mag = 0;
            if (h.kind == 1)
                mag = h.e >= 0 ? h.m << h.e : h.m >> -h.e;
            static const int bits[] = {8, 16, 32, 64, 8, 16, 32, 64};
            for (int k = 0; k < 8; k++) {
                int is_signed = k < 4, n = bits[k];
                uint64_t direct = word64(), via = word64();
                int in_range = h.kind < 2;
                uint64_t want = 0;
                if (in_range && is_signed) {
                    /* |v| <= 65504, so the signed range check is on the magnitude */
                    uint64_t lim = h.sign ? (1ull << (n - 1)) : (1ull << (n - 1)) - 1;
                    in_range = mag <= lim;
                    want = h.sign ? 0 - mag : mag;
                } else if (in_range) {
                    in_range = (!h.sign || mag == 0) && (n == 64 || mag < (1ull << n));
                    want = mag;
                }
                uint64_t mask = n == 64 ? ~0ull : (1ull << n) - 1;
                if (in_range) {
                    judge(2 * k, direct, want & mask, 0, 0);
                    judge(2 * k + 1, via, want & mask, 0, 0);
                } else {
                    judge(2 * k, direct, via, 0, 0);
                    var[2 * k + 1].cases++;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 5) {
        fprintf(stderr, "usage: check <mode> <lo> <hi> <x86|arm|riscv>\n");
        return 2;
    }
    mode = argv[1];
    uint64_t lo = strtoull(argv[2], 0, 10), hi = strtoull(argv[3], 0, 10);
    if (!strcmp(argv[4], "x86"))
        rule = RULE_X86;
    else if (!strcmp(argv[4], "arm"))
        rule = RULE_ARM;
    else if (!strcmp(argv[4], "riscv"))
        rule = RULE_RISCV;
    else {
        fprintf(stderr, "check: unknown NaN rule '%s'\n", argv[4]);
        return 2;
    }
    decode_all();
    if (!strcmp(mode, "add"))
        run_arith(0, lo, hi);
    else if (!strcmp(mode, "sub"))
        run_arith(1, lo, hi);
    else if (!strcmp(mode, "mul"))
        run_arith(2, lo, hi);
    else if (!strcmp(mode, "div"))
        run_arith(3, lo, hi);
    else if (!strcmp(mode, "narrow32") || !strcmp(mode, "narrow64") || !strcmp(mode, "int32") ||
             !strcmp(mode, "int16") || !strcmp(mode, "widen") || !strcmp(mode, "toint"))
        run_conv(mode, lo, hi);
    else {
        fprintf(stderr, "check: unknown mode '%s'\n", mode);
        return 2;
    }
    uint16_t extra;
    if (next16(&extra)) {
        fprintf(stderr, "check: %s: the subject's stream runs past the slice\n", mode);
        return 3;
    }
    for (int v = 0; v < nvar; v++)
        printf("result %s %s %s %llu %llu\n", mode, var[v].name, var[v].role == ROLE_PROOF ? "proof" : "control",
               var[v].cases, var[v].mismatches);
    return 0;
}
