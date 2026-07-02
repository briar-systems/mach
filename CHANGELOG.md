# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.13.0] - 2026-07-02

A follow-up hardening, cleanup, and tooling sweep over the 2.12.0 overhaul.
`--pie` images regain read-only hardening for their relocated constants
(`PT_GNU_RELRO`), the register allocator sheds redundant copies for a smaller
binary, `mach test --format json`, `mach check --format json`, and `mach info
targets` add machine-readable surfaces, diagnostics gain `= help:` lines and
secondary spans, and out-of-range integer literals now report their bounds.
Contains breaking CLI, manifest, and source-acceptance changes - see Breaking.
Built with mach 2.12.0.

### Breaking

- sema: an integer literal that overflows a signed destination is a compile
  error; it previously compiled and silently wrapped (#1804).
- driver: unknown flags are hard errors on every command; `-h` / `--help` are
  not flags (use `mach help <command>`); the `--` separator is honored only by
  `mach run` (#1810).
- cli: the `--color` and `--artifacts` flags are removed (#1784, #1829).
- target: incoherent tuples are rejected at composition - an object format that
  does not cover the ISA, an ABI that does not target it, or an OS that cannot
  load the format (`windows`+`elf`, or `raw` on any real OS - flat images are
  `os = "freestanding"`) (#1806).
- build: `-o` cannot be combined with `--all-targets` (#1831).

### Added

- link/elf: **`PT_GNU_RELRO` under `--pie`** - relocated constant pointers
  (vtables, `val` pointer globals) that live in read-only data are writable while
  the static-PIE image self-relocates at startup, then `mprotect`'d read-only
  before `main`; a write through a relocated constant now faults. The linker
  emits the page-rounded header over the single read-only reloc-bearing segment,
  and the mach-std runtime re-protects it after applying its `RELATIVE`
  relocations (#1778).
- test: **`mach test --format json`** - a versioned, machine-readable NDJSON
  event stream (`run_start` / `test` / `summary`, plus `case` under `--list`;
  `schema: 1`) for editors and CI, documented in `doc/tooling/test-json.md`. JSON
  goes to stdout and build diagnostics to stderr; strings are `ensure_ascii`
  escaped so the stream is byte-identical across platforms; labels are emitted
  verbatim; `test` events arrive in **completion order** (sort by `index` for
  declaration order) (#1792).
- check: **`mach check --format json`** - a versioned, machine-readable NDJSON
  diagnostic stream (a `diagnostic` object per diagnostic carrying severity,
  message, primary location, note, help, and the LSP-shaped related list, then a
  closing `summary`; `schema: 1`) for editors and CI, documented in
  `doc/tooling/check-json.md`. JSON goes to stdout with no human frames
  interleaved; positions are 1-based and spans half-open; it reuses the
  `mach.cli.json` emitter, which grew nested-object and array support to carry
  the model (#1815).
- diag: diagnostics gained a distinct **`= help:` line** and an optional
  **secondary span** - a duplicate definition now points at the prior binding in
  its own `--> file:line:col` frame labelled `previous definition here`, and
  did-you-mean suggestions moved from `= note:` to `= help:`. Both are structured
  fields on the shared location model, ready for the coming
  `mach check --format json` consumer (#1783).
- driver: **`mach info targets`** prints the supported `<os>-<isa>` target
  matrix, derived from per-vtable capability declarations (each object format's
  ISA relocation coverage and each ABI's ISA) rather than a curated list, so a
  new target appears by declaring its capability (#1806).

### Changed

- codegen: **regalloc copy hygiene** - a dead-def sweep and copy-source
  allocation hints in the linear-scan allocator, backed by tighter live-range
  extension, eliminate redundant register moves. On mach's own release build the
  executable shrinks ~-4.0% and redundant register-to-register self-moves drop
  -88% (#1801).
- target: **incoherent target tuples now fail at composition** - `select_of`
  enforces the joint object-format/ABI capability at selection, so a tuple whose
  object format does not cover the ISA's relocations (e.g. `windows`+`aarch64`,
  COFF being x86_64-only) or whose ABI does not match the ISA is rejected with a
  message naming the missing capability, instead of composing and failing deep in
  codegen or link (#1806).
- driver: **`mach init` scaffold target names** follow the `<os>-<isa>`
  convention (`linux-x86_64`, `windows-x86_64`, `darwin-x86_64` on the host isa),
  matching the root manifest's normalized naming. A fresh project now writes
  `out/linux-x86_64/…` rather than `out/linux/…` (#1832).
- cli: **JSON emission moved to `std.data.json`** - the streaming NDJSON emitter
  that backed `mach test --format json` and `mach check --format json` (the
  `mach.cli.json` module) now lives in mach-std, unified with the tree emitter
  behind one escape core. `src/cli/json.mach` is deleted and both consumers
  (`testing.mach`, `diagnostic.mach`) import `std.data.json`; the mach.lock
  mach-std bump carries the moved code. Both JSON streams are byte-identical
  before and after (mach-std#338).

### Removed

- cli: the orphaned **`--color` flag** and its `ColorMode` surface, dead since
  the ASCII-only diagnostic renderer (#1777) dropped the color argument -
  `--color` had parsed but no-opped (#1784).
- build/run/test: the inert **`--artifacts <dir>` flag** and its `Config`
  field - it parsed but no code ever read it, so it silently no-opped. The
  object-tree location is expressed by the manifest `obj` template
  (`out/{target}/{profile}/obj`), which expands per target/profile/artifact; a
  flat CLI override cannot and would collide every target's objects into one
  directory (#1829).

### Fixed

- driver: **`--emit-ir` dumps the final post-pipeline IR** - the dump moved from
  the lower loop to the codegen loop, beside `--emit-asm`, so the `.ir` reflects
  the optimized module the objects are built from - it now varies with `-O` and
  picks up test-mode `main` neutralization, instead of the naive pre-pipeline
  lowering (#1802).
- sema: **out-of-range integer literals report their bounds** - a literal that
  does not fit its destination now reports
  `literal <v> is out of range for <T> (<min>..<max>)` instead of a generic type
  mismatch whose cast note advised the truncating `value::Type`. A literal that
  overflowed a signed slot (e.g. `val mask: i64 = 0xFFFFFFFFFFFFFFFF;`) used to
  compile and silently wrap at runtime; it is now rejected, while `u64` slots
  keep their legitimate maximum (#1804).
- link/elf: **the ELF exec writers refuse a program-header table that overflows
  its reserved header page** - the PIE and dynamic writers extend the first
  `PT_LOAD` down exactly one page to cover the ELF header + program headers, which
  only maps segment 0 at its vaddr while the header block fits that page
  (`phdr_count <= 72`). Past that the image was silently unloadable; the writers now
  return a link error instead. Latent (real builds emit ~9 headers), owning the
  invariant before segment counts grow (#1814).
- build: **`mach build --all-targets -o <path>` is now rejected** - `-o` names a
  single artifact path, so combined with `--all-targets` every target linked to
  that one path and overwrote the previous, leaving only the last target's binary
  with no warning. The combination now errors (`-o cannot be combined with
  --all-targets`) at parse; single-target `-o` is unchanged (#1831).
- driver: **unknown flags are rejected instead of silently misparsing** - an
  unrecognized flag was never reported and hijacked positional resolution
  (`mach build --color always .` resolved `always` as the project path). Each
  command now marks the flags it consumes and rejects the first unmarked
  `-`-prefixed token before resolving positionals - `error: unknown flag '<flag>'
  for '<command>'`, exit `1`. `-h` / `--help` are not flags; use `mach help
  <command>`. The `--` end-of-flags separator is honored only by `mach run`, which
  forwards every post-`--` token to the executed program as its `argv`; on every
  other command `--` is itself rejected as an unknown flag (#1810).

## [2.12.0] - 2026-07-01

The terminal output & test-harness overhaul: framed source-snippet diagnostics,
a per-phase build readout, and `mach test` rebuilt around a single dispatcher
binary with live, parallel, deterministic reporting (epic #1788 - on mach's own
438-test suite, `mach test .` drops 44.9s -> 7.2s wall and 9.4 GB -> one 7.3 MB
artifact). Static-PIE self-relocation is now active under `--pie`. Contains
breaking CLI changes - see Breaking. Built with mach 2.11.0.

### Breaking

- build/test: the `--verbose` flag is gone; use `-v` / `-vv` (#1775).
- test: `--runner <cmd>` now launches `<cmd> <exe> <idx>` - the runner receives
  the dispatcher path and a test index instead of a per-test executable (#1789).
- test: the `tests` manifest template's `{name}` resolves to the product name
  (one dispatcher binary) rather than a per-test name (#1789).

### Added

- diag: a framed source-snippet renderer for driver diagnostics - each
  reported error/warning shows its source line in a frame with a caret span,
  file:line:col header, and severity-tagged message, replacing the one-line
  flush (#1777).
- test: a per-module roll-up readout for `mach test` - all-passing modules
  collapse to one line, failures expand with the child's captured stdout,
  `file:line`, and exit code or signal, and the run ends with a summary that
  re-lists the failures; a crashing test reports its signal and the run
  continues (#1776). Extended to a live, parallel readout later in this
  cycle (see the `Changed` entries).
- link: static-PIE self-relocation is active - under `--pie` the mach-std
  runtime relocates the image's absolute pointers at startup, covered by an
  int exec guard (#1727).
- build: `-v` prints a per-phase readout (load / resolve / sema / lower /
  optimize / codegen / link) with item counts and timing, closed by a
  `built <path>  N modules  <size>  in <time>` summary; `-vv` adds a
  per-module/file line under each phase with its own duration and a `(slow)`
  marker on the slowest item. Fixed-width ASCII on stderr, identical across
  platforms - timing via `chrono.monotonic`, durations via
  `chrono.format_duration`, columns via the `{:<N}`/`{:N}` format spec (#1775).

### Changed

- test: `mach test` runs tests **in parallel** - a sliding window of `--jobs`
  child processes (default: the CPUs available to the process; `--jobs 1`
  serializes), reaped with a blocking wait-any. Each child's stdout *and
  stderr* are captured to a per-test file under `log/` beside the dispatcher;
  a passing test's file is removed on reap, a failing test's stays (the
  expanded failure shows the first 64KB, a `full output:` pointer when
  truncated, and the exact `rerun: <exe> <idx>` command). Results render in
  collection order regardless of completion order, so the readout is
  deterministic (#1791).
- test: the `mach test` readout is **live** - each module's roll-up prints the
  moment its last test completes, and failures expand as they happen. Column
  widths are computed from the collected tests (clamped) instead of hard-coded,
  test labels print verbatim as declared (the old `<module>.`-prefix stripping
  is gone), durations right-align, and the closing summary reports the run's
  wall time. `-v` prints each test's line as it completes; `-vv` now also
  prints passing tests' captured output (it was a silent alias of `-v`);
  `mach test --help` documents both (#1790).
- test: `mach test` builds **one dispatcher executable** covering every
  collected test instead of one standalone executable per test - one link
  instead of N (on mach itself: 44.9s → 7.2s wall, 9.4 GB → 7.3 MB on disk).
  Each test still runs as its own process, spawned as `<exe> <idx>`. `--runner`
  now receives the executable path and the test index as its two arguments;
  `--filter` selects at run time, so the built executable is identical
  regardless of filter; the `tests` template's `{name}` resolves to the product
  name (falling back to the project id) rather than a per-test name (#1789).

### Removed

- build: the `--verbose` flag, replaced by `-v`/`-vv` (#1775).

### Fixed

- ci(int): the darwin-x86_64 integration leg never ran - it queued forever on
  starved `macos-13` runners and died at GitHub's 24h cap; routed to `macos-14`
  under Rosetta 2, mirroring CD (#1787).

## [2.11.0] - 2026-06-30

Position independence and multi-arch ELF dynamic linking: opt-in static-PIE
executable emission for linux, PLT/GOT writers for aarch64 and riscv64, plus
editor groundwork and a loud-error sweep over the backend's silent fallbacks.
Built with mach 2.10.0.

### Added

- link/elf: opt-in **static-PIE** (`ET_DYN`) linux executables via `--pie` /
  `$mach.build.pie` - `PT_PHDR` load-bias recovery, a synthesized `.rela.dyn`
  of `R_*_RELATIVE` base relocations, and `PT_DYNAMIC`; runtime self-relocation
  activates in the next release (#1727).
- of/elf: aarch64 and riscv64 **PLT/GOT dynamic-link writers** with byte-level
  encoding guards, completing dynamic linking across the shipped linux ISAs
  (#1741).
- editor: sema retains per-expression types on `SemaResult` for editor/LSP
  consumption (#1501).
- ci: an x86_64-darwin release lane via Rosetta 2 on `macos-14` - Intel
  runners are queue-starved (#1728); darwin integration legs run on merge to
  main rather than a schedule (#1765).
- int: structural (field) and freestanding (flat-loader) producers widen what
  the integration harness can assert (#1760).

### Fixed

- be: silent backend fallbacks now fail loudly, with the AAPCS64 edge rules
  documented where they were being papered over (#1745).

## [2.10.0] - 2026-06-29

Native arm64 macOS. mach now compiles, ad-hoc code-signs, and self-hosts
position-independent arm64 Darwin executables that run on Apple Silicon - the
first published Darwin release binary. Also a format-neutral base-relocation
foundation for position independence, project-scoped `mach test`, `mach run`
without a rebuild, a riscv64 ELF build-attributes section, and a parser
correction. No breaking changes.

### Added

- target: native **arm64 Darwin (Apple Silicon)**. mach emits position-independent
  Mach-O executables (`MH_PIE`, `LC_MAIN`, an `LC_DYLD_INFO` rebase stream, and an
  ad-hoc `CS_LINKER_SIGNED` code signature) that exec and self-host on Apple
  Silicon; CD publishes an `aarch64-darwin` archive (#1679, #1717, #1722). x86_64
  Darwin stays cross-compile-only (#1728).
- link: a format-neutral image base-relocation set - the linker collects every
  in-image absolute pointer once and each object format encodes it, the foundation
  for PIE/ASLR across targets (#1722).
- arm64: `cset` in the inline-asm assembler (#1714).
- target: a riscv64 `.riscv.attributes` ISA-string ELF section (#1673).
- run: `mach run` executes the already-built artifact without recompiling (#1482).
- test: `mach test` collects only the current project's tests by default; pass
  `--include-deps` to widen (#1556).

### Fixed

- macho: coalesce object sections by kind so modules with more than 255 sections
  link (Mach-O's `n_sect` is a single byte) (#1682).
- macho: map the mach header into `__TEXT` so the Apple Silicon kernel admits the
  signed image (#1717).
- parser: require a block body for `fin`; a bare `fin stmt;` was never intended to
  parse (#1548).

### Changed

- deps: mach-std v0.16.2 - the arm64 Darwin runtime (`LC_MAIN` entry, syscall layer).

## [2.9.0] - 2026-06-27

Darwin executables, the constant-time secret qualifier in the type checker, and
the riscv64 backend hardened against real code. macOS now has working dynamic
executables, completing both Darwin triples, and a sweep driven by compiling and
running real std under qemu closed every riscv64 codegen gap real programs hit.
No breaking changes.

### Added

- target: a macOS / Darwin OS substrate plus Mach-O dynamic executables (an
  `emit_dyn_exec` that writes `LC_LOAD_DYLINKER`, `LC_LOAD_DYLIB`, an
  `LC_DYLD_INFO_ONLY` bind stream, `LC_SYMTAB`/`LC_DYSYMTAB`, `LC_UNIXTHREAD`,
  and per-arch import stubs), completing the `x86_64-darwin` and `aarch64-darwin`
  cross-compile triples. Byte-verified static and dynamic for both (#1178).
- target: riscv64 RV64A atomic inline-asm mnemonics, `lr`/`sc`, the `amo*`
  family, the `.aq`/`.rl`/`.aqrl` ordering suffixes, and `pause`, so atomic
  read-modify-write can be expressed for the riscv64 std atomics and runtime
  (#1668).
- sema: constant-time secret-qualifier flow typing. The `^` qualifier carries a
  public-to-secret lattice (public coerces up, any secret operand taints the
  result), the type checker gates what a leakage model can observe (no secret
  branch condition, memory index, or variable-latency operand), `:^` is the only
  downgrade, and secret pointers are welded and non-launderable. Type-checking
  only, with codegen taint a later step (#1645, epic #1643).

### Fixed

- codegen(riscv64): long-branch relaxation. An out-of-range B-type conditional
  becomes an inverted guard plus a `jal`, and an out-of-range `jal` becomes an
  `auipc`+`jalr` trampoline, resolved by a per-function relaxation fixpoint.
  Large functions no longer fail to encode (#1666).
- codegen(riscv64): local frame slots no longer overlap the saved ra/s0 record,
  fixing a silent SIGSEGV for any function with an address-taken or spilled local
  (#1670).
- codegen(riscv64): 32-bit `and`/`or`/`xor` now encode full-register ops instead
  of nonexistent word-group instructions, fixing a SIGILL (#1672).

## [2.8.0] - 2026-06-26

The first working bare-metal build, plus a riscv64 codegen fix surfaced by the
new backend. The freestanding `raw` object format now has a real flat-image
build path, so an `os=freestanding, of=raw` target builds end to end.

### Added

- build: a flat-image build path for image-producing object formats. An object
  format declares through a `produces_image` predicate whether it round-trips
  through relocatable `.o` files, and an image format (the freestanding `raw`
  writer) now links straight from the in-memory codegen output, bypassing the
  per-module `.o` emit/parse round-trip. This is the last gap to a real
  `os=freestanding, of=raw` build (#1616).

### Fixed

- codegen: a riscv64 variable-amount shift with a constant value operand
  (`1 << node`) used as a call argument now materializes the constant into a
  register before the shift instead of failing to encode. The shift was the lone
  reg-reg ALU encoder not materializing an immediate operand (#1657).
- diag: the unknown-os diagnostic now lists `freestanding` among the expected
  values (#1617).

## [2.7.0] - 2026-06-26

First targets on the retargeting foundation. riscv64 lands end to end as the
register-machine validator (substrate authoring plus vtable hooks, no
shared-backend edits), Mach-O joins ELF and COFF as an object format, and the
constant-time work begins with the secret type qualifier in the front end.
Releases now ship optimized (opt=2) binaries. No breaking changes, and existing
x86_64 and aarch64 builds are unaffected.

### Added

- target: a full riscv64 (RV64) isa backend (machine model, instruction
  selection, encoder, relocations) and an lp64/lp64d ABI, composing the
  `riscv64-linux` cross-compile target. Byte-verified against llvm-mc and run to
  its exit code under qemu (#1172).
- target: a riscv64 inline-asm assembler so `asm riscv64 { ... }` blocks encode
  to RV64 bytes, including `ecall`. The cross lane now runs a qemu exit-code e2e
  (#1642).
- target: a Mach-O object format (writer, reader, static executable) for x86_64
  and arm64, keyed on the isa with no new shared backend hook. Cross-compile
  byte-verified for the darwin triples (#1176).
- frontend: the secret type qualifier `^` (`^u32`, `*^u8`, `[N]^u8`, `^MyRec`)
  and the `:^` strip-cast token. Lexer, parser, and grammar only, the
  de-risking first slice of the constant-time guarantees epic (#1643); no
  type-checking behavior yet (#1644).
- build: `[profile.debug]` and `[profile.release]` profiles with
  profile-segmented output paths, so debug and release builds no longer clobber
  each other. Releases build with `--profile release` (opt=2) gated by a
  self-host fixpoint, and a release-profile CI lane catches -O2 regressions on
  PRs (#1591, #1592).

### Changed

- link: the relocation and ELF-class seam is widened onto the substrate, so a
  new isa owns its reloc packing and object-format mapping rather than editing
  shared linker and ELF code (#1625, #1635).

### Fixed

- test: `mach test` bounds per-test link memory with a per-test scratch arena
  reset after every test. Memory was O(test count) through the session arena and
  tripped the windows runner ceiling at `coff: unwind table alloc failed` once
  the suite grew (#1653).
- codegen: riscv64 sub-word loads read the source width rather than the clamped
  width, fixing a sub-word load miscompile (#1639).
- codegen: `fp_class_index` finds the float bank by RegClass kind instead of
  assuming a >=128-bit float bank (#1624).
- docs: `doc/language/files.md` no longer misframes `main()` as compiler-known
  (#1631).

## [2.6.0] - 2026-06-26

Retargeting foundation. A compilation target is now a composition of named
substrates (isa, abi, os, object-format) carrying a by-value machine model, so a
new register-machine target plugs in as substrate modules plus vtable hooks with
no shared-backend edits. Adds the first bare-metal capability: a freestanding os
and a raw flat-image object format. No syntax changes, and existing x86_64 and
aarch64 builds are unaffected.

### Added

- target: a `freestanding` os substrate (no syscalls, no OS runtime, custom
  `_start`, image base 0) and a `raw` flat-image object-format writer that lays
  each segment at its virtual address and emits the bytes with no container,
  entered at the image base (#1613).
- target: object-format selection by name. An optional `of` key on a
  `[target.<name>]` table picks the object format explicitly (e.g. `of = "raw"`);
  omitting it derives the format from the os default, as before (#1615).
- target: a `MachineModel` record carrying the machine description (widths,
  register file and classes, addressing modes, frame model) as data, read by the
  shared backend stages (#1606).

### Changed

- target: targets compose by name through name-keyed substrate registries. The
  legacy os/arch/abi/of ids are derived from the resolved substrates rather than
  authored, and the central `canonical_*` switches are gone (#1609).
- codegen: lowering, register allocation, and frame layout read the widths, the
  register file, and the index/address width from the machine model instead of
  shared constants (#1603, #1607, #1608). Behavior-preserving for the existing
  targets.
- link: each isa owns its relocation packing through an `apply_reloc` vtable hook
  and declares its own ELF `e_machine`, replacing the `arch_id` switches in the
  linker (#1610, #1612).
- target: removed the dead `syscall_layer` os field and consolidated
  primitive-type classification behind a `prim_desc` table (#1604, #1601).

## [2.5.9] - 2026-06-24

Fixes a latent miscompile: a runtime array/pointer index narrower than the
machine word (e.g. a `u8`) was passed to the GEP at its source width, so the
back end scaled an index register carrying undefined high bits and computed a
wild address.

### Fixed

- lower: `lower_index_lvalue` widens a sub-word runtime GEP index to the
  machine-word index type (zext unsigned / sext signed) before `emit_gep`, so
  the back end scales a clean register. In-tree index sites cast (`::u32` /
  `::usize`) and masked it; an uncast sub-word index miscompiled (#1596). The
  64-bit widen lives in the target-agnostic lowering for now; #1598 tracks
  moving index normalization into the target-aware codegen.

## [2.5.8] - 2026-06-24

Incremental type-checking (#1164), completing the incremental front-end: parse,
name resolution, and now sema all run as memoized queries, so an edit re-checks
only the changed module plus dependents whose typed public surface moved.

### Added

- sema: `Q_SEMA` (owned-handle query keyed by StableModuleId) with a
  `Q_TYPED_EXPORTS` fingerprint firewall — an impl-only edit firewalls type
  dependents while a public-type change re-checks them. `Q_MODULE_NUMBER` gates
  re-check on a graph reshape that renumbers a module's nominal TypeIds.

### Fixed

- type: field tables (keyed by TypeId) are rebuilt per build via a field-table
  epoch, so a record's field-type change on a long-lived session no longer
  leaves a stale layout (latent; fresh-session builds were unaffected).

## [2.5.7] - 2026-06-23

Incremental name resolution (#1164). The front-end is now query-engine-driven
through resolve: an edit re-resolves only the changed module plus importers
whose public surface actually moved, instead of the whole closure.

### Added

- query: parsing and name resolution run as memoized queries (`Q_PARSE`,
  `Q_RESOLVE`) with owned pointer-graph handles; a `Q_EXPORTS` fingerprint of
  each module's resolved public surface is the early-cutoff firewall, so a
  private-body edit does not re-resolve importers while a public-signature or
  public comptime-const-value change does. Build-stable `StableModuleId`s key
  the cached results across rebuilds.

### Fixed

- parser: a function's `DeclId` is reserved before its body is parsed, so a body
  edit no longer shifts the function's id (which silently staled importers
  indexing it).
- driver: each module's `Q_EXPORTS` is refreshed in topological order during the
  resolve pass, so an importer is never validated against a stale dependency
  fingerprint.

## [2.5.6] - 2026-06-23

Incremental parse caching (first increment of #1164): unchanged modules are no
longer re-parsed across builds, so a one-file edit's rebuild re-lexes/re-parses
only the changed file instead of the whole reachable closure.

### Changed

- driver: parsed ASTs are cached in a session-owned `AstCache` keyed by
  `(FileId, source revision)` and reused across builds; `parse_module` skips
  tokenize+parse on a cache hit. `ModuleEntry` now borrows its AST from the
  cache (the cache is the sole owner; `dnit_project` no longer frees it), so the
  cache survives a project teardown and rebuild.
- source: `update` no longer bumps a file's revision when the new text is
  byte-identical to the old, so an unchanged file keeps its revision (and stays
  a cache hit) across rebuilds.

## [2.5.5] - 2026-06-23

`mach doc` now reads first-class documentation, so the compiler, the docstring
lint, and editor hover all share one doc source.

### Changed

- doc: `mach doc` sources documentation from each decl's `Decl.doc` span via
  `mach.lang.fe.doc` (walking the parsed AST, recursing into comptime branches)
  instead of re-scanning raw source text. `#[attr]` decorator lines no longer
  leak into rendered docs, and component whitespace is normalized. Content after
  `# ---` that is not a valid `name: desc` component is dropped, matching the
  lint and hover.

## [2.5.4] - 2026-06-23

First-class documentation. Doc-comment blocks are now captured on declarations
and validated by a docstring lint, laying the groundwork for editor hover and
`mach doc` to share one source of truth.

### Added

- fe: capture each declaration's doc-comment block as a span on `Decl`
  (attribute-aware — `#[...]` lines between the docstring and the decl are
  skipped), plus `fe/doc.mach`, an allocation-free doc-block parser exposing the
  summary and component lines with their name/description spans.
- fe: a `pub`-scoped docstring lint (`fe/doclint.mach`) that warns when a
  documented component names no real parameter/field/generic/`ret`
  (misspelled), is out of declaration order (judged over the documented subset,
  so partial docs are fine), or has an empty description. It does NOT require
  completeness — undocumented items are never flagged.

### Fixed

- fe: the doc-block parser no longer mistakes a wrapped prose continuation line
  beginning `word:` for a component head (continuation lines are indented past
  the `# name` column).

## [2.5.3] - 2026-06-23

Front-end performance and visibility. Bumps mach-std to 0.14.1, whose
linear-time `str_region_equals` removes the quadratic-parse stall that froze the
front-end of stdlib-heavy builds.

### Added

- build: `--verbose` now streams per-module `lex`/`parse`/`resolve` lines during
  the front-end, alongside the existing `sema`/`lower`/`codegen` stages (#1567).

### Changed

- resolve: hash-index the module scope for O(1) name lookups instead of a linear
  scan (#1565).
- driver: hoist the resolve dep-closure scratch out of the per-module loop,
  removing O(modules²) allocation during the resolve pass (#1565).
- deps: bump mach-std to 0.14.1 (linear `str_region_equals`).

## [2.5.2] - 2026-06-22

Patch — const-string global references now fold correctly.

### Fixed

- lower: a `str`/pointer-typed global initialized to a reference to a const-string
  `val` (`pub val B: str = A;`) emitted the string as raw inline bytes in the
  pointer slot instead of a pointer-to-rodata relocation. The global-init folder
  now routes any initializer that comptime-folds to a string through the rodata +
  relocation path, generalizing the v2.5.1 comptime-path fix (#1559).

## [2.5.1] - 2026-06-22

Patch — global initializers now fold comptime intrinsic paths (#1557).

### Fixed

- lower: a module-level `val`/`var` initialized to a comptime intrinsic path
  (`$project.version`, `$mach.*`) that folds to a string is now lowered to a
  pointer-to-rodata relocation instead of being mis-emitted as raw inline bytes,
  so a global like `pub val X: str = $project.version;` folds correctly. The
  global-init aggregate folder routes string-valued comptime paths through
  `comptime.eval`, mirroring expression-position folding (#1557).

## [2.5.0] - 2026-06-20

Minor — a breaking CLI change to project-root resolution (#1545) and a codegen
fix for field access on struct-typed `$each` pack elements (#1549). The project
commands now resolve the project root one way only: `build`, `run`, `test`, and
`doc` each take the project root as a required positional (`mach build .`); a
bare invocation with no path is a user error, collapsing the three redundant
resolution paths (bare-cwd default, positional, and `--cwd`) that had drifted
in.

### Changed

- cli: `build`, `run`, `test`, and `doc` now **require** an explicit project-path
  positional (`mach build <path>`). A bare invocation prints
  `error: missing project path; pass the project root (e.g. '.')` and exits `1`.
  `doc` previously took no positional; it now requires one like the others
  (#1545).

### Removed

- cli: the `--cwd <path>` flag. Pass the project root as the positional instead
  (`mach build .`) (#1545).

### Fixed

- lower: field access (`a.x`) or address-of (`?a`) on a struct-typed `$each` pack
  element fabricated an extern global named after the loop variable, failing at
  link with `undefined symbol`. `lower_ident_lvalue` now resolves a pack element
  to its expanded-parameter storage slot, mirroring the rvalue path (#1549).

## [2.4.1] - 2026-06-20

Patch — windows git dependency resolution (#1538) and a multi-target union-build
crash (#1540). The windows git path failed in two distinct ways, both now covered
by the `Windows (native)` CI lane, which resolves the vendored std with
`mach dep pull` instead of a manual clone.

### Fixed

- dep: `resolve_cmd` searched `PATH` with POSIX separators (`:` list, `/` path),
  so it never resolved an executable on windows — `PATH` is `;`-separated there
  and drive-letter prefixes (`C:\…`) embed a `:` that shattered every entry. it
  now selects the list/path separators by target os, so `git.exe` is found on
  windows as it is on linux. also fixes runner resolution in `mach run`/`mach test`
  (#1538).
- dep: git was spawned on windows with a reconstructed allowlist environment that
  omitted `SystemRoot`, so its winsock initialization failed with
  `getaddrinfo() thread failed to start` once git was found. the allowlist exists
  only for posix `execve` (which does not inherit and exposes no `environ`
  handle); on windows `CreateProcess` inherits the full parent environment
  natively, so git is now spawned with a nil child env there (#1538).
- driver: `walk_comptime_if_union` cached a `*ModuleEntry` (and AST-arena pointers
  into it) across a recursive load that can `reallocate` the `p.modules` array,
  so multi-target union builds spanning ≥3 OS/arch tuples dereferenced freed
  memory and crashed (SIGSEGV in tooling/LSP). the taken branches are now
  snapshotted before recursing, mirroring `walk_use_for_load` (#1540).

## [2.4.0] - 2026-06-19

Minor — Phase 2 (collapse) of the `#[attr]` decorator migration (#1526): `#[attr]` is now
the **only** decorator syntax; the backtick form is removed. Treated as pre-stable churn
rather than a breaking major — the backtick form shipped days ago (2.0.0) with ~0 external
adoption, and the change is purely surface-level (same Decorator AST, same codegen).
Byte-reproducible from the v2.3.0 seed; the vendored-std advance to mach-std 0.14.0 is
inert.

### Removed

- syntax: backtick decorators (`` `symbol(...)` ``, `` `library(...)` ``, `` `inline` ``,
  `` `align(N)` ``, `` `section(...)` ``). Use the `#[attr]` form (`#[symbol("...")]`,
  `#[inline]`, …) added in v2.3.0. A backtick at decorator position now reports a
  migration diagnostic — `backtick decorators were removed in v2.4.0; use #[name(...)]` —
  and recovers at the next declaration (#1535).

### Changed

- deps: the vendored mach-std advances to **v0.14.0** — the `#[attr]`-migrated standard
  library.

## [2.3.0] - 2026-06-19

Minor — Phase 1 of the `#[attr]` decorator migration (#1526): the parser now accepts
Rust-style `#[attr]` decorators **alongside** the existing backtick decorators. Purely
additive and backward-compatible — both surfaces produce the same declaration AST, and
the compiler's own source is unchanged (still backticks), so it stays byte-reproducible
from the v2.2.0 seed. The v2.4.0 collapse migrates all source to `#[attr]` and removes
the backtick form.

### Added

- syntax: `#[symbol("...")]`, `#[library("...")]`, `#[align(N)]`, `#[section("...")]`,
  `#[inline]` decorator forms. The lexer opens an attribute when `#` is *immediately*
  followed by `[` (otherwise `#` stays a line comment — a literal comment beginning
  `#[` must insert a space). `#[...]` and the backtick form attach identically; the
  decorator AST and sema/resolve/validate are unchanged (#1532).

## [2.2.0] - 2026-06-19

Minor — correctness and cross-arch coverage, hardening the compiler ahead of a manual
audit. This release changes how the compiler emits float comparisons (#1446), so it
**re-seeds** the self-host baseline: it is intentionally *not* byte-reproducible from the
2.1.0 seed (stage1≠stage2 by design) but converges to a byte-identical fixpoint
(stage2==stage3) on both x86_64 and aarch64.

### Added

- target/aarch64: AAPCS64 **HFA/HVA** classification — a homogeneous floating-point
  aggregate (1–4 members of the same FP type, counted recursively, including >16 bytes)
  is passed and returned in consecutive SIMD/FP registers (V0–V7), with all-or-memory
  spill when the run doesn't fit. Completes the aarch64 by-value float-aggregate ABI;
  x86_64 SysV and win64 emission are unchanged (#1174).

### Fixed

- codegen/x86_64: floating-point comparisons now follow **IEEE-754** for NaN, matching
  aarch64 — an unordered (NaN) operand yields false for `<`, `<=`, `>`, `>=`, `==` and
  true for `!=` (`<`/`<=` via operand-reversed `SETA`/`SETAE`; `==` as `SETE ∧ SETNP`;
  `!=` as `SETNE ∨ SETP`). Cross-arch NaN comparisons now agree (#1446).
- comptime: `$each f in $fields(T)` over a **generic** type parameter no longer errors at
  template sema — it defers to monomorphization (mirroring variadic packs), so reusable
  generic derive helpers (`debug[T]`, `equals[T]`, …) compile. A non-record instantiation
  is diagnosed at instantiation (#1523).
- ci/aarch64: the `Test corpus under qemu-aarch64` step is un-gated — `ut_manifest`
  gained the `[target.linux-arm64]` stanza so `target = "native"` resolves to the aarch64
  host under qemu (#1391) — and the Integration lane installs `qemu-user-static`, so the
  aarch64 integration suites (HFA register placement, `aarch64run`) **execute** under
  emulation in CI instead of skipping (#1464).

## [2.1.0] - 2026-06-19

Minor — comptime type-directed-dispatch ergonomics and multi-artifact tooling
graphs, surfaced while building the mach-std `{}` formatter and loading
multi-binary projects in mach-lsp.

### Added

- driver: the long-lived tooling union (`build_project_union`) now unions the
  import closure over **every declared artifact** (the full target × artifact
  matrix), not just one — a multi-artifact project (`[bin.*]` + `[lib.*]`) loads
  its whole module graph in tooling/LSP instead of erroring on artifact
  ambiguity. Single-artifact builds and the `mach build`/`mach run` ambiguity
  guard are unchanged (#1505).
- comptime: `$error("msg")` is now a real compile-time directive — it fails the
  build with its message when reached on a live path (an unconditional position
  or a selected `$if`/`$or` arm; a dead arm's `$error` never fires) and is valid
  in both declaration and statement scope. It was previously parsed as a silent
  no-op (#1511).

### Changed

- comptime/sema: a `$type_of`-comparison `$if`/`$or` chain now type-checks only
  the **selected** arm, pruning provably-dead arms at monomorphization.
  Type-directed dispatch over a concrete type no longer needs per-arm identity
  casts and is statically total (#1511).

## [2.0.1] - 2026-06-19

Patch — parser error-recovery and a grammar-doc fix surfaced while migrating the
mach ecosystem (blit, mach-glfw, mach-sieve, …) to 2.0.0.

### Fixed

- parser: a removed-syntax error at declaration scope — a comptime attribute
  setter (`$sym.attr = value;`) or a C-style `...` variadic parameter — now
  recovers at the **next** declaration instead of skipping it and reparsing its
  body at module scope, which sprayed a spurious "expected a declaration" for
  every non-`val` body statement. `sync_to_decl` no longer advances past a token
  that already begins a declaration; a regression suite (`tests/recovery`)
  guards both cases.

### Documentation

- grammar.md: removed the stale C-style `...` variadic **parameter** production
  (rejected since 2.0.0) — the comptime variadic pack `name: ...` is the only
  declaration form; function **pointer types** still carry `...` for FFI (#1518).

## [2.0.0] - 2026-06-19

**BREAKING.** The comptime collapse completes the v1.7 metaprogramming arc and is
the first new-only, post-migration release — hence the major version bump from
the 1.7.0 transition, which kept the legacy surfaces resolving for seed
compatibility. Code that still uses `$mach.target.*`, the top-level `$target.*`
root, `$<sym>.*` attribute setters, or C-style varargs must migrate (see Removed).
The self-host fixpoint holds — stage1==stage2==stage3 byte-identical on x86_64 and
aarch64 (the compiler binary shrinks as the legacy paths are deleted, but it
reproduces itself exactly).

### Removed

- comptime: the legacy `$mach.target.*` and top-level `$target.*` namespace
  spellings — build facts read through `$mach.build.*` and the declared target
  tuple through `$project.target.*` (#1480).
- comptime: the C-style varargs feature — the trailing `...` parameter marker,
  the `variadic` signature flag, `va_arg`/`va_start`/`va_end`, the `va_list`
  type, the OP_VA_* IR opcodes, and the per-ABI register-save callee prologue
  (superseded by comptime variadic packs in v1.7.0) (#1478).
- comptime: the `$<sym>.symbol` / `$<sym>.library` / `$main.symbol` attribute
  setters — superseded by `` `symbol(...)` `` / `` `library(...)` `` backtick
  decorators (#1476); a stray `=` after a comptime directive now reports a
  migration diagnostic (#1478).
- ci: the content-based seed-safety guard (#1486) — obsolete now that the seed is
  v1.7.0 and the vendored std legitimately uses `va:` / `$each` / `$mach.build.*`.
- abi: the vestigial VaModel register-save geometry left behind by the varargs
  removal — the 12 save-area fields, the per-ABI `va_save` helpers, and the
  never-emitted `MIR_VA_FP_SAVE` pseudo. The functional call-ABI facts (the SysV
  vector-count register and the win64 float-duplication) are retained (#1478).

### Changed

- deps: the vendored mach-std pin advances to v0.12.0 — the migrated, new-only
  std (comptime packs + `$mach.build.*`) (#1480, #1478).
- comptime: float-literal folding imports `std.math.bignum` from the vendored std
  instead of the temporary in-tree bignum port added for v1.7.0 seeding (#1483).

## [1.7.0] - 2026-06-19

Comptime rework and first-class variadics — the last big breaking change before
stability. The C-style `va_list` surface is gone, replaced by comptime variadic
packs that monomorphize per call; the `$` comptime channel gains first-class type
values; and per-declaration attribute directives move onto backtick decorators.
The compiler source uses no new syntax, so the self-host fixpoint converges
byte-identical on x86_64 and aarch64 — except that float-literal folding is now
correctly rounded, which intentionally changes the compiler's own float constants
relative to the v1.6.0 seed (this release re-seeds).

### Added

- comptime: first-class type values — `$type_of`, `$fields(T)`, field projection
  `v.[f]`, bounded `$each` expansion, and type `==` in `$if` (#1472, #1473).
- variadics: comptime variadic packs — a `va: ...` pack parameter consumed by
  `$each arg in va`, monomorphized per call-site type-list (no `any`, no runtime
  `va_list`); `va...` forward, `va.len` (#1474, #1475). aarch64 variadics work.
- decorators: per-declaration backtick decorators `` `symbol` ``/`` `library` ``/
  `` `inline` ``/`` `align` ``/`` `section` `` (codegen-only; `align`/`section`
  emit, including per-function section placement) (#1476).
- comptime: provenance-rooted namespaces — resolved `$mach.build.*`, fixed
  `$mach.{os,arch,abi,mode}.*` tag tables, and `$project.*` from the manifest
  (#1471). The legacy `$mach.target.*` spellings still resolve this release.
- comptime: correctly-rounded float-literal folding via exact bignum — large
  exponents no longer lose precision (e.g. `1.0e100`) (#1483).
- ci: a self-host fixpoint lane that byte-diffs seed→stage1 vs stage1→stage2 on
  PRs, catching uniform codegen changes the release `cmp` can't see (#1488).

### Changed

- deps: the vendored mach-std seed freeze is now single-source — `mach.lock` +
  `mach dep pull` with an immutable `commit/` pin, the redundant submodule
  removed, and a content-based seed-safety guard (#1486).

### Fixed

- dep: `mach dep pull` now finds `git.exe` on Windows (the lookup missed the
  `.exe` extension) (#1506).

## [1.6.0] - 2026-06-14

The aarch64-linux debut: mach now cross-compiles and self-hosts on 64-bit ARM
linux (AAPCS64) alongside x86_64 linux and windows. The compiler itself makes no
variadic calls, so the self-host fixpoint converges byte-identical on aarch64
under qemu exactly as it does natively on x86_64.

### Added

- target/aarch64: a complete aarch64-linux back end (#1431). The A64 fixed-width
  encoder emits 32-bit instruction words directly; instruction selection covers
  the scalar integer and floating-point paths; integer division and remainder
  lower to `sdiv`/`udiv` + `msub` with the defined divide-by-zero and
  signed-overflow trap semantics; and variable (runtime-count) shifts lower to
  the register-operand shift forms.
- target/aarch64: the AAPCS64 calling convention — argument and result
  classification across the general-purpose (x0–x7) and SIMD/FP (v0–v7) register
  banks, the x8 indirect-result register for large/`sret` returns, and
  natural-alignment of stack-passed arguments.
- target/aarch64: calls, relocations, and frames — `bl` calls and the
  `adrp`+`add`/`ldr` symbol-address idiom emitting the R_AARCH64_CALL26,
  ADR_PREL_PG_HI21, ADD_ABS_LO12_NC, and LDST\*_ABS_LO12_NC relocation kinds,
  with the leaf and non-leaf frame prologue/epilogue (`stp`/`ldp` of x29/x30).
- target/aarch64: inline-asm mnemonic support for the A64 instruction set, so the
  mach-std aarch64 runtime entry (`_start`) and OS layer assemble through the same
  per-architecture asm path as the x86_64 runtime.
- ci/release: the aarch64-linux release asset is enabled (`RELEASE_AARCH64`), with
  a qemu-aarch64 smoke-test that proves the cross-built aarch64 `mach` can both
  build and run a real project under emulation before it ships — mirroring the
  windows/wine compiler smoke. CI's aarch64 lane cross-builds mach to aarch64 and
  byte-verifies the emitted encodings; the runtime is exercised by that qemu smoke,
  the aarch64 self-host fixpoint, and the in-source test corpus run under qemu-aarch64
  (#1464).

### Fixed

- target/aarch64: f64 and f32 memory loads and stores selected the wrong register
  class — addressing a general-purpose register where a SIMD/FP register was
  required — so a float loaded from or stored to memory (a struct field, array
  element, or pointer dereference) now uses the SIMD `ldr`/`str` forms (#1459).
- parser: the bare statement keywords `cnt` / `brk` are disambiguated from
  identifiers via lookahead — they are keywords only in the bare `cnt;` / `brk;`
  statement form, so `cnt = expr;` now parses as an assignment instead of failing
  with the misleading "expected ';' after 'cnt'" (#1458). This was a prerequisite
  for aarch64: the aarch64 std runtime's inline asm uses the `brk` mnemonic
  (`asm aarch64 { brk 0 }`), which the pre-fix parser mis-parsed as the keyword.

### Known limitations

- aarch64-linux v1.6.0 ships without variadic formatting (`printf`/`format`/
  `vformat`): mach-std gates the variadic surface out of the aarch64 build, with
  the redesign deferred to v1.6.x as a cross-platform varargs effort. The
  compiler makes no variadic calls, so self-host is unaffected; only aarch64
  programs that call the variadic formatting helpers are impacted.

## [1.5.5] - 2026-06-14

Tooling-prep patch. Adds reload-friendly source handling so a long-lived session
(e.g. the language server) can rebuild its project graph on every save without
growing without bound, and completes the target-layer interner-elimination by
dropping the now-dead interner parameter from `register_all` and every target
registrar — the target vtables are now immutable singletons.

### Added

- driver/source: reload-friendly source handling for long-lived sessions. The
  session `SourceMap` now dedups by path (`source.load`): re-loading a path returns
  its existing `FileId` and swaps the text in place rather than appending, so a
  session that reloads its project graph on every save no longer grows without
  bound. `dnit_project` now also resets the session's per-build module registries
  (AST/sema/resolve/comptime/fqn and export/import maps), dropping the borrowed
  references the freed Project leaves behind so one Session is reusable across
  rebuilds. This is path-dedup + reclamation only; reusing untouched ASTs/resolve
  results across a rebuild (true incremental rebuild) is tracked separately in
  #1164 (#1389).

### Changed

- target: `register_all` and the target registrars no longer take an interner (or
  the vestigial allocator) parameter. The registrars set immutable `str` vtable
  fields and intern nothing, so the parameter had been dead since the OS-vtable
  interner-elimination — each registrar is now trimmed to exactly the registry it
  installs into, and `mach info` no longer builds a throwaway interner to call them.
  Behavior-preserving, verified by the byte-identical self-host fixpoint; completes
  the interner-elimination begun in #1402 (#1418).

## [1.5.4] - 2026-06-14

Stabilization + multi-target-prep release. Adds the multi-target union Project for
long-lived tooling (the language server now sees modules reachable only on a
non-default target), lands the post-windows compiler-stability audit fixes
(object-format parse-leak reclamation + truthful ELF symbol counts, the COFF REL32
addend convention, a distinct unknown-target-name diagnostic, and an arm64
register-id correction), and completes the OS-vtable interner-elimination — readying
the target layer for the v1.6 architectures.

### Added

- driver: `build_project_union` builds the union of every manifest target's import
  closure into one deduplicated Project, so long-lived multi-target tooling (e.g. the
  language server) sees modules reachable only on a non-default target. Each `$if`
  chain follows the first active branch of every declared target; the Project resolves
  under the default (native-resolved) target's tuple — the documented v1 default, "the
  default target's branches win" — and a module that does not fully resolve there
  records diagnostics without aborting the build (#1391).

### Changed

- target: the os/arch name↔id mapping is consolidated to one canonical table per
  dimension — `os.os_id_for`/`os.os_name_for` and `isa.arch_id_for`/`isa.arch_name_for`
  — replacing the copies that lived in `driver.mach` and `manifest.mach`. An
  unrecognized manifest `os`/`isa` name now reports a distinct "unknown target os/isa
  name '<name>' (expected: ...)" diagnostic instead of folding to `*_UNKNOWN` and being
  mis-reported as an unsupported pair (#1412).
- The `OsVTable` `entry_symbol`, `syscall_layer`, `libdir`, and `dynamic_linker`
  fields are immutable `str` constants set directly by the OS registrars, no longer
  `StrId` fields re-interned per session; the linker now interns these at the point
  of use (the entry-symbol lookup and the dynamic interpreter path), completing the
  interner-elimination from the OS vtable started in #1377. Behavior-preserving,
  verified by the byte-identical self-host fixpoint (#1402).
- target: the dead `IsaVTable.endianness` field is removed. It was set by the ISA
  registrars but read by nobody — the object-format writers hardcode little-endian —
  so the field documented a behavior that did not exist. The `ENDIAN_LITTLE`/
  `ENDIAN_BIG` enum is retained for the eventual big-endian abstraction, and the
  comments that claimed endianness was honored now state the little-endian
  assumption plainly (#1411).
- objfmt (COFF): the writer/reader resolve every relocation's target symbol through a
  name→index map built once per pass instead of a linear symbol-table scan per
  relocation, turning the two reloc passes from O(reloc × symbol) into O(reloc +
  symbol). Behavior-preserving (same indices resolved, same unresolved-symbol error),
  verified by the byte-identical self-host fixpoint (#1413).

### Fixed

- target (aarch64): `IsaVTable.fp_scratch_reg` for the (stub) arm64 backend is now
  the composite vector register id `regid_make(REG_CLASS_ID_XMM, 31)` instead of the
  raw bank index `31`, which would have read as a GP register. Dead today (the arm64
  backend has no encoder), corrected before it seeds the future backend (#1414).
- objfmt: the COFF and ELF object-file parsers allocate the section/symbol/relocation
  arrays up front but left each `ObjectImage` count at `0` until its populate loop
  finished, so an error return before or within those loops (a truncated or hostile
  object) leaked the arrays — `obj.dnit` frees nothing when the counts are `0`. Each
  count is now set to its populated size immediately after the array is allocated (the
  loops use local write cursors), so teardown reclaims the full arrays on any later
  parse error; the ELF symbol array is sized and counted to the populated count
  (excluding the reserved index-0 entry) so allocation, count, and entries all agree.
  Emitted output is unchanged (#1410).
- objfmt (COFF): REL32 relocations are next-byte-relative (`S + A − (P+4)`), but the
  abstract addend uses ELF's field-relative convention (`S + A − P`), so the writer's
  on-wire field and the reader's recovered addend were off by 4 — a foreign
  (MSVC/clang/GAS) COFF read by mach landed calls 4 bytes past target, and mach's
  emitted `.obj` was off by −4 under link.exe/lld. The writer now folds `A_coff =
  A_elf + 4` and the reader recovers `A_elf = A_coff − 4` for REL32 only; ABS64
  (ADDR64) and ADDR32NB are unaffected. A mach-emitted `.obj`'s REL32 wire field
  changes (that is the fix — a `call`'s field is now 0, the COFF convention), but a
  mach→mach round-trip recovers the same abstract addend and links to the same final
  binary; only foreign-COFF interop behavior changes (#1409).

## [1.5.3] - 2026-06-13

Correctness patch for two silent defects in shipped v1.5.2: a relocation
patch-site miscompute that corrupted out-of-`imm32` constant stores to globals,
and an extreme float-literal exponent that hung the compiler.

### Fixed

- codegen (x86-64): an 8-byte store of an immediate outside signed-`imm32` range to
  a global is legalized into `MOVABS r11, imm64` + a register store, but the PC32
  relocation was patched at the pre-legalization offset — landing 4 bytes early,
  corrupting the encoding and leaving the store pointed at `rip+0`. The patch site
  now tracks the legalized `disp32` (the same fix covers the ALU memory-destination
  and inline-asm `mov [sym], imm64` paths). No diagnostic was emitted; ordinary code
  storing a large constant to a global silently miscompiled (#1407).
- comptime: a float literal with an extreme exponent (e.g. `1.0e2000000000`) hung the
  compiler in the unbounded decimal-scale loop and could silently fold to its mantissa
  on i64 exponent overflow. The exponent accumulator is now clamped past the point
  f64 saturates, so such literals fold to `inf`/`0.0` instead (#1408).

## [1.5.2] - 2026-06-13

Maintenance patch: path dependencies materialize through the standard library
instead of host `ln`/`rm`, the target vtables become immutable `str`-named
singletons, and the windows CI gains a trailing-import-descriptor regression
guard.

### Fixed

- Path dependencies materialize via the std filesystem primitives (`fs.symlink`,
  `fs.remove_all`) instead of shelling to `ln -s` / `rm -rf`, so they no longer
  require host `ln`/`rm` — fixing path-dependency materialization on native
  Windows, where `ln` is not on `PATH`. The `git` transport is unchanged (#1392).

### Changed

- The OF/ISA/OS/ABI vtable `name` and register-class names are immutable `str`
  constants set directly by the registrars, no longer `StrId` fields re-interned
  per session; the dead `OfVTable.file_extension` is removed. Behavior-preserving,
  verified by the byte-identical self-host fixpoint (#1377; the remaining
  interner-elimination is tracked in #1402).
- CI: a `threadsync` wine integration test guards the trailing PE
  import-descriptor call-thunk against regression (#1388), and the win64 wine
  tests are bounded with a `timeout` so a faulting binary fails fast instead of
  hanging the lane (#1399, #1404).

## [1.5.1] - 2026-06-13

Native Windows lane: CI now builds `mach.exe` and runs the in-source test suite
on real Windows, not just the wine cross-compile path. Two codegen fixes complete
the windows backend end to end — per-page stack probing for frames over a page,
and per-descriptor PE import call-thunks — and the vendored standard library is
updated to its windows-complete release. The native lane passes 468/468.

### Fixed

- Windows function prologues now probe the stack one page at a time for frames
  larger than a page, instead of a bare `sub rsp, N`. Windows commits the stack
  one guard page at a time, so a single large subtraction skipped the guard page
  and the first write near the bottom of the frame faulted on reserved memory
  (`STATUS_ACCESS_VIOLATION`); Linux and wine auto-grow the stack on any in-range
  fault and so never surfaced it. The encoder now emits the inline `__chkstk`
  page-walk (mach links no runtime) when the target OS commits incrementally — a
  new `OsVTable.stack_probe` flag (true on Windows, false on Linux/Darwin) gated
  by the OS `page_size`. This was the root cause of the native-Windows exec
  failures in briar-systems/mach-std#262 (a 32 KiB `spawn_redirected` cmdline frame)
  (#1395).
- PE import call-thunks for the **last** import descriptor jumped through the
  previous descriptor's null IAT slot (a `jmp 0` access violation on the first
  call). `pe_iat_slot_rva` never advanced its dependency index, so it re-scanned
  only the first descriptor when mapping an import ordinal to its IAT slot — the
  trailing descriptor's stubs landed short of its `FirstThunk`. Reordering the
  `libs` list moved the breakage to whichever DLL was last. Now every import's
  thunk targets its own descriptor's slot, so the trailing descriptor's calls
  (e.g. advapi32's `SystemFunction036`) dispatch correctly (#1388).

## [1.5.0] - 2026-06-12

Inline-asm & foundations: carry-flag mnemonics and numeric local labels in x64
inline assembly, per-symbol DLL attribution for windows imports, the first PE
output the native Windows loader accepts, float `%` correctness, `nil`
coercion to function types, materialized `path` dependencies, declared
foreign-target runners, and session-owned target registries under the hood.

### Added

- `mach test` and `mach run` accept `--runner <cmd>`: every child exec becomes
  `<cmd> <binary> <args...>`, the declared host-side launcher for
  foreign-target artifacts (e.g. `--target windows --runner wine`). The value
  is a single command name or path (no shell-style word splitting), resolved
  on `PATH`. Absent the flag, binaries are exec'd directly and a launch
  failure is reported as a failure — no auto-detection (#1345).
- One-line install scripts: `install.sh` (Linux) and `install.ps1` (Windows),
  shipped in the repo and as release assets. They resolve the latest tag from
  the `releases/latest` redirect, download the versioned archive for the host,
  verify it against `SHA256SUMS`, and install to `~/.local/bin`
  (`%LOCALAPPDATA%\mach\bin` on Windows); `MACH_VERSION` and
  `MACH_INSTALL_DIR` override the release and destination (#1352).
- Releases now ship versioned archives — `mach-<version>-x86_64-linux.tar.gz`
  and `mach-<version>-x86_64-windows.zip`, each containing the binary and
  LICENSE — alongside the existing assets, with `SHA256SUMS` covering the
  full set (#1352).
- x64 inline assembly accepts the carry-flag mnemonics `jc` / `jnc` (and the
  `jb` / `jae` / `jnb` aliases) and `setc` (and the `setb` / `setae` / `setnb`
  aliases), reusing the existing conditional-branch and SETcc encoders. Previously
  only `je` / `jz` / `jne` / `jnz` were recognized, forcing carry-flag handling
  through `.byte` escape hatches (#1359).
- x64 inline assembly accepts NASM-style numeric local labels: a `<digits>:`
  statement defines a block-local label and `<digits>f` / `<digits>b` branch
  targets resolve to a rel32 within the function (no relocation). Every branch
  mnemonic takes a symbol or a local-label target; a backward reference binds to
  the nearest preceding definition and a forward reference to the nearest
  following one, redefinition of a number is allowed, and an unresolved or
  malformed reference is a hard compile error. This unblocks block-local forward
  branches such as the `jc 1f` / `1:` shape in std's darwin syscall wrappers
  (#1365).
- `$<sym>.library = "<dll>"` pins an `ext` import to a specific dependency DLL,
  giving the PE (Windows) import directory per-symbol attribution. Imports were
  previously all forced onto the first dependency (kernel32.dll), so extra DLLs
  in `[target.*].libs` emitted only empty descriptors; an attributed import now
  lands under its named library, an unattributed one still binds to the first,
  and pinning to a library absent from the link's dependencies is a hard link
  error rather than a silent fallback. Composes with `.symbol` — the rename sets
  the imported name, `.library` the DLL it is imported from (#1382).

### Fixed

- Float `%` now evaluates to the truncated (C `fmod`) remainder
  `a - trunc(a / b) * b`, whose result takes the dividend's sign
  (`5.5 % 3.0 == 2.5`, `-5.5 % 3.0 == -2.5`). The operator had no float lowering:
  the runtime path fell through to the integer IDIV/DIV opcodes, running an
  integer divide over the raw IEEE-754 bit patterns (a passthrough-below-divisor,
  near-zero-above shape), and the comptime fold rejected it as non-constant. Both
  now synthesize the remainder from the existing float divide / truncating
  conversion / multiply / subtract primitives, exact for `|a / b| < 2^63` (#1378).
- `nil` coerces to function types, so a fun-typed binding can be explicitly
  nil-initialised, not only default-initialised. Previously `var q: fun(u32) =
  nil` was rejected with `type mismatch: expected fun(u32), found *u8` and the
  cast spelling `var r: F = nil::F` failed lowering with `global initialiser
  must be a constant expression`, even though a fun-typed value already compares
  `== nil` and `nil::F` was accepted in argument position. nil now coerces to
  any pointer-like target — `ptr`, `*T`, or `fun(...)` — uniformly across
  globals, locals, record fields, array elements, arguments, and return slots,
  and a nil global initialiser (bare or written through pointer casts) folds to
  the null constant. nil into a non-pointer slot remains a type error (#1369).
- `mach dep pull` now materialises a `path` dependency at `<dep>/<alias>/` as a
  relative symlink to the resolved source instead of silently doing nothing —
  previously it printed "mach.lock up to date", exited 0, and never created the
  dep directory, so any later build failed to resolve the dep's modules. The
  `path` is resolved relative to the requiring manifest's directory; a stale link
  is replaced and an already-correct one left in place (idempotent), while a
  missing directory, a directory without a `mach.toml`, or a vendor location
  occupied by a real directory is a hard error. A path dep carries no lock entry —
  its manifest `path` is the record (#1370).
- Sema reports a teaching diagnostic for every symbol kind that can never be a
  value reaching value position — a record, union, or `def` type name (local
  or imported, bare or `alias.member`), a generic type parameter, and a member
  access on an expression with no value (a call with no return type) — instead
  of silently poisoning and surfacing link `undefined symbol` or span-less
  lowering errors; completes the #1343 silent-poison audit (#1348).
- The PE emitter no longer produces executables the native Windows loader
  rejects with `ERROR_BAD_EXE_FORMAT`. The image base reserved a full 64 KiB
  below the first segment, placing the first section at RVA `0x10000` while the
  headers spanned one page — a 15-page unmapped gap the loader refuses (wine
  tolerated it). ImageBase now sits exactly one header span below the first
  segment, so the first section maps immediately after the headers with no gap,
  matching the layout MSVC emits. Every `mach`-built Windows binary, including
  the published `mach.exe`, now launches natively (#1374).
- A `$<sym>.symbol` rename on an `ext fun` is no longer clobbered. The bare `ext`
  identifier was re-registered over the rename after `scan_export_directives`
  recorded it, so renames on foreign imports silently did nothing and bound under
  the mach identifier; the bare name is now only a default that an explicit
  `.symbol` override wins, order-independently (#1382).
- `mach init` validates the project id before scaffolding: a name the manifest
  grammar would reject (`.`, path separators, spaces) is refused with nothing
  written, instead of silently scaffolding an ungrammatical `[bin..]` manifest.
  The positional name is taken verbatim — no basename derivation (#1355).
- A type mismatch against a call with no return type reads `expected i64, found
  no value` (with a clarifying note) instead of rendering the misleading
  `<error>` placeholder, and diagnostics consistently say "returns nothing" /
  "no value" — mach has no `void` (#1360).
- `infer_generic_call` reports internal generic-substitution and type-argument
  allocation failures instead of silently poisoning the expression (#1361).

## [1.4.1] - 2026-06-12

Patch release clearing the open bug board: environment forwarding for spawned
user programs, the `-o` absolute-path override, `fwd` module re-export
consumption, and inline-asm comment handling.

### Changed

- Bumped the vendored mach-std to v0.6.0 (adds `std.process.env.environ()`;
  fixes the thread spawn/join deadlock and json key lookup).

### Fixed

- `mach test` and `mach run` now forward the parent environment to spawned
  user programs instead of execing them with an empty `envp`; `getenv` in a
  test or run child sees the inherited variables (mach-std#197).
- An absolute `-o` path is honored verbatim; previously its leading slash was
  swallowed by an unconditional join with the project root and the output
  landed project-relative (#1340).
- A consumer can chain through `fwd` module re-exports in expression position
  (`lib.alpha.answer()`), at any depth including a `fwd` of another library's
  `fwd`; a module alias referenced in value position now reports
  `a module alias is not a value` instead of silently poisoning and surfacing
  internal lowering errors (#1343).
- Inline-asm comments are now opaque to the instruction parser regardless of
  their bytes. A comment containing `;` was split into a phantom instruction
  and one containing `{...}` was misread as a local binding; comments are now
  stripped to end-of-line when the asm body is materialized, before any
  substitution or statement tokenization (#1297).

### Removed

- The unused test-runner write-primitive machinery (`RunnerWrite`,
  `OsVTable.runner_write`): since 1.4.0's per-test executables, test binaries
  perform no OS output — the host process reports — so the vtable hook had no
  consumers on any OS (#1292).

## [1.4.0] - 2026-06-12

The clean-break release: one manifest format, the project module, and the
test infrastructure the language was designed around.

### Added

- **The project module**: `[project] module = "lib.mach"` — a bare project-id
  `use glfw;` / `fwd glfw;` resolves to the project's declared module. Never
  inferred; module-less bare imports and dangling declarations error loudly.
- **OS link overlays**: `[os.<name>] libs = [...]` — link requirements scoped
  to one tuple component, cascading to consumers across every ISA/ABI of that
  OS (`[isa.*]`/`[abi.*]` reserved).
- **Per-test executables**: every `test` block builds to its own standalone
  program under the `tests` path template; `mach test` runs each in its own
  process — a crashing test reports `FAIL(signal n)` and the run continues —
  with `--list` and `--filter`. Test artifacts never touch the project binary
  path.
- The compiler versions itself from its manifest (`$project.version`):
  release bumps are now a one-file change.

### Changed

- **One manifest format.** The pre-1.3 manifest system is removed entirely —
  `[targets.*]`, `dir_*` keys, string opt levels, and `type/path/version`
  dep stanzas no longer parse. Projects update their manifests by hand;
  doc/manifest.md documents the format.
- `[profile.*] opt` is an integer (`0 | 1 | 2`).
- `mach dep pull` clones into an empty dependency placeholder directory
  (plain-clone installs work without `--recurse-submodules`).
- Inline-asm comments are fully opaque to the instruction parser; every
  audited diagnostic now names its subject and carries a span.

## [1.3.1] - 2026-06-11

Transitional self-seeding release. Carries the v1.4.0 feature line developed
so far and exists so the next release can drop the old manifest format
entirely: this binary reads both manifest formats; the next reads only the
current one.

### Added

- The new `mach.toml` manifest format: explicit `[target.*]`/`[bin.*]`/
  `[lib.*]`/`[profile.*]`/`[deps.*]` stanzas, path templates, `native` target
  resolution, multi-artifact matrix builds, tuple-matched cascading dependency
  libs, per-target comptime `defines`.
- The `mach dep` model: `pull`/`update` with a manifest-is-intent lockfile
  law, `git`/`path` source forms, transitive resolution.
- Comptime namespace roots `$project.*`, `$target.*`, `$bin.*`; bare `$ident`
  is now rejected with a teaching diagnostic.
- `mach info` (and `mach info --version`): compiler version, build host, and
  the registered target capability surface.
- Mixed-numeric comparisons: legal and value-correct across signedness and
  widths (comptime agrees with runtime); int↔float still requires a cast.
- A diagnostics overhaul: ~70 audited messages now carry spans, name the
  offending symbol, and say what to do.

### Changed

- Release-mode builds are ~5x faster (quadratic pass costs removed); the
  optimizer gained phi simplification, post-inline promotion, and call-graph
  aware inlining.
- The IR verifier is real (dominance, reachability, type agreement) and CI
  enforces `--release --verify-ir`.
- Relocation kinds are a closed enum; the object-format layer no longer
  captures per-session interners.
- The inline corpus grew to ~390 tests; comment content inside `asm` blocks
  is now fully opaque to the instruction parser.

## [1.3.0] - 2026-06-11

Correctness and foundations release. A full-compiler audit (125 findings,
45 confirmed bugs) was executed end to end: every confirmed miscompile is
fixed, the ABI abstraction boundary is complete ahead of new targets, and
mixed-numeric comparisons joined the language with value-correct semantics.

### Added

- **Mixed-numeric comparisons**: integer comparisons across any signedness
  and width are now legal and compare mathematical values, with results
  identical in both operand orders (`i64`/`u64` lowers to a sign-test plus
  unsigned compare). Float widths mix exactly; int↔float still requires an
  explicit cast. Comptime evaluation agrees with runtime semantics on the
  same boundary cases.
- Variable-index function-pointer dispatch: `table[i]()` now parses correctly
  (the bracket payload is resolved against the callee — generic call vs
  index-then-call — instead of guessed in the parser).
- C interop, win64 variadic, float-argument, conversion-boundary, comptime/
  runtime-agreement, and release-verifier integration suites run in CI.

### Fixed

- **Miscompiles**: FP-register interference tracking (swapped float arguments
  collapsing); win64 callee-saved XMM6–13 never preserved (with full
  `UWOP_SAVE_XMM128` unwind coverage); inline-asm clobber inference
  implemented as documented; `u32`→float converting as signed; register
  `i32`→`i64` sign-extension reading only 16 bits; SysV float-bearing
  aggregates never classified to SSE registers (C FFI divergence); comptime
  `u64`-range constants evaluating as negative.
- The program entrypoint entered every callee with an 8-byte misaligned
  stack (mach-std v0.4.2) — fatal for C callees using aligned SSE accesses.
- The IR verifier now verifies: real dominance (near-linear), reachability,
  operand type agreement, dangling-definition detection; `--release
  --verify-ir` passes on the full compiler and is enforced in CI.
- Optimization passes: constant folding no longer crashes the compiler on
  `INT64_MIN / -1`; dead phis are eliminated; phi simplification unblocks
  constant propagation across inlining; the inliner refuses call-graph
  cycles.
- The ELF/COFF writers and linker fail loudly on unresolved relocation
  symbols, unknown relocation kinds, and malformed objects; weak/strong
  symbol resolution is order-independent; section/relocation counts are
  validated before narrowing.
- IR teardown frees operand arrays and aggregate blobs by true capacity —
  correct under any allocator.
- CLI: `mach run --` argument passthrough, `mach init --name`,
  `mach build <path>`, a lockfile-writer heap overflow, `--quiet`/`--color`
  now functional, unified exit/signal handling.
- Frontend: the grammar holes vs the locked spec (multi-line strings,
  `val`/`var` forms, integer-literal overflow) are rejected with diagnostics;
  parser OOM can no longer yield a silently corrupt AST.
- Generics: cross-module arity checking, deeply nested parameter
  substitution, `fwd` module re-export, duplicate diagnostics.

### Changed

- The ABI layer is complete and arch-keyed: selection consults (isa, os),
  classifiers carry an explicit by-reference class, the variadic model is
  vtable-driven end to end, and the test runner's output primitive comes
  from the target OS — groundwork for the aarch64 and darwin targets.
- `mir` is split into per-concern modules (data model, lowering context,
  IR→MIR core, calling-convention lowering, variadic expansion) with
  byte-identical output.
- `mach init` scaffolds mach-std pinned to `branch/main` (v0.4.x); the
  vendored std is v0.4.2.
- Object-format writers share a common binary-IO layer.

## [1.2.0] - 2026-06-10

Native Windows release. `mach.exe` now builds Mach itself on the win64 target
to a byte-identical fixpoint (verified under wine), and releases ship
per-target artifacts.

### Added

- **Per-target release artifacts**: `mach-x86_64-linux`, `mach-x86_64-windows.zip`
  and `SHA256SUMS` alongside the bare `mach` seed binary; the windows artifact is
  gated on a wine smoke test in which `mach.exe` builds and runs a real project.
- CI now runs the integration suites (dynlink, extlink, opt, win64byref,
  win64fnptr) with wine on every pull request, plus a windows cross-build of the
  compiler.
- Per-function `.pdata`/`.xdata` unwind metadata on win64 executables, with
  spec-correct `UNWIND_INFO` encoding.

### Fixed

- Function-to-pointer casts (`fn::*u8`) lowered as a 32-bit truncating move,
  corrupting every type-erased function pointer above 4 GiB — the fault that
  kept native `mach.exe` from running on win64 image bases.
- win64 by-reference aggregates passed as a fifth-or-later argument: caller and
  callee now agree on the spilled hidden pointer via an explicit ABI class
  (`CLASS_STACK_BYREF`), covering sub-pointer (3/5/6/7-byte) aggregates.
- win64 unwind metadata: removed the incorrect frame-register declaration,
  fixed `UWOP_SAVE_NONVOL_FAR` to record unscaled offsets, and stopped
  misreading `[r13+disp]` stores as frame saves.
- The ELF and COFF writers now fail loudly, naming the symbol, when a
  relocation targets an unresolved symbol instead of silently emitting a
  corrupt object.
- COFF extern-function inference is scoped to foreign objects, indexed O(n),
  and propagates allocation failure.
- `mach dep`: unified dependency-root resolution across sync/add/remove,
  proxy environment variables (`ALL_PROXY`, `all_proxy`, `no_proxy`) forwarded
  to git, idempotent lockfile writes, and `mach init` no longer pre-creates
  the dependency directory.
- Release tags are verified against the manifest version before any artifact
  is built.

## [1.1.1] - 2026-06-09

Patch release for project scaffolding correctness.

### Fixed

- `mach init` now scaffolds fully buildable projects with all required manifest
  entries and `mach-std` dependency wiring.

## [1.1.0] - 2026-06-08

Tooling and cross-compilation release. Mach can now emit Windows executables,
link dynamically, manage dependencies, and answer editor queries — while the
Linux self-host continues to build to a byte-identical fixpoint.

### Added

- **Windows cross-compilation** for `x86_64-windows`: the Microsoft x64 calling
  convention (win64 ABI), a COFF/PE object and executable writer, and kernel32
  import linking. Mach builds runnable Windows `.exe`s. (Running the compiler
  itself natively on Windows is in progress for a later release.)
- **ELF dynamic linking**: link against shared libraries via `-l`/`-L` and
  `[targets.*].libs`, with a real PLT/GOT and `DT_NEEDED`/`PT_INTERP`.
- **External linking and static archives**: link prebuilt `.o`/`.a` inputs; a
  Unix `ar` archive reader.
- **`mach dep`**: git-based dependency management (`add`/`remove`/`sync`/`vendor`)
  with a `mach.lock`.
- **`mach check`**: single-file diagnostics with no project or link step.
- **Per-target optimization levels** via the manifest, overridable on the CLI.
- **Editor query surface** (`mach.lang.editor`): single-file/unsaved-buffer
  open, parse, resolve, and diagnostics for tooling and language servers.

### Fixed

- `fwd` re-exports now resolve against the dependency set correctly.
- x86-64 `imul` by a constant outside signed-imm32 range no longer truncates to
  the low 32 bits (silent miscompile of large-constant multiplies).
- A global `val` initialized from a constant cast no longer silently lowers to
  zero; a non-foldable global initializer is now a hard error.
- Several win64 codegen fixes (shadow space, variadic definitions, callee-saved
  register preservation) and a COFF weak-symbol round-trip.

## [1.0.0] - 2026-06-06

First stable release of Mach: a self-hosting, dependency-free native compiler
for the Mach systems programming language. The compiler builds its own source
to a byte-identical fixpoint and emits statically-linked x86-64 ELF directly,
with no external backend, assembler, or linker.

### Added

#### Compiler

- Self-hosting compiler that builds its own source to a byte-identical fixpoint.
- Direct x86-64 (Linux, SysV ABI) native code generation: lexer, parser,
  resolver, semantic analysis, an SSA mid-end, instruction selection,
  linear-scan register allocation, and ELF object/executable emission — with no
  LLVM and no external assembler or linker.
- Optimization pipeline: `mem2reg` (stack-to-SSA promotion), constant folding,
  dead-code elimination, function inlining, algebraic simplification, and local
  common-subexpression elimination. `-O0` runs the always-on subset
  (`mem2reg` / constant folding / DCE); `-O1` and `-O2` run the full pipeline.

#### Language

- Records (`rec`) and overlapping-layout unions (`uni`).
- Generics with bracket syntax (`T[U]`) and monomorphization.
- Compile-time evaluation: `$if` / `$or` branch selection, `$mach.*` target
  parameters, comptime value-parameter monomorphization, and value/layout
  intrinsics (`$size_of`, `$align_of`, `$offset_of`, …).
- Two cast operators: `::` (value conversion) and `:~` (same-size bit
  reinterpret).
- Pointers (including `nil`), slices, and fixed-size arrays.
- Error handling with `Result` and `Option`.
- Modules with `use`, module aliases, and `pub` visibility.
- Inline assembly (`asm`) and variadic functions.

#### Standard library

- `mach-std`: runtime, allocators, strings, collections, I/O, filesystem,
  formatting, OS/syscall bindings, and the core `Option` / `Result` types.

#### Tooling

- `mach` CLI: `build`, `test`, and `init`.
- Differential test harness (optimization-level and cross-compiler miscompile
  detection), a crash fuzzer, and a compiler compile-time benchmark harness.
