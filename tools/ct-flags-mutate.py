#!/usr/bin/env python3
"""Mutation harness for the #[oblivious] inline-asm flags facts (#2460).

For every row of the x86-64 grammar, flip one flags fact in the direction that
would let a leak through, and assert the suite notices. A mutation the suite does
NOT notice is a fact nothing depends on, which is indistinguishable from a fact
that is wrong.

Run from the repo root:

    python3 tools/ct-flags-mutate.py

It rewrites `x64/encode.mach` once per mutation by renaming the shipping
classifier and wrapping it, then restores the file byte-for-byte. The pristine
copy lives outside the tree so an interrupted run cannot leave a mutated
classifier staged. Takes roughly 10 minutes: one full `mach test .` per
applicable mutation.

WHAT THIS PROVES, AND WHAT IT DOES NOT. It shows each fact is load-bearing: some
test observes it. It does NOT show any fact is TRUE — a row that is confidently
wrong in a self-consistent way passes every mutation here. Truth is measured
separately, against the CPU, by `int/surface/ct-flags-hardware`.

TWO MODES, AND THE DEFAULT IS THE HONEST ONE.

Three unit tests in `x64/encode.mach` assert the facts DIRECTLY - they read
`defines_flags` and friends off the classifier and check them. With those active
almost every mutation dies, but most of those kills are the table catching a
change to itself, which says nothing about whether the walk depends on the fact.

    --behavioural   (default) stub those tests out in the pristine copy, so a
                    mutation can only be caught by a test that observes the
                    COMPILER'S VERDICT on a program. This is the number to report.
    --with-asserts  leave them in. Useful only to confirm the fact-pinning tests
                    themselves still work.

On the shipping tree these differ sharply: 56 killed behaviourally versus nearly
everything with the assertions in. If you find yourself reporting the larger
number, you are reporting that the table agrees with itself.

Read the survivor list, never just the count. A survivor is either a structural
impossibility (fine, and the three classes are named in the PR for #2460) or a
gap in the tests (not fine). They look identical from the outside, and the only
way to tell is to work out why no program could observe it.
"""
import subprocess, shutil, sys, os

W = os.environ.get("MACH_ROOT", os.getcwd())
SRC = W + "/src/lang/target/isa/x64/encode.mach"
# the untouched copy every mutation is built from, and what SRC is restored to.
# written beside the run rather than in the tree, so an interrupted run cannot
# leave a mutated classifier committed.
PRISTINE = os.environ.get("CT_PRISTINE", "/tmp/ct-flags-pristine.mach")
# the mutation BASE (possibly with fact-asserting tests stubbed) and the byte-exact
# ORIGINAL are two different files. Restoring the tree from the base would delete
# whatever the base stubbed out, which is a silent edit to the shipped source.
ORIGINAL = PRISTINE + ".orig"

DEFINES = "ADD SUB AND OR XOR CMP TEST NEG CMPXCHG XADD".split()
WRITTEN = "INC DEC SHL SHR SAR".split()
OPAQUE  = "POPFQ IRETQ SYSCALL".split()
READ    = "SETB SETAE".split()
BRANCH  = "JE JNE JB JAE".split()
# every ROW of the shipping grammar, name -> code. rows outnumber codes: `jz` and
# `je` are one instruction under two spellings, and both are mutated, because the
# bar is stated per row.
ROWS = [l.strip().split("|") for l in
        subprocess.run(["python3","-c","""
import re,sys
t=open(sys.argv[1]).read()
for m in re.finditer(r'name: *"([a-z0-9 _]+)", *code: *x64\\.([A-Z0-9_]+)', t):
    print(m.group(1)+"|"+m.group(2))
""", SRC], capture_output=True, text=True).stdout.splitlines() if l.strip()]

def facts(code):
    """the facts the shipping classifier sets for a code, so a mutation that cannot
    change anything is reported as a no-op rather than counted as a survivor."""
    if code in DEFINES: return dict(writes=1, defines=1, opaque=0, reads=0)
    if code in WRITTEN: return dict(writes=1, defines=0, opaque=0, reads=0)
    if code in OPAQUE:  return dict(writes=1, defines=0, opaque=1, reads=0)
    if code in READ:    return dict(writes=0, defines=0, opaque=0, reads=1)
    return dict(writes=0, defines=0, opaque=0, reads=0)

