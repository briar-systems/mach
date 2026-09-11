# Migration compiler pin (#3218, #3219)

The record S1 (mach-std#617, the std 2.0.0 migration) and Track C build
against: the exact feat/3218 state the L10 acceptance was taken on, what built
it, and what it proved. It is a document, not a tag; the owner tags at
release. It supersedes the L8 candidate record (94b834983 on `feat/3218-l8`,
base 872e1c6f), whose base moved when dev was merged through PR #3273.

## The pin

| | |
| --- | --- |
| branch | `feat/3218` (PR #3236), with dev fully merged through PR #3276 (N5 phase 1) |
| measured tree | commit `860b121f8` (the L10 lane `feat/3218-l10` merged into `feat/3218`; every bar below was taken on this commit by the coordinator, in one worktree, after the merge) |
| pinned head | the commit on top of `860b121f8` that carries this document, which is what #3236 merges into `dev`; no compiler source, test, corpus case or golden differs between the two |
| std pin | `168a9f760d7c0f7a182f3b0685081e62f1a4f682` at `dep/std` (the old pin, std 1.0.1; the std 2.0.0 migration is S1's) |
| seed that built it | Mach 4.30.0 at `/home/octalide/.local/bin/mach`, sha256 `29b9264cfd5477419a190a8d6649e9ae8302744d935991ac8ffc47f2b4c26535` |
| host | linux x86_64 natively; aarch64 and riscv64 under qemu-user (qemu-riscv64 11.1.0); cc 16.2.1; llvm-objdump 22.1.8; SPIRV-Tools 2026.3 (`test/tools.lock`) |

The compiler's own source uses no v5 syntax, so the 4.30.0 seed builds it.
Every `sel` identifier in the compiler and std trees was renamed before the
keyword was reserved (L2), and the seed-built generation A compiles the
compiler's source with the new lexer.

## Fixpoint

| generation | built by | command | sha256 |
| --- | --- | --- | --- |
| A | the 4.30.0 seed | `mach build . -o out/audit/mach` | `a881f3c288cadee62d3ffd302af46ca5f8f0d41216ce183a9d26e8b0ada130eb` |
| B | A | `out/audit/mach build . -o out/audit/mach-b` | `b1fe8a8747120bebae334fbb6dc196342bd77875c0667901983b9e0b0b6a13cd` |
| C | B | `out/audit/mach-b build . -o out/audit/mach-c` | `b1fe8a8747120bebae334fbb6dc196342bd77875c0667901983b9e0b0b6a13cd` |

`cmp out/audit/mach-b out/audit/mach-c`: byte-identical. A differs from B
because the seed's code generation differs from this tree's; the invariant is
B == C (three generations, as the fin-semantics work established). The
release-profile compiler used for the second suite run is
`mach build . --profile release -o out/audit/mach-release`, built by the seed.

## What the acceptance proved

Numbers as measured on the measured tree with generation B (the fixpoint
compiler) for the corpus, link, vecrows and debug suite, and the release build
of generation A for the release suite. The L10 lane's own run at its lane head
(`l10-acceptance-3218.md` section 6) agrees on every corpus, link and vecrows
count; the suite grew by N5's five tests between the two.

| check | result |
| --- | --- |
| unit suite, debug from-source compiler (generation B, `out/b/mach test .`) | 2916 passed, 0 failed, 2916 total (2911 at the L10 lane head plus the five N5 phase 1 tests merged from dev) |
| unit suite, release from-source compiler (`out/a/mach build . --profile release -o out/rel/mach`, then `out/rel/mach test .`) | 2916 passed, 0 failed, 2916 total (agrees with the debug build) |
| `sh test/census.sh` | every census ok |
| corpus layers A and B, nine columns | 2625 pass, 0 fail, 623 skip over the nine columns (per-column counts in `migration-compiler-3218.md`); every skip is a declared SKIPS entry or a verified `g` refusal |
| corpus layer C, x86_64-linux (native) and riscv64-linux (qemu) | 408 pass, 0 fail, 0 skip (102 cases, o0 and o2 on each of the two executing columns, every checksum equal to the C reference) |
| link leg, x86_64-linux (`MACH_LINK_MACH=out/b/mach bash test/link/run.sh --leg x86_64-linux`) | 140 pass / 0 fail / 0 skip over 140 cells (debug and release) |
| link leg, riscv64-linux | not run on this host (no riscv64 libc headers); CI runs it |
| vecrows, x86_64-linux | ok, 184 probe cells, 0 declared exceptions |

Corpus cell counts per target column after this lane (102 cases: 92 before,
nine `tag` cases and `cmp/ret_chain_inline`):

| column | layer A | layer B | layer C |
| --- | --- | --- | --- |
| x86_64-linux | 306 pass (g, o0, o2) | 102 pass | 204 pass (native) |
| aarch64-linux | 306 pass | 102 pass | not on this host (CI arm runner) |
| riscv64-linux | 306 pass | 102 pass | 204 pass (qemu-riscv64) |
| spirv | 186 pass, 104 skip recorded (102 `g` policy refusals verified, `mem/rec_layout` a:o0, `cmp/ret_chain_inline` a:o2), 16 cells declared away by the eight TARGET LIMITATION entries | 93 pass, 9 skip (eight target limitations, `cmp/ret_chain_inline` #3275) | none (engine none) |
| x86_64-windows | 204 pass, 102 skip (`g` unsupported, refusal verified) | 102 pass | none on this host |
| x86_64-darwin | 306 pass | 102 pass | none on this host |
| aarch64-darwin | 306 pass | 102 pass | none on this host |
| riscv32 | 102 skip (declared) | 102 skip (declared) | none |
| mos6502 | 102 skip (declared) | 102 skip (declared) | none |
| total | 2625 pass, 0 fail, 623 skip (layers A and B together) | | 408 pass, 0 fail |

Layer B goldens: 71 new files (nine tag cases on seven native columns and
spirv, `cmp/ret_chain_inline` on seven native columns); no existing golden
moved.

## Reproducing it on a clean checkout

```sh
cd /abs/path/to/mach || exit 1
git fetch -q origin feat/3218
git checkout -q 860b121f8
rm -rf dep/std && git clone -q https://github.com/briar-systems/mach-std dep/std \
    && git -C dep/std checkout -q 168a9f760d7c0f7a182f3b0685081e62f1a4f682
sha256sum "$(command -v mach)"          # 29b9264c... (Mach 4.30.0)

mach build . -o out/audit/mach                          # generation A
mach build . --profile release -o out/audit/mach-release
out/audit/mach build . -o out/audit/mach-b              # B
out/audit/mach-b build . -o out/audit/mach-c            # C
sha256sum out/audit/mach out/audit/mach-b out/audit/mach-c
cmp out/audit/mach-b out/audit/mach-c

out/audit/mach test . --jobs 8
out/audit/mach-release test . --jobs 8
sh test/census.sh
M=$PWD/out/audit/mach
MACH_CORPUS_MACH=$M MACH_CORPUS_OUT=$PWD/out/corpus-ab bash test/run.sh \
    --target x86_64-linux --target aarch64-linux --target riscv64-linux --target spirv \
    --target x86_64-windows --target x86_64-darwin --target aarch64-darwin \
    --target riscv32 --target mos6502 --layer a --layer b
MACH_CORPUS_MACH=$M MACH_CORPUS_OUT=$PWD/out/corpus-c bash test/run.sh \
    --target x86_64-linux --target riscv64-linux --layer c
MACH_LINK_MACH=$M bash test/link/run.sh --leg x86_64-linux
MACH_VECROWS_MACH=$M MACH_VECROWS_OUT=$PWD/out/vecrows bash test/vecrows/run.sh --target x86_64-linux
```

Do not run `mach dep pull`: it resets `dep/std` to the lock, and the pin above
is the lock's commit checked out by hand so that the clone is shared and
immutable. `mach build` skips `test` blocks; only `mach test` compiles them.

## Tests and cases added since the L8 candidate

The L8 tests (listed in the superseded record) are on the branch. The L9 lane
added the `editor.tag`, `editor.recovery`, `editor.deprecation`,
`driver:deprecated_*`, `driver:tag_*_edit_*` and query codec tests. This lane
adds no suite test; it adds the corpus `tag` group and `cmp/ret_chain_inline`
(`l10-acceptance-3218.md` section 3).

## Known limits carried forward

- A constant tag is comptime-visible only inside its own module (L8).
- A nested record payload's omitted fields read at compile time need the
  constant's checked type, which name resolution does not have (L8).
- Darwin has no execution lane on this host, so darwin cells are
  build-and-decode; the RV32 forms have no execution engine; windows cells are
  build-and-decode, and the L5 C control ran under wine.
- The spirv o2 structurizer refuses an inlined three-way early-return chain
  (#3275); `cmp/ret_chain_inline` carries it as a MACH DEFECT skip.
- The compiler still seeds canonical `res`/`opt`/`err`; S1 removes it.
