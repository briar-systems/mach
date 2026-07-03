# `dbg/` — debug-info verification harness

Machine-checks the debug info `mach build -g` emits, against the format's own
validators. The compiler is its own linker and ships no external tools, so
debug-info correctness is asserted in CI against committed fixtures using
test-only validators (`llvm-dwarfdump`, `addr2line`, and — as their producers
land — `llvm-pdbutil` / `lldb`). Those deps live here, never in `int/`, which
stays tool-free by design.

Every debug-info tier issue (the DWARF and CodeView epics under #1688) references
this lane in its acceptance: it exists early and is extended per producer.

## Layout

```
dbg/
  verify.sh          the harness: build each fixture ±-g, run the validators
  fixtures/
    multi/           a tiny multi-function program (mach.toml + src/main.mach)
  README.md
```

`verify.sh <compiler>` builds every `fixtures/*` case with and without `-g`
using the given compiler and runs the checks. `--filter <glob>` narrows to
matching fixture directories. It exits nonzero if any check fails. Per-fixture
build state (`dep/`, `out/`, `mach.lock`) is gitignored.

## What is live vs staged

**Live — DWARF on ELF.** The DWARF producer emits real
`.debug_abbrev`/`.debug_info`/`.debug_line` under `-g`, so the ELF lane runs
today. `llvm-dwarfdump`/`addr2line` read the target object regardless of the
host's ISA, so one linux runner validates every shipped ELF ISA
(`linux`, `linux-arm64`, `linux-riscv64`). Per target it asserts:

1. `llvm-dwarfdump --verify` accepts the whole image.
2. the fixture compile unit exists and its PC range lies inside `.text`.
3. every helper function is represented in the line table at its own `fun` line,
   and `addr2line` round-trips one such PC back to `main.mach:<line>`.
4. the line table references several distinct source lines (multi-function).
5. **additive-only:** every `PT_LOAD` segment is byte-identical between the `-g`
   and no-`-g` builds (modulo the ELF header's section-table bookkeeping). `-g`
   must not perturb one byte of the loaded program — it only appends non-loaded
   debug sections. This is the invariant that guards the whole program.

**Staged — pending their producers.** Wired as documented expansion points in
`verify.sh` and provisioned runners; each turns on when its producer lands:

- **Mach-O DWARF runtime** (`darwin-x86_64` / `darwin-aarch64`): the runtime
  checks from #1698's runtime-check comment — `codesign --verify` covering the
  `__DWARF` bytes, dyld load, and an `lldb` source-line breakpoint — need a macOS
  runner, which `int-main.yml` already provides on merge to `main`. The
  structural `llvm-dwarfdump --verify` already works host-side on a cross-built
  Mach-O and can be lifted into the ELF loop's shape when a macho fixture leg is
  added.
- **COFF / CodeView** (`windows`): needs the CodeView producer (#1595). `mach
  build -g` emits no debug sections for COFF today, so the lane is dormant and
  byte-identical off. Once it lands, `llvm-pdbutil` / `llvm-readobj --sections`
  assert the discardable `.debug$S`/`.debug$T` sections and the COFF long-name
  string table, on the existing `windows` runner in `ci.yml`.

## Adding a fixture

Drop a mach project under `fixtures/<name>/` (a `mach.toml` plus `src/`). The ELF
lane picks it up automatically; it must contain the `add`/`mul`/`square`
functions the line-table checks anchor on, or extend `verify.sh`'s anchor set. A
tier issue turns a staged lane live by adding its checks to the staged block in
`verify.sh` and its runner leg to the workflow.