MUT = {
    # the row stops tainting: catches a fact that must taint
    "no_taint":    "a.writes_flags = false; a.opaque_flags = false;",
    # the row starts clearing: catches a fact that must NOT clear (unsound direction)
    "force_clear": "a.writes_flags = true; a.defines_flags = true;",
    # the row stops propagating out of FLAGS: catches the setcc laundering guard
    "no_read":     "a.reads_flags = false;",
    # the row stops clearing: catches a clear nothing relies on
    "no_clear":    "a.defines_flags = false;",
}

def plan():
    """the full 60-row x 3-fact grid the issue states as the bar.

    Each row gets one mutation per falsifiable fact:
      no_taint    - stop tainting        (writes_flags / opaque_flags -> off)
      force_clear - start clearing       (defines_flags -> on)
      no_read     - stop propagating     (reads_flags -> off)
    A mutation that cannot change the class for that row is a NO-OP, reported as
    such: counting it either way would misstate the evidence."""
    out = []
    for name, code in ROWS:
        f = facts(code)
        out.append((name, code, "no_taint",    bool(f["writes"] or f["opaque"])))
        out.append((name, code, "force_clear", not f["defines"]))
        out.append((name, code, "no_read",     bool(f["reads"])))
    return out

def apply(code, mut):
    t = open(PRISTINE).read()
    assert "pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass {" in t
    t = t.replace("pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass {",
                  "fun asm_ct_class_mutated_base(code: u32, flags: u16) ct.AsmClass {", 1)
    wrapper = (
        "pub fun asm_ct_class(code: u32, flags: u16) ct.AsmClass {\n"
        "    var a: ct.AsmClass = asm_ct_class_mutated_base(code, flags);\n"
        "    if (code == x64.%s::u32) { %s }\n"
        "    ret a;\n"
        "}\n\n" % (code, MUT[mut])
    )
    t = wrapper + t if False else t.replace(
        "fun asm_ct_class_mutated_base(code: u32, flags: u16) ct.AsmClass {",
        wrapper + "fun asm_ct_class_mutated_base(code: u32, flags: u16) ct.AsmClass {", 1)
    open(SRC, "w").write(t)

def run():
    r = subprocess.run(["mach", "test", "."], cwd=W, capture_output=True, text=True)
    return r.returncode == 0   # True == suite green == mutation SURVIVED

# Take the pristine copy from the tree at startup, and refuse to run if the tree is
# already mutated - a previous run that died mid-mutation would otherwise have its
# wrapper baked into the baseline, and every subsequent result would be measured
# against a classifier nobody wrote.
BEHAVIOURAL = "--with-asserts" not in sys.argv

# the tests that assert the facts directly rather than observing a verdict
FACT_TESTS = ["permitting_surface_is_bounded",
              "flag_reader_set_is_exactly_fifteen",
              "every_grammar_row_states_flags"]

_src = open(SRC).read()
if "asm_ct_class_mutated_base" in _src:
    sys.exit("refusing to start: %s still carries a mutation wrapper.\n"
             "restore it (git checkout) before running." % SRC)

if BEHAVIOURAL:
    for _n in FACT_TESTS:
        _h = 'test "mach.lang.target.isa.x64.encode.asm_ct_class:%s" {' % _n
        _i = _src.index(_h)
        _j = _src.index("\n}\n", _src.index("ret ", _i)) + 3
        _src = _src[:_i] + _h + "\n    ret 0;\n}\n" + _src[_j:]
open(ORIGINAL, "w").write(open(SRC).read())
open(PRISTINE, "w").write(_src)
print("mode: %s" % ("behavioural (fact-asserting tests stubbed)" if BEHAVIOURAL
                    else "with-asserts"), flush=True)

killed, survived, noop = [], [], []
tasks = plan()
print("grid: %d mutations (%d rows x 3 facts)" % (len(tasks), len(ROWS)), flush=True)
for i, (name, code, mut, applicable) in enumerate(tasks):
    if not applicable:
        noop.append((name, mut)); continue
    apply(code, mut)
    green = run()
    (survived if green else killed).append((name, mut))
    print("[%3d/%3d] %-14s %-12s %s" % (i+1, len(tasks), name, mut,
          "SURVIVED" if green else "killed"), flush=True)
shutil.copy(ORIGINAL, SRC)

print("\n=== RESULT ===")
print("grid:      %d" % len(tasks))
print("applicable:%d" % (len(killed)+len(survived)))
print("killed:    %d" % len(killed))
print("survived:  %d" % len(survived))
print("no-op:     %d  (fact not set on that row; the mutation changes nothing)" % len(noop))
for n, m in survived:
    print("  SURVIVOR %-14s %s" % (n, m))
