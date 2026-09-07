# fuzzing the untrusted boundaries

One question: does a boundary **answer** every input, or can some input make it
do something other than answer? An answer is a parsed value or a typed
rejection. A crash, a hang, or an allocation that outruns its cap is not an
answer, and every one of those is a finding.

## What is a boundary

A boundary is a place where bytes the compiler did not produce reach code that
believes something about them. Each one is two things that must agree:

- a directory under `corpus/`, holding the retained inputs, and
- a row in the registry in `src/lang/fuzz.mach`, naming the function that
  answers them.

The corpus test fails on a directory that names no registered boundary and on a
boundary whose corpus is empty, so neither side can be added alone. Adding a
boundary is: write `src/lang/fuzz/<name>.mach` with a
`fun(*A.Allocator, *u8, usize) verdict.Verdict`, add one `put` line to
`boundaries`, and fill `corpus/<name>/`. Nothing else changes.

The answer function is handed the candidate's bytes with a NUL written one past
the end, so a boundary over program text can read them as a `str` without
copying. It returns `accepted`, `rejected` with a reason, or `violated` with
one. The third is for a boundary that answered but broke a postcondition it
states for itself, such as a token span reaching past the source or a relocation
writing outside the section it was handed: the process exits normally, so the
harness prints a marker the driver recognises.

The boundaries are the object formats `elf`, `coff`, `macho` and `ar`, the
SPIR-V carrier `spv`, the `lexer` and the `parser`, the `manifest`, inline `asm`
across every dialect, relocation application in `reloc`, the linker's
`riscv-attributes` consumer, the `ir` and `mir` verifiers with the validation
catalog, and `dwarf`.

## What is here

- `corpus/<boundary>/` — the retained corpus. Every file in it is replayed by
  the ordinary test suite, so a finding never comes back silently. The seeds
  named for an artifact are real output a producer emitted; the rest are shapes
  the boundary is likely to get wrong: truncation at each structural boundary,
  saturated counts and lengths, a zeroed header, the magic alone, and nothing at
  all. A text corpus adds unterminated strings and comments, deep nesting,
  maximal identifiers and literals, adjacent token pairs, and invalid UTF-8.
- `run.sh` — the bounded runner. `lib/fuzz.py` holds the driver.
- The harness is `src/lang/fuzz.mach`. Its corpus test replays everything
  retained; its candidate test answers exactly the one file named by
  `MACH_FUZZ_INPUT` at the boundary named by `MACH_FUZZ_BOUNDARY`.

## Running it

```
sh test/fuzz/run.sh                       # 200 candidates per boundary
sh test/fuzz/run.sh --iterations 2000 --seed 4
sh test/fuzz/run.sh --boundary ar --timeout 2 --memory 262144
```

`--seed` makes a run reproducible. `--timeout` is the wall-clock seconds one
candidate may take and `--memory` the address space in KiB it may map; both are
applied per candidate process, so one bad input cannot take the machine with it.

The runner refuses to start on a corpus that does not already pass. A run that
finds something minimizes the input by deleting byte ranges while it still
reproduces, writes it into `corpus/<boundary>/found-<digest>.bin`, and exits
non-zero. Commit that file: from then on the ordinary suite replays it.

A finding whose fix is not this stream's to make goes one level down, into
`corpus/<boundary>/expected/`, and gets an entry in `FINDINGS.md` naming its
owner and its site. The suite does not replay those, because a reproducer that
crashes or hangs would take the suite with it; the runner checks each one on
every run and fails if one has started answering, so a stale record cannot sit
there quietly.

## Mutation

A binary boundary is mutated by flipping, saturating, deleting, and splicing
bytes. A text boundary is mutated with fragments taken from its own corpus,
split on whitespace, as well as with raw bytes: random bytes alone almost never
form a token, so they die in the lexer instead of reaching the grammar. The
alphabet is therefore data, not a table in the driver — a new corpus file widens
it.

## Profiles

The corpus is replayed by the suite in both profiles, so every retained input
must answer in both. They are not interchangeable: release frames are larger,
and a recursion depth that answers in debug can exhaust the stack in release by
a factor of three. `run.sh --profile release` points the candidates at the
release build; the default is debug. A retained input that nests anything should
sit an order of magnitude under the shallowest depth `FINDINGS.md` records.

## Bounds

One candidate is one process. An allocation refusal inside the address-space cap
is an answer, not a finding: a boundary is allowed to say it cannot fit the
input, and is not allowed to die trying. A walk over a self-referential
structure carries its own bound so a cycle ends as a rejection rather than a
hang.
