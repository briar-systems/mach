# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
  failures in octalide/mach-std#262 (a 32 KiB `spawn_redirected` cmdline frame)
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
