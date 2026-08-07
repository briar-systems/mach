/* Measures the flags facts `mach.lang.ct` asserts, against the CPU this runs on.
 *
 * Two probes, both from the same pair of runs: the instruction is executed twice
 * with identical operands, once with RFLAGS preset all-set and once all-clear.
 *
 *   WRITER probe  - read RFLAGS back. A flag with the same result in both runs is
 *                   DEFINED by the instruction; one that comes out equal to its
 *                   input in both is PRESERVED. That is the `defines_flags`
 *                   predicate, measured.
 *   READER probe  - read the DESTINATION REGISTER back. If it differs between the
 *                   two presets, the instruction consumed a flag. That is the
 *                   `reads_flags` predicate, measured.
 */
#include <stdint.h>

#define FSET   0x0CD5UL   /* every arithmetic flag set   */
#define FCLR   0x0002UL   /* every arithmetic flag clear (bit 1 is reserved-1) */
#define ZF     0x0040UL
#define CF     0x0001UL

/* Run INSN once with RFLAGS preset to `pre`, and report BOTH observables from that
   single run: the flags afterwards, and the destination register afterwards. One
   macro rather than two so the writer and reader probes cannot drift apart in the
   operands or the preset they use. */
#define RUN(INSN, A, B, PRE, OUTF, OUTD)                                       \
    do { uint64_t _a = (A), _b = (B), _d = 0, _o;                              \
         __asm__ volatile ("push %[p]\n\tpopfq\n\t" INSN                       \
                           "\n\tpushfq\n\tpop %[o]"                            \
             : [o]"=&r"(_o), [d]"+r"(_d), [x]"+r"(_a), [y]"+r"(_b)             \
             : [p]"r"((uint64_t)(PRE)) : "cc", "memory", "rax");                      \
         (OUTF) = _o; (OUTD) = _d; } while (0)

static const char *verdict(uint64_t hi, uint64_t lo, uint64_t bit) {
    int oh = (hi & bit) != 0, ol = (lo & bit) != 0;
    if (oh == ol) return "defined";
    if (oh == ((FSET & bit) != 0) && ol == ((FCLR & bit) != 0)) return "preserved";
    return "varies";
}

extern int ct_probe_line(const char *name, const char *zf, const char *cf, int reads);
extern int ct_probe_note(const char *name);

#define W2(NAME, INSN, A, B)                                                   \
    do { uint64_t _hi, _lo, _dh, _dl;                                          \
         RUN(INSN, A, B, FSET, _hi, _dh);                                      \
         RUN(INSN, A, B, FCLR, _lo, _dl);                                      \
         ct_probe_line(NAME, verdict(_hi,_lo,ZF), verdict(_hi,_lo,CF), _dh != _dl); \
    } while (0)

void ct_flags_probe(void) {
    /* the ten rows classified `defines_flags`: every one must measure defined/defined */
    W2("add",     "add %[y], %[x]",            5, 3);
    W2("sub",     "sub %[y], %[x]",            5, 3);
    W2("cmp",     "cmp %[y], %[x]",            5, 3);
    W2("test",    "test %[y], %[x]",           5, 3);
    W2("and",     "and %[y], %[x]",            5, 3);
    W2("or",      "or %[y], %[x]",             5, 3);
    W2("xor",     "xor %[y], %[x]",            5, 3);
    W2("neg",     "neg %[x]",                  5, 0);
    W2("neg0",    "neg %[x]",                  0, 0);
    W2("xadd",    "xadd %[y], %[x]",           5, 3);
    /* cmpxchg compares RAX against the destination, so RAX is part of the input and
       both outcomes must be measured: the equal path stores the source and sets ZF,
       the not-equal path loads the destination into RAX and clears it. A row that
       defined flags on only one of its two paths would not be a definer at all. */
    W2("cmpxchgeq", "mov %[x], %%rax\n\tcmpxchg %[y], %[x]", 5, 3);
    W2("cmpxchgne", "mov $9, %%rax\n\tcmpxchg %[y], %[x]",   5, 3);

    /* the traps: partial writers that must NOT be read as defining */
    W2("inc",     "inc %[x]",                  5, 0);
    W2("dec",     "dec %[x]",                  5, 0);
    W2("shl1",    "shl $1, %[x]",              5, 0);
    W2("shl0",    "shl $0, %[x]",              5, 0);
    W2("shr0",    "shr $0, %[x]",              5, 0);
    W2("sar0",    "sar $0, %[x]",              5, 0);

    /* rows classified as writing no observable flag */
    W2("not",     "not %[x]",                  5, 0);
    W2("mov",     "mov %[y], %[x]",            5, 3);
    W2("lea",     "lea 1(%[y]), %[x]",         5, 3);
    W2("xchg",    "xchg %[y], %[x]",           5, 3);

    /* the SETcc rows: the reader probe is the point. they write no flag, and their
       DESTINATION must differ between the two presets - that difference IS the
       laundering path `reads_flags` exists to close. */
    W2("setb",    "setb %b[d]",                5, 3);
    W2("setae",   "setae %b[d]",               5, 3);

    /* A BRANCH CONSUMES A FLAG AND THE READER PROBE CANNOT SEE IT.
       `jc` reads CF, but a taken branch leaves no register difference to measure,
       so the probe reports reads:0. That is a limit of the probe, NOT a fact about
       `jc`, and it is the structural reason the ten `j*` rows do not need
       `reads_flags`: they are refused outright by `cond_flags` before any fold. */
    W2("jc",      "jc 1f\n\t1:",                5, 3);

    /* THE ROWS THIS HARNESS CANNOT CLASSIFY, PRINTED SO THE GAP IS IN THE ARTIFACT
       RATHER THAN ONLY IN A COMMENT.

       `popfq` PASSES THE WRITER PROBE AS "defines BOTH". The incoming flags really
       do not affect the result, so the probe is right on its own terms and wrong as
       a classification: the value came from the stack, which the model does not
       represent and an attacker may control. Classifying it `defines_flags` on this
       evidence would let it clear a taint from an arbitrary source.

       `defines_flags` asks "does this instruction overwrite the flags with a value
       derived from its own operands?" The probe measures only the first half. For
       these three the second half is false, and no experiment of this shape can
       tell you that - the question is where the value came FROM, which is
       structural.

       So: do not extend this harness to classify these rows from their output. If
       you are here because you noticed the probe handles them fine, that is the
       trap. */
    ct_probe_note("popfq");
    ct_probe_note("iretq");
    ct_probe_note("syscall");
}
