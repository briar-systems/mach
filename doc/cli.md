# `mach` — the command-line interface

```
mach <command> [options]
```

The compiler dispatches on `argv[1]`. With no command, or an unknown one, it
prints usage and exits `1`. The project commands — `build`, `check`, `run`,
`test`, `clean`, `fmt`, and `doc` — take the project as a **required** positional: a directory
containing `mach.toml`, or the path of a `mach.toml` itself. Nothing is
searched for: `mach build src` inside a project is `error: no mach.toml in the
project directory`, and a bare invocation with no path is a user error.

This page documents the flags the current binary actually parses; every
command's help (`mach help <command>`, or `mach <command> -h`) is rendered
from the same schema the parser consumes, so the two cannot disagree. Flags
are matched exactly: `--flag value` (a value follows in the next argument) or a
bare `--flag` toggle. The combined `--flag=value` form and bundled short flags
are **not** recognized (`unknown flag '--profile=debug' for 'mach build'`).

An unrecognized flag is a hard error. Before it resolves positionals, each
command rejects the first `-`-prefixed token it does not accept — `error: unknown
flag '<flag>' for '<command>'`, exit `1` — so a typo'd or removed flag never
silently misparses as a project path or link input. An option has exactly one
effect or is rejected: a build-only option under `run`, or `--emit` under
`test`, is an error naming the command, never an accepted no-op.

The `--` end-of-flags separator is honored only by `mach run`, which forwards
every token after `--` to the executed binary as its `argv` — `mach run <path> --
--flag` passes `--flag` through to the program. On every other command `--` has
no special meaning: it is an unmarked `-`-prefixed token and is rejected as an
unknown flag like any other.

Consequently a lone `-`, a `--`, or a negative-number token (e.g. `-1`) at a flag
position is rejected as an unknown flag. No CLI positional legitimately begins
with `-`, so a dash-leading positional is unsupported outside `mach run`'s
argument forwarding.

## Commands

| Command | Summary |
|---------|---------|
| `build` | compile the project to objects and (for a `bin` artifact) a linked binary |
| `check` | run the frontend over the source the selected artifacts reach, and stop |
| `run`   | execute the already-built binary (a post-`build` convenience, not a rebuild) |
| `test`  | build the test binary and run the collected tests |
| `clean` | remove the project's build output directory trees |
| `fmt`   | rewrite the project's own source in the one canonical layout, or report what differs |
| `dep`   | realize, verify, and change the project's dependencies under `dep/` |
| `init`  | scaffold a new project |
| `doc`   | generate Markdown reference docs from source docstrings |
| `info`  | print compiler version, build host, and registered target capabilities |
| `help`  | print usage; `mach help <command>` for detail |

## Global flags

Read by `build` and `test`, which share one schema. `check` accepts the
selection subset without `-o` (`--target`, `--profile`, `--bin`, `--lib`) and
the readout flags, `run` only the selection subset (`--target`, `--profile`,
`--bin`, `-o`) and `doc` only
`--target`, `--bin`, `--lib`, `--out`, and `--quiet`; every other option is
unknown to them by name (`unknown flag '--emit-ir' for 'mach doc'`,
`unknown flag '--lib' for 'mach run'`), exactly as a misspelled one is. A
verbosity flag (`-v`/`-vv`) and `--quiet` together is an error
(`` `-v`/`-vv` and `--quiet`/`-q` cannot be combined ``).

| Flag             | Value            | Effect |
|------------------|------------------|--------|
| `-v`             | —                | `mach build`: per-phase roll-up (load/resolve/sema/lower/optimize/codegen/link) with item counts + timing, then a `built … N modules … in …` summary, on stderr |
| `-vv`            | —                | `-v` plus a per-module/file line under each phase with its duration and a `(slow)` marker on the slowest |
| `--quiet`, `-q`  | —                | suppress non-error output |
| `--target <name>`| target name      | select a declared target; absent, resolves the host-matching declared target (`native`) |
| `--profile <name>`| profile name    | select a `[profile.<name>]` build variant, whose `opt` sets the optimisation pipeline; absent, the sole declared profile, else the one marked `default = true` (see [manifest.md](manifest.md#profile-requirement-and-selection)) |
| `--bin <name>`   | artifact name    | narrow the build to one `bin` `[artifact.<name>]` |
| `--lib <name>`   | artifact name    | narrow the build to one `static`/`shared` `[artifact.<name>]` (mutually exclusive with `--bin`) |
| `-o <path>`      | path             | override the artifact path, rooted at the project root (build/run/test); accepted only when the selection resolves to one cell |
| `--all-targets`  | —                | build every declared `[target.*]`, not just the default |
| `--emit-asm`     | —                | emit per-module assembly text (`.s`) — each line is one machine instruction the encoder emitted for that module, so the `.s` corresponds to the `.o` instruction for instruction. Emission is opt-in, controlled only by this flag |
| `--emit-ir`      | —                | emit per-module SSA IR text (`.ir`), the final post-pipeline IR the object is built from (so it varies with the profile's `opt`) — emission is opt-in, controlled only by this flag |

The IR verifier runs between every optimization stage in every build; there
is no flag to enable or disable it (the former `--verify-ir` is rejected as an
unknown flag).

> Under `mach test`, the entry module's `--emit-ir` dump shows the neutralized
> project entry the test dispatcher substitutes for the real one (the final IR
> that build is made from), so it differs from the same module's `mach build`
> dump. That divergence is expected.

> `mach dep`, `mach init`, and `mach clean` do not use the shared config
> parser; they read only their own flags listed below.

## `mach check`

```
mach check <path> [--target <name>] [--profile <name>] [--bin <name> | --lib <name>] [-v | -vv | --quiet]
```

Runs the frontend over the source a build would compile and stops there.
Selection is `mach build`'s: every artifact declared for the resolved target,
or the one `--bin`/`--lib` names, on the selected profile, and the source
checked is what those artifacts reach from their entries through `use`,
including dependency modules. That is the selected-artifact graph, not the
whole `src` tree: a file nothing selected imports is not read, and a `--bin`
that reaches less checks less.

Each cell runs load, resolve and sema through the same driver, queries and
phase outcomes `mach build` and the editor use, so what check accepts, rejects
or fails on is what the same build's frontend would, with the same diagnostics
at the same locations and the same classification. The exit status is that
classification: 0 accepted, 1 rejected (or a user error such as a bad manifest
or selection), 2 an internal failure, 3 an environment failure. It is never
derived from counting or matching diagnostic text.

Nothing else runs. No build step or dependency step executes, nothing is
lowered, generated, linked, published or written, and `out/` is not created.
A generated source module or an embedded input that a step or a required
artifact would produce is reported as the missing input it is, naming it; run
the build that produces it first. A clean check is a statement about the
frontend only: it does not establish that lowering, code generation, the ABI or
the link would succeed.

## `mach build`

```
mach build <path> [options]
```

Compiles the project named by `<path>` — a project directory containing a
`mach.toml` (e.g. `mach build .`).
With no `--bin`/`--lib`, it builds every declared artifact for the selected target
and profile. An artifact another one requires through `need` is built first, on the
targets it declares, whether or not the selection names it; `-o` still names the
selected artifact's own output. See
[manifest.md](manifest.md#artifact-requirements).
Each artifact's module graph is rooted at its `entry`; only that module
and its transitive `use`/`fwd` dependencies belong to the build cell. Other files
under `src` may back artifacts for different targets and are not compiled for this
cell. Every reachable module is driven through sema → lower → optimise → codegen to
one relocatable object, written under the resolved object tree at
`<out>/obj/<fqn-as-path>.o`. For a `bin` artifact the objects are linked into the
resolved artifact path (its `out` template); for a `static`/`shared` library (or with
`--emit obj`) the objects are the deliverable and nothing is linked.

A target whose object format delivers **finished modules** — SPIR-V, whose object
output is a complete self-contained module rather than a link input — always
takes the second shape. Each module is written as `<out>/obj/<fqn-as-path>.spv`
and the module tree is the artifact, so `mach build --target <spirv-target>` and
`mach build --target <spirv-target> --emit obj` produce exactly the same files.
There is no executable to link, no archive or shared-library form, and no test
dispatcher to run; each is refused by name rather than attempted.

Relocatable object output preserves explicit section names, native flags and
section associations, including empty sections. Symbols retain local, global,
weak, common and absolute definitions. A relocatable combination remaps section,
symbol and relocation references without applying final executable layout or
removing debug sections. `--emit obj` still delivers the per-module object tree
described above, rather than combining that tree into one file.

ELF section groups and Mach-O difference relocations and indirect-symbol tables
retain their native relations. COFF DLL-attributed imports use native import
tables and code thunks, preserving named or ordinal lookup and public aliases.
Unrelated sections remain intact. Native metadata belongs to its object format:
an output writer rejects metadata from another format, and ELF or Mach-O
relocatable output rejects provider-attributed imports it cannot represent.

Mach-O may combine eligible compiler-generated default data sections, but keeps
explicit placements and parsed native sections distinct. An output requiring
more than 255 distinct Mach-O sections is refused. COFF common symbols use its
size-derived alignment, capped at 32 bytes, and reject an explicit alignment
that the native common-symbol form cannot preserve.

Final linking resolves absolute symbol values without treating them as external
references or adding a loader base adjustment. Native Mach-O indirect-only
sections currently require relocatable output. Position-independent links also
reject absolute-address expressions that require an unsupported runtime base
adjustment, including differences between absolute and image-relative symbols.

The optimisation pipeline comes from the selected profile's `opt` — the profile
is how a build picks its optimisation level.

`--plan` resolves the effective build — the (target, artifact) matrix and, per
unit, the selected project, target, profile, artifact, entry, output paths, the
prerequisite steps in execution order (each dependency's exported steps first,
then the project's own), the artifact requirements, the manifest, dependency
export and command-line link requirements, and the phase order — prints it, and
exits. Nothing is executed: no generator or build step runs, no dependency is
fetched, no compiler or linker runs, and no output is written or published.

The plan is resolved through the same planner and the same dependency
configuration a build uses, so an invalid manifest, a dependency that is not
realized, and a dependency cycle are reported here exactly as a build reports
them, with the same classification. What planning cannot know it says instead of
guessing: a unit with prerequisites prints that its generated input bytes are
unresolved until those prerequisites run. A printed plan is a statement about
selection, not a claim that those bytes are valid or that the link will
succeed.

| Flag           | Value          | Effect |
|----------------|----------------|--------|
| `-O0`          | —              | force the debug pipeline for this build, overriding the selected profile's `opt` |
| `-O2`          | —              | select the release pipeline, overriding the selected profile's `opt`; it includes loop auto-vectorization on a vector-capable target (the profile's `vectorize` key and the `#[scalar]` decorator opt out). `-O1` is rejected on every command with one message (`-O1 was removed; use -O0 or -O2`) until a distinct pipeline exists |
| `-g`           | —              | emit debug info for this build, forcing the selected profile's `debug` on (precedence `-g` > profile > off) |
| `--emit <kind>`| `obj`\|`exe`   | `obj` stops at the relocatable objects; `exe` (default) links a binary |
| `--cache`      | —              | reuse and publish persistent object images under the output directory (opt-in, see below) |
| `--no-cache`   | —              | force a genuinely uncached build: no persistent object images and no build-step reuse; wins over `--cache` |
| `--jobs <n>`   | count          | codegen worker threads (default: host CPUs; `1` serialises) |
| `--pie`        | —              | emit a position-independent (ET_DYN) executable for ASLR instead of the default fixed-address one; opt-in (see below) |
| `--subsystem <k>` | `console`\|`gui` | the environment a windows executable declares it runs under, overriding the artifact's `subsystem` key (see below) |
| `-L <dir>`     | dir            | add a search directory for `-l`-resolved inputs; repeatable |
| `-l <name>`    | name           | link a named object, archive, or target-format shared library, resolved through the `-L` dirs (see below); repeatable |
| `--plan`       | —              | print the effective build plan and exit without building |
| *(positional)* | input path     | a bare argument that contains `/`, ends in `.o` / `.obj` / `.a` / `.lib`, or names a `.so`, `.dylib`, or `.dll` is linked explicitly |

Plus the global flags above.

With `--cache`, successful code generation stores serializable object images in
`.mach-cache` under the selected project output directory, and a later compiler
process that opts in reuses them. The front end still runs; a module whose
object the cache holds skips lowering, optimization and code generation, and
linking still resolves and validates its current inputs (the linked artifact is
never cached). The key includes the running compiler's build id, the selected
build configuration, the active source graph including dependency and generic
bodies, embedded bytes, and the executed build-step inputs and tools. Paths are
keyed in canonical form, so `mach build .` and `mach build /abs/project` share
entries; with `-g` the spelling reaches the line tables and is keyed as well. A
change to any source in the active build cell invalidates that cell's objects:
the key is the whole cell, so an edit misses every module and a rebuild of an
unchanged tree, a reverted edit or a branch switched back hits every module.

The cache is opt-in. A warm hit is about 2.6 times faster than an uncached
build (about 175 ms against 460 ms on a 42-module project), but publishing costs
an uncached build 4 to 7 percent and a cell misses as a whole, so an edit and
rebuild loop would pay the publication on every build and hit on none; the
default flips when a module's key no longer spans the whole cell, which
needs stable module identities in the typed products (#3221). A build without
`--cache` executes no cache code.

`mach build . --no-cache` and `mach test . --no-cache` neither read nor write
persistent object entries or build-step stamps, and force every declared build
step to execute. `-vv` reports each restored image as `cached object` with the
time its restore took, and the cell key's cost as `cell snapshot`.

A missing, corrupt, locked, or inaccessible cache falls back to compilation.
Allocation failures and internal compiler failures remain errors. Each cache
holds at most 512 MiB of logical entry bytes, with at most one additional staged
entry during publication. Individual entries and decoded image owners are limited
to 64 MiB each. Filesystem metadata, allocation-unit overhead, and the compiler's
other live state are separate from those limits. Eviction runs under a short
exclusive lock before a publication and removes the least recently published
entries first (entry mtime, then name), never one the current build restored or
published; a publication that cannot make room without one is declined and the
store is left as it was. Cache operations never fetch dependencies or modify
Git state.

`--pie` is an opt-in: without it, a linux executable links fixed-address
(`ET_EXEC`) exactly as before — a normal build is byte-identical. With it, the
linker emits a position-independent `ET_DYN` image the kernel loads at a
randomized base (ASLR), self-relocated by the runtime before `main` (no `ld.so`).
It applies to a static executable; combining `--pie` with a dynamic `-l<lib>`
dependency is rejected.

`--subsystem` overrides the selected artifact's
[`subsystem`](manifest.md#artifactname) key for this invocation, with the usual
precedence — the flag wins over the manifest, and the manifest over the `console`
default. `gui` writes `IMAGE_SUBSYSTEM_WINDOWS_GUI` into the PE optional header so
the Windows loader starts the process without attaching a console window; `console`
is the default and what mach has always emitted. Only the PE image carries the
field, so passing the flag on a target whose format has none (ELF, Mach-O, a flat
image) is refused as unsupported, naming the flag, the target and the format:
`--subsystem: Subsystem gui is unsupported by elf (target 'host' produces a elf
image, which declares no subsystem)`. The same holds for the manifest key.

### External link inputs

`ext fun` declarations are forward references whose definitions are supplied at
link time by external precompiled code — a loose `.o`/`.obj` object, a static
`.a`/`.lib` archive, or a target-format shared library. Those inputs come from the command line and
from the manifest's matching `[link.X]` entries — an artifact's referenced entries
plus exported dependency entries whose `os`/`isa`/`abi` filters match the build
(see [manifest.md](manifest.md)); both sets are linked. An input that resolves to no
existing file is a hard error, so a typo never silently drops a dependency.

- **Explicit input path** — a bare (non-flag) argument that contains a `/`, ends
  in `.o`/`.obj` (object) or `.a`/`.lib` (archive), or names an ELF `.so`, Mach-O `.dylib`,
  or PE `.dll` is
  treated as an input path. The first non-flag positional after `build` is the
  project root and is skipped; remaining input-path positionals are link inputs.
  A relative path is tried verbatim against the working directory first, then
  rooted at the project root.
- **`-l <name>`** — resolves to an object, archive, or shared library. Each
  `-L <dir>` is searched for the `lib<name>` and `<name>` forms of `.o` and
  `.a`; a PE/COFF target additionally probes `.obj` and `.lib`. If none hit,
  the same target-appropriate candidates relative to the working directory are
  tried. Only if no static
  object or archive is found does resolution fall back to the selected target's
  shared-library spelling: `lib<name>.so[.N]` for ELF or
  `lib<name>[.<N>].dylib` for Mach-O. The `-L` directories are searched first,
  followed by the selected target OS's system library directories; cross-target
  planning does not consume a host library of another format.
- **`-L <dir>`** — adds a search directory for the `-l` resolution above. Both
  `-L` and `-l` may be repeated.

#### Static vs dynamic resolution

How an input resolves decides whether the link is static or dynamic:

- A loose **`.o`/`.obj`** object or static **`.a`/`.lib`** archive is a
  **static** input, merged into the executable at link time. An archive contributes
  only members that define a global currently undefined by the objects and
  members preceding it. Selection repeats within that archive to a fixed point,
  so a selected member may pull a dependency located earlier in the same archive;
  an archive is not revisited after the next input begins. With only static inputs
  the output is a fully static binary, and any
  undefined `ext` that no input defines is a hard error.
- A shared **`.so`**, **`.dylib`**, or **`.dll`** is a **dynamic** dependency.
  The file is read and validated for the selected target before anything is
  recorded: an ELF `.so` that is not a loadable shared object for the target's
  architecture (a linker script such as `INPUT(-lfoo)`, a foreign-arch file,
  a stray text file) is refused (`'<file>' is not a loadable ELF shared object
  for the selected architecture`), and such a file is never picked up as an
  `-l` candidate (`cannot find link input 'foo'`).
  ELF records the library's `DT_SONAME`, Mach-O records its `LC_ID_DYLIB`
  install name, and PE records the DLL basename. When a discovered Mach-O
  install name starts with `@rpath/`, the resolved library directory is retained
  as an `LC_RPATH`; an unresolved explicit `@rpath/...` input is rejected because
  it supplies no usable search directory. Darwin frameworks are declared through
  `[link.X]` with `source = "framework"` and become version-independent system
  framework paths. Undefined `ext` functions are then emitted as imports for the
  target format. A static definition of the same symbol always wins.

Archive members are currently all parsed and validated before selection. An
unused member is never linked, but it must still be a parseable object for the
selected target; keep an archive target-homogeneous. This preserves the existing
archive-parser constraint while changing which members reach symbol resolution.

`-l <name>` prefers a target-appropriate static object/archive over a shared
library, so an `-l name` that has a local object is resolved statically exactly
as before; PE/COFF adds `.obj`/`.lib` to the portable `.o`/`.a` probes, while an
ELF or Mach-O plan ignores those COFF-only bare-name candidates. The `.so` or
`.dylib` fallback only applies when no static candidate exists. A bare `-l`
name is also the logical identity accepted by `#[library("name")]`; manifest
requirements can set a cross-platform logical identity explicitly with their
`library` key. Manifest requirements are resolved before CLI inputs, giving a
stable, deterministic link order.

##### Toolchain runtime archives

A cross-target C binding can expose its toolchain's static runtime through an
ordinary build-step-produced local archive. Materialize the archives into the
project output tree, then list them in linker order:

```toml
[step.mingw-runtime]
argv = ["sh", "tools/materialize-mingw-runtime.sh", "{project.out}/crt/mingw32.lib", "{project.out}/crt/compiler_rt.lib"]
in   = ["tools/materialize-mingw-runtime.sh"]
out  = ["{project.out}/crt/mingw32.lib", "{project.out}/crt/compiler_rt.lib"]
need = []

[link.mingw32]
source = "local"
path   = "{project.out}/crt/mingw32.lib"
os     = "windows"
isa    = "*"
abi    = "*"
export = false

[link.compiler_rt]
source = "local"
path   = "{project.out}/crt/compiler_rt.lib"
os     = "windows"
isa    = "*"
abi    = "*"
export = false
```

The project-owned script should use its configured compiler's supported
discovery/output mechanism and copy or build the exact target archives at those
declared paths. Mach neither searches compiler installations nor relies on a
compiler's private cache layout. Archive definitions such as `memcpy`, `strlen`,
or `vsnprintf` then remain static definitions; do not claim them in a DLL entry's
`symbols`. Only imports left undefined after archive selection need normal
two-level-namespace attribution below.

##### Attributing imports on a two-level namespace

PE and Mach-O record imports **per dependency**: the image names which library
provides each symbol, unlike ELF's flat global search. Every import therefore
needs an attribution, and an unattributed one is a hard link error rather than a
guess. Resolution never opens a library to find out — no export table is read on
any host — so the mapping comes entirely from the link's own declarations, and
cross-linking a Windows PE from Linux needs no target DLL on disk.

Two declarations supply it:

- `#[library("name")]` on an `ext` declaration, for a symbol your Mach source
  declares. See [language/ext-fun.md](language/ext-fun.md#library-attribution).
- `symbols = [...]` on a `[link.X]` entry, written as **source-level** names (the
  target's C symbol prefix is applied by Mach, so one spelling is right on every
  target), for a symbol your source never declares — the Win32 or system references a **static archive leaves
  undefined**. Merging an `.a` pulls in its members' undefined symbols, and those
  have no Mach declaration to decorate, so the providing library is named in the
  manifest instead. See [manifest.md](manifest.md#linkname--link-requirements).

Both write the same attribution, so a symbol may be claimed only once: a
`symbols` entry that contradicts another entry's claim, or a `#[library]`
decorator's, is an error naming both claimants rather than a silent
order-dependent win. The same holds between two `#[library]` decorators: two
`ext` declarations resolving to one link name under different libraries are an
error naming both declarations, so no attribution is decided by module load
order. Declarations under different arms of one `$if` chain are exempt: no target
selects both arms, so they never both apply. Repeating an identical claim is
accepted, which is what an
`export = true` entry reaching a consumer through both the cascade and its own
manifest does, and what two modules declaring the same binding under the same
library do.

On ELF the loader resolves imports by global search, so neither declaration
changes the emitted binary; both are still validated against the link's
dependencies.

Exit codes: `0` ok, `1` user error (missing project path, no `mach.toml`,
unknown target, compile errors, an unresolvable link input), `2` internal error.

## `mach run`

```
mach run <path> [options] [-- args...]
```

Executes the binary `mach build` already produced for `<path>` — a post-build
convenience, **not** a rebuild. The same selection flags (`--target`, `--profile`,
`--bin`) that narrow a build resolve which artifact to run. With no artifact flag,
one is inferred when exactly one artifact declares the resolved target; several
matching artifacts remain an error that asks for `--bin`/`--lib`. Its path is read
from the artifact's `out` template and the existing file is exec'd. When the
artifact does not exist yet, `mach run` errors and points at `mach build` rather
than building it.
Arguments after a `--` separator are forwarded to the child as its `argv`. The
child's exit code becomes this command's exit code.

Only the selection flags apply. A build-only option is rejected by name,
since nothing is built (`option '--emit-ir' is not applicable to 'run'; run
executes an existing artifact`).

| Flag                     | Value   | Effect |
|--------------------------|---------|--------|
| `--runner <cmd>`         | command | execute the binary as `<cmd> <binary> <args...>` instead of directly |
| `--timeout_seconds <n>`  | count   | terminate the run and its process group after `<n>` seconds (default: unbounded) |

`--runner` names a host-side launcher for binaries the host cannot exec
directly — e.g. `mach run . --target windows --runner wine`. `<cmd>` is a single
command name or path (no shell-style word splitting); a bare name is resolved on
`PATH`. Without the flag the binary is exec'd directly, and a launch failure
(such as a foreign-format binary on a host without a binfmt handler) is reported
as a failure — exit `127` when `execve` rejects the binary in the spawned
child — with no auto-detection.

`--timeout_seconds` bounds the run. `<n>` is a positive integer number of
seconds; zero and negative values are rejected, there is no duration grammar,
and omitting the flag leaves the run unbounded exactly as before. When the
bound passes, the program's whole process group is signalled, so a child the
program spawned dies with it rather than outliving the run, and the child is
still reaped. `mach run` then reports the overrun with its bound and exits
`3` — the program produced no exit status of its own, so its own exit code
cannot be forwarded. To make the group killable, the program is spawned into
its own process group when and only when the flag is given.

Exit codes: the child's exit code, `1` on a resolution/user error (including a
missing artifact), `2` on internal error, `3` when the run exceeded
`--timeout_seconds`.

## `mach test`

```
mach test <path> [options]
```

Builds one standalone executable per `test` declaration (the test plus the
project's transitive code, with a synthesized `main` calling just that test),
then spawns each as a separate process, captures its output, and times it.
A test build always links executables, even for a library target.

The readout is live: every all-passing module collapses to a single roll-up
line that prints the moment the module's last test completes, and a module
with failures expands each failing test as it happens:

```
mach.lang.intern      3 ok     568us
mach.lang.driver     27 ok     1 FAIL    146ms
  FAIL  mach.lang.driver.builds:cyclic_import  ./src/lang/driver.mach:142  (exit 1)
    expected a diagnostic, got none

failures:
  mach.lang.driver.builds:cyclic_import  ./src/lang/driver.mach:142  (exit 1)

437 passed, 1 failed, 438 total  (268ms)
```

A roll-up is `<module>  <ok> ok[  <fail> FAIL]  <duration>`. Column widths are
computed from the collected tests before the run (clamped, so one long name
cannot blow out the table); test labels print verbatim, exactly as declared.
Each expanded failure carries the test's `file:line`, its exit code
(`(exit N)`) or signal (`(signal N)`), and the child's captured stdout
indented beneath it; a passing test stays quiet. A crashing test reports its
signal and the run continues. The closing summary re-lists every failure as
`<test>  file:line  (reason)` and reports the run's wall time. `-v` prints a
module header when its first test starts and a line per test as it completes;
`-vv` additionally prints passing tests' captured output. The layout is
fixed-width ASCII (no color, no terminal-width queries).

Only `test` blocks declared in the current project's own modules are collected by
default; tests in dependency modules are excluded unless `--include-deps` is
passed.

The primary artifact is selected as `mach doc` selects it (`--bin`/`--lib`,
else the sole artifact supporting the target, else `default = true`; several
with none marked are refused). A selector that
resolves to nothing fails at planning, before anything builds, with the
selector's own message: `no artifact supports the selected target (bin p1,
bin p1-windows)`, `no bin named 'nosuch'`, `no profile named 'nosuch'`.

| Flag                | Value   | Effect |
|---------------------|---------|--------|
| `--cache`           | —       | reuse and publish persistent object images while compiling tests (opt-in, as for `build`) |
| `--no-cache`        | —       | force a genuinely uncached test build: no persistent object images and no build-step reuse; wins over `--cache` |
| `--jobs <n>`        | count   | run up to `<n>` test processes at once **and** size the build's codegen workers (default: the CPUs available; 1 serializes) |
| `--filter <pattern>`| pattern | run only tests whose name contains `<pattern>` |
| `--include-deps`    | —       | also collect tests declared in dependency modules |
| `--list`            | —       | list the collected tests and exit |
| `--format <mode>`   | `human`\|`json` | output format: the live readout (default `human`), or the machine-readable JSON event stream |
| `--runner <cmd>`    | command | launch every test as `<cmd> <exe> <idx>` instead of exec'ing the dispatcher directly |
| `--timeout_seconds <n>` | count | terminate a test process and its process group after `<n>` seconds (default: unbounded) |

Plus the `build` and global flags (`-v` lists every test). `--emit` is accepted
for compatibility with `mach build` but has no effect — a test build always links
executables. The build produces a
single dispatcher executable covering every collected test; the runner keeps up
to `--jobs` children in flight, each spawned as `<exe> <idx>`, so every test
still runs in its own process. Each child's stdout and stderr are captured to a
per-test file under `log/` beside the dispatcher: a passing test's file is
removed on the spot, a failing test's file stays for inspection (the expanded
failure shows the first 64KB, a `full output:` pointer when that truncates, and
the exact `rerun:` command). Results render in collection order regardless of
completion order, so the readout is deterministic. `--filter` selects at run
time — the built executable is identical regardless of filter. `--runner` has
the same semantics as on `mach run`: a single command name or path (no
shell-style word splitting), resolved on `PATH`, for foreign-target tests the
host cannot exec directly — e.g. `mach test . --target windows --runner wine`
(the runner receives the executable path and the test index as its two
arguments). Without it, a test executable the host cannot launch reports a
per-test failure — `(exit 127)` when `execve` rejects the binary in the spawned
child, `(spawn failed)` when the spawn itself fails — with no auto-detection.

`--timeout_seconds` bounds each spawned test process independently: every test
gets `<n>` seconds of its own, measured from its spawn, not a budget shared
across the run. `<n>` is a positive integer number of seconds; zero and
negative values are rejected, there is no duration grammar, and omitting the
flag leaves every test unbounded exactly as before.

A test that passes its bound is a distinct outcome, neither a failing test nor
an infrastructure failure. Its whole process group is signalled, so a process
the test spawned dies with it, and the test process is still reaped. It renders
as `(timed out after <n>s)`, is counted on the summary line as
`<p> passed, <f> failed (<t> timed out), <n> total`, keeps its captured output
for inspection, and reports `"kind": "timeout"` with its bound in the JSON
stream. It is still a failing test for the exit code — a hung test is the
test's fault, not the machine's — so the suite exits `1`, the same as any
other failure, and `2` only when a test failed for an infrastructure reason.
The distinct count and kind are what make the class visible.

`--format json` replaces the live readout with a machine-readable stream: one
JSON object per line on stdout (`run_start`, one `test` per result, `summary`),
with no human text interleaved. `--list --format json` emits one `case` object
per collected test instead. The schema is versioned and documented in
[tooling/test-json.md](tooling/test-json.md); pin tooling to its `schema`
integer. Build diagnostics stay on stderr, so the stdout stream is clean.

Exit codes: `0` all passed, `1` any failed, `2` build/internal error.

## `mach clean`

```
mach clean <path>
```

Removes every concrete output the manifest declares, under a validated project
root capability, and nothing else: each artifact's linked output, and the
object, test, IR, and assembly trees under each `out` cell, naming what it
removed.

```
removed out/linux-x86_64/debug/bin/p1
removed out/linux-x86_64/debug/obj
```

The now-empty directories above them are left in place. `<path>` names the
project (a directory or a manifest file), like every project-rooted command.

Only the manifest is read: no module graph is loaded and nothing under `dep/`
is touched. Removal is idempotent (`mach clean` on an already-clean project
prints `nothing to clean` and succeeds). The command takes no options.

Exit codes: `0` on success, `1` on a missing project or unparseable manifest, `2`
on an allocator or io failure.

## `mach fmt`

```
mach fmt <path> [--check]
```

Formats every `.mach` file under the manifest's `project.src`, including
subdirectories, in the one canonical layout described below, and prints the
path of each file it rewrote. With `--check` it prints the path of each file
whose layout differs and writes nothing: no file, no publication lock, no
claim, no staging entry. `<path>` names the project (a directory or a manifest
file), like every project-rooted command.

Only the manifest is read: no module graph is loaded, no build step or
dependency step runs, and nothing under `dep/` is read or fetched. The source
directory is reached one component at a time through held directory
capabilities, so a symlinked component or file is refused rather than
followed, and a `project.src` that resolves to the dependency tree by any
spelling is refused by identity. Directories are visited in name order.

A file that does not parse is reported through the ordinary located
diagnostics (`path:line:col: error: ...`) and left byte for byte as it was;
the walk continues to the next file and the exit status is `1`. A file whose
layout is already canonical is not touched. A rewrite goes through the
publication boundary every compiler output uses (`std.filesystem.transaction`
under `mach.lang.publication`): the canonical text is staged beside the file
and renamed over it in one step, the rename is refused when the file is no
longer the object that was read, and the file's permission bits are kept.
Before any rewrite the canonical text is parsed again and compared with the
original for meaning (the check is described under the layout below), so the
formatter never writes a file that means something other than its source.

Exit codes: `0` when every file is canonical or was made so, `1` when a file
differs under `--check`, a file is malformed, or the invocation is a user
error, `2` on an internal failure, `3` on a filesystem failure.

### The layout

There is exactly one layout and no configuration: no option, no file, no
comment directive selects another. The formatter re-emits the file's tokens
from the parse. It never changes a token, and it reorders and removes tokens
in exactly two places, both without changing meaning: a `use` block is
sorted, and a `use` alias that repeats the last segment of its path is
dropped. Comments and inline assembly are carried verbatim, and formatting is
a fixed point (formatting a formatted file changes nothing).

Line structure:

- Indentation is four spaces per enclosing `{` block; tabs are never emitted.
  Output lines are terminated by `\n` alone, carry no trailing whitespace,
  and the file ends with exactly one newline. A file with no tokens and no
  comments is empty.
- A function or `test` body always expands: its `{` ends its line, its
  contents are indented one level, and its `}` starts a line at the enclosing
  indentation. An empty body is `{}` on one line.
- A statement body (an `if`, `or`, `for` or `fin` block, a comptime branch,
  a `$each` body) that holds exactly one statement and that the author wrote
  on one line stays on one line: `if (neg) { ret false; }`. A body with two
  or more statements, a nested block, a comment, a decorator or an assembly
  body always expands, even when the author wrote it on one line. A
  single-statement body the author broke across lines stays expanded: the
  author's break is kept.
- A `rec`, `uni` or `tag` body, declared or inline in a type, stays on one
  line when the author wrote it on one line and expands otherwise.
- `or` always starts its own line, at the indentation of its `if`; `} or {`
  is never emitted. An expanded chain is `}` on one line and `or (...) {` on
  the next; an inline chain puts each arm on its own line.
- Every `}` that closes a body ends its line. A `;`, `)`, `]`, `,`, `.` or an
  infix operator after a `}` stays on its line.
- Every `;` ends its line, as does a decorator (`#[...]`), a comment and an
  inline assembly body.
- A top-level declaration whose body expands (`fun`, `test`, a multi-line
  `rec`, `uni`, `tag` or top-level `$if`) is separated from its neighbours by
  one blank line. Between single-line declarations (`use`, `fwd`, `val`,
  `var`, `def`, `ext fun`, a one-line type declaration or `$if`) the author's
  grouping is kept: at most one blank line where the source had one. Between
  statements, and between the elements of a literal, one blank line is
  likewise kept where the source had one; runs of blank lines collapse to
  one, and a blank line directly after a `{` or before a `}` is dropped.
- A struct, array or vector literal stays on one line unless the author broke
  the line after its `{`; then its `{` ends the line, its elements are
  indented one level, and its `}` starts a line. Between elements a line
  break is kept where the source has one.
- A line break the author placed inside an expression, a parameter list, a
  call or a condition is kept, and the continuation line is indented one
  level past the statement. A `)` or `]` that the author placed on its own
  line after a trailing comma keeps its line, at the statement's indentation.
  Line length is not a rule: nothing is wrapped and nothing is joined that the
  author broke.

Spacing:

- One space around binary operators and `=`, after `,`, `:`, `;` and a
  keyword, before a body's `{`, and between a type and the name it follows;
  more where column alignment (below) pads a line.
- No space inside `(`, `[`, `#[` or a literal's `{}`, before `,`, `;`, `:`,
  `.`, `)`, `]`, or after a prefix operator (`-x`, `!b`, `~m`, `?p`, `@p`),
  a pointer or secret type marker (`*T`, `^T`, `*^T`), or `$` (`$if`,
  `$cases(T)`). An inline body is spaced inside its braces: `{ ret; }`.
- Casts and declassification are tight: `x::u8`, `x:~T`, `x:>u32`. A `^` after
  `:` keeps its space (`x: ^u32`), since `:^` is a token of its own.
- A call, index or generic argument list is tight to what it applies to:
  `f(x)`, `a[i]`, `res[T, E].err{e}`, `vector.push[u8](?v, b)`, `f()()`. An
  array type is spaced before and tight after: `x: [4]u8`, `ret [2]i32{1, 2}`.
  The keywords `if`, `or`, `for` and `ret` are spaced before `(`.
- A spread is tight (`args...`); a variadic parameter is spaced (`va: ...`).
- Tag construction, case tests and projections are tight: `Fail.user{m}`,
  `sel f.user`, `sel f.[c]`, `v.[f]`.

Column alignment:

Mach's one-line declarations and statements share one skeleton,
`[keywords] name[: type] [= value]`, and the formatter aligns the columns of
that skeleton across a run. A run is a sequence of consecutive lines of one
shape at one indentation; a blank line, a comment line, a line of another
shape, a continuation line or a line at another indentation ends it. Within
a run each column is padded with spaces to the longest key before it in the
run plus one space. A run of one line takes single spaces. Padding is spaces
only, counted in characters, and only ever widens a line: nothing else about
the line changes. The shapes and their columns:

| Shape | Columns |
|-------|---------|
| `[pub] val\|var NAME: Type [= init];` | the name (after the keywords), the type, the `=` |
| `[pub] def NAME: Type;` | the name, the type |
| `use alias: path;`, `fwd alias: path;` | the alias, the path |
| a `rec`/`uni` field or a `tag` case with a payload, `name: Type;` | the type |
| an assignment statement `place = value;` | the `=` |
| a named entry of an expanded literal, `name: value,` | the value |
| an inline `if`/`or`/`$if`/`$or` arm | the body `{` |
| a one-line `rec`/`uni`/`tag` declaration | the name, a tag's discriminator type, the body `{` |
| a doc comment `# name: description` | the description |

A `use` or `fwd` without an alias, a bare tag case and a positional literal
element have no column and end a run. `val` and `var`, and `pub` and
unqualified, are one shape: the keywords differ in length and the name column
absorbs the difference. Only whole lines are aligned: the assignment inside
`if (x) { a = 1; }` is spaced, not padded. A doc comment is a comment of the
form `# name: description` on its own line; a run of them aligns the
descriptions, and a comment of `#` followed by two or more spaces directly
under one is a continuation of its description and is re-padded to the
description column. Every other comment is verbatim.

`use` blocks:

- A `use` alias that repeats the last segment of its path is dropped:
  `use str: std.types.string.str;` becomes `use std.types.string.str;`. The
  binding is the same name either way. An alias that differs from the last
  segment is kept.
- A block is a run of `use` lines with nothing between them: no blank line
  and no comment line. Within a block the lines are sorted by the path text
  in byte order, after alias dropping; lines with equal paths keep their
  order. Blank lines between blocks are kept and blocks are sorted
  independently. A `use` under a doc comment is pinned where it is, so the
  comment never moves: it ends the block above it, and the lines under it
  form a new block.

A worked example, excerpts of `src/lang/fe/sema/guard.mach`,
`src/lang/target/abi/sysv.mach` and std's `collections/vector.mach` as the
formatter leaves them:

```
use mach.lang.alloc;
use A: std.allocator;
use std.format;
use std.text.string.str_free;

pub rec Vector[T] {
    a:    *A.Allocator;
    data: *T;
    len:  usize;
    cap:  usize;
}

        var neg:       bool         = false;
        var arm_place: id.ExprId    = id.EXPR_NIL;
        var arm_case:  intern.StrId = intern.STR_NIL;
        if (!sel_test(sc, s.data.if_.cond, ?neg, ?arm_place, ?arm_case)) { ret false; }
        if (place == id.EXPR_NIL)                                        { place = arm_place; }
        or (!place_equal(sc, place, arm_place))                          { ret false; }

        if (eb0_used) {
            if (eb0_sse) { fp_need = fp_need + 1; }
            or           { gp_need = gp_need + 1; }
        }
        if (sel e_opt.err) {
            context.report(sc, span, fallback);
            ret false;
        }
```

The `use` block is sorted by path (`mach.lang.alloc` before `std.format`)
with `format: std.format` de-aliased; the record's fields align on the type
column; the three `var` lines align on their type and `=` columns and the
`if` that follows, a different shape, starts a new run with the two arms
under it; the inner `if`/`or` chain aligns its braces with `or` on its own
line; and the last body, which held two statements on one line in the
source, expands.

Comments and inline assembly:

- A comment is carried verbatim from its `#` to the end of its text, with
  trailing whitespace dropped, except that a doc comment's description is
  re-padded as described above. A comment that shares a line with code stays
  on that line after one space; a comment on its own line stays on its own
  line at the current indentation. A doc run (own-line comments directly
  above a declaration, with no blank line between) stays attached to that
  declaration, and a comment separated from the declaration below by a blank
  line stays separated.
- An inline assembly body is carried verbatim from its `{` to its `}`,
  including its line breaks and indentation. Its language has no formatting
  contract of its own, so it is never reflowed.

The meaning check before a write compares the two texts as token sequences
with the same comments, the same tree shape and the same doc runs, where a
`use` block compares as the set of its lines, a dropped alias as no alias,
and a doc comment by its key and description; so the two places the
formatter moves or drops a token are exactly the two places the check
allows a difference.

## `mach dep`

```
mach dep <action> <path> [args]
```

Realizes, verifies, and changes the project's dependencies under `dep/`.
Dispatches on `argv[2]`. The model it operates is documented in
[manifest.md](manifest.md#depid): a dependency is named by its project id in
the manifest key, the directory, and source alike; a `git` dependency is a git
**submodule** at `dep/<id>/` whose committed gitlink is the pin; a `path`
dependency is copied in as ordinary files; the root's `dep/` holds the whole
transitive closure one level deep; and there is no lock file.

| Action   | Args | Effect |
|----------|------|--------|
| `pull`   | `<path>` | restore existing Git dependencies to their recorded gitlinks and initialize empty gitlink checkouts. Realize missing declared dependencies, cloning Git sources or copying path sources as needed. Retain existing path copies. Use `update` to refresh them from their sources. |
| `verify` | `<path>` | run the build's dependency checks as a command, closure and selectors included, and print `ok`, or the first failure. A `mach.lock` in the root is refused here as everywhere else. |
| `add`    | `<path> <name> (--git <url> [--ref <ref>] \| --path <dir>)` | validate the candidate declaration, realize its dependency closure, then publish `[dep.<name>]` in `mach.toml`. Git stages `.gitmodules` and gitlinks. Nothing is committed. |
| `update` | `<path> (<name> \| --all)` | advance `branch/` selectors to their current remote tips and re-stage the gitlinks; move an identity to the exact selector the root declares for it (`b: <old> -> <new> (pinned to the exact selector)`, or `(exact selector, already pinned)` when nothing moves). |
| `remove` | `<path> <name> [--purge]` | remove a Git dependency’s registration from the index and `.gitmodules` when no longer required, then publish the manifest without its declaration. The checkout is retained unless `--purge` is given. |
| `list`   | `<path>` | print each realized dependency with its source, selector, pinned commit, and state (`realized`/`missing`). |
| `sync`   | `<path>` | the pre-`pull` name, kept as a deprecated alias of `pull`; it runs `pull` exactly. |

Dependency changes use Git's normal submodule and index operations. Validation
rejects conflicts that can be determined before those operations begin. A remote
fetch, checkout, newly discovered transitive conflict, or manifest publication can
still fail after an earlier operation succeeds. Mach stops, reports the failed
operation and completed work, and leaves Git's state available for inspection.
It does not restore an earlier index or recursively erase partial checkouts.
Use `git status` and inspect the named dependency before retrying.

`mach.toml` is published only after the requested dependency operations succeed.
Its contents are replaced atomically with the complete old or new file. An error
can occur after replacement, so inspect `mach.toml` when publication reports an
error. Completed Git operations remain. Concurrent Mach manifest
edits are serialized. Git provides its own locking for each Git operation.
Directories outside the resulting closure are reported and retained for explicit
removal.

Every action requires its project directory or manifest path as the first
positional operand, resolved by the same rules as `mach build <path>`. Write `.`
for the current project. A dependency name follows the project path, for example
`mach dep update ../app widget`. Relative local dependency sources are resolved
from the selected project's manifest directory. Extra operands, missing option
values and repeated source options are refused.
`--quiet`/`-q` suppresses routine output on every action.

```
$ mach dep list .
  std  git=https://github.com/briar-systems/mach-std  ref=branch/main  pin=74ce8f4e65943172274f523e6bbe3a638ae3fadc  realized
$ mach dep verify .
ok
```

### Selectors

`--ref` on `add` and `ref` in the manifest take exactly three forms:
`branch/<name>`, `tag/<name>`, and `commit/<full-object-id>`. A bare name, a
short id, or an empty ref is rejected (`[dep.std].ref must be branch/<name>,
tag/<name>, or commit/<full-object-id>`). `--ref` is valid only with `--git`;
`--path` forbids it.

### Transport policy

`mach build` never requires the network: a project whose `dep/` is realized
builds offline, and the build never fetches and never writes under `dep/`.
A project and its local path dependencies do not need a Git repository or index.
Git dependencies require **git** for offline checkout and pin verification. Only
`pull`, `update`, and `add` reach the network, through git
discovered on `PATH` and invoked with an allowlisted environment (`PATH`,
`HOME`, the common git/ssh/proxy/CA variables, and
`GIT_CONFIG_COUNT`/`GIT_CONFIG_KEY_n`/`GIT_CONFIG_VALUE_n`, which is how a
local fixture enables git's file transport: `GIT_CONFIG_COUNT=1
GIT_CONFIG_KEY_0=protocol.file.allow GIT_CONFIG_VALUE_0=always`). Git's
absence is an environmental error, exit `3`.

For a git dependency, realization is `git submodule add` (on `add`) or a
detached checkout at the committed gitlink (on `pull`); `.gitmodules` is
generated from the manifest and never read for pins. No git command ever runs
against a gitlink that is not yet a checkout of its own: an empty directory a
plain `git clone` leaves for a submodule is initialized in place by `pull`. A
checkout that is not at its gitlink, or that has uncommitted changes, fails
verification (`dependency 'std': checkout is dirty:  M mach.toml`);
realization must be a physical directory, not a symlink. A project without its
own Git repository realizes Git dependencies as plain clones and verifies those
checkouts directly. Dependency commands never create the project repository,
create commits, move project branches, or stage `mach.toml` or unrelated files.

For a **path** dependency, `add --path <dir>` copies the local source files
into `dep/<name>/`, without the source's own `dep/` or Git metadata, and refuses
a source that escapes through a symlink. Copied files are not automatically
staged. Verification checks the filesystem realization and does not require it
to be tracked in Git. The `path` is resolved relative to the requiring manifest's
directory. `pull` copies a missing path dependency and retains an existing copy.
`update` refreshes the copy from the source. Before copying, Mach inventories
source files and excludes the destination itself when the source is an ancestor.
An existing destination entry absent from that inventory, or with a conflicting
type, requires explicit cleanup before the update can proceed. Mach does not
delete extra files. A copy failure retains completed writes for inspection.

### The closure and clashes

`add` recomputes the root's transitive closure from the realized manifests and
realizes every identity in it directly under `dep/`: declaring `a`, whose
manifest declares `b`, realizes `dep/a/` and `dep/b/` (`realized b @ <commit>`),
and `a`'s own `use b.*` resolves against the root's `dep/b/`. A consumed
dependency's own `dep/` is never initialized: the empty `dep/a/dep/b/` git
leaves behind is the materialization of `a`'s gitlink, and it is neither
realized, verified, nor descended into.

One identity resolves to one commit per closure. When two requirers select
different commits for one id, `add` stops and prints both chains and the
root declaration that would decide (the exact text is in
[manifest.md](manifest.md#one-identity-one-commit)); the root resolves it by
declaring the identity with its own `ref`, at upstream or at a fork, and
`mach dep update <path> <id>` re-pins the realized checkout to that declaration.
Verification then holds every requirer's exact selector against the realized
commit (`dependency 'b': exact ref 'tag/v1.0.0' resolves to '<commit>' but the
realized commit is '<other>'; run `mach dep update <path> b` to re-pin it, or declare
the identity at the root to override`), except for an identity the root
declares with a `tag/`, whose gitlink is the pin; the rules are in
[manifest.md](manifest.md#what-a-build-verifies).

### Removed forms

A `[dep.<key>]` whose realized project declares a different id is an **alias
key**, removed in 5.0.0: `pull`, `verify` and every build refuse it
(`[dep.foo] realizes project 'std'; alias keys were removed in 5.0.0: the
manifest key, the directory under dep/, and the project id are one name, so
rename the table to [dep.std] and the directory to dep/std`); `pull` rolls the
refused realization back. A realized `dep/<id>/dep/<x>/mach.toml` (a **nested
realization** left by an older tool) is refused the same way, naming the
directory to delete; the empty directory git materializes for a consumed
dependency's own gitlink is not a realization and passes. A `mach.lock` in the
project root is refused by every command that opens the project
(`mach.lock was removed in 5.0.0 and is refused; the committed gitlinks under
dep/ are the pins: delete mach.lock`).

Exit codes: `0` ok, `1` user error, `2` internal error, `3` environmental
error (git missing or a git operation that failed).

## `mach init`

```
mach init [dir] [options]
```

Scaffolds a new project in `[dir]` (default: the current directory). It
writes a complete `mach.toml` with a `[project]` block, `[target.*]` platforms
for `linux`/`windows`/`darwin` on the host ISA, one binary artifact whose
`out = "bin/<id>{artifact.suffix}"` names `<id>.exe` on Windows and `<id>`
elsewhere (or, under `--lib`, one `static` library artifact,
`lib/lib<id>{artifact.suffix}`, marked `default = true` so a bare
`use <id>;` in a consumer binds its entry), a `[link.kernel32]` entry the
binary links on Windows,
`[profile.debug]` (`default = true`) and `[profile.release]` with all five
policy keys spelled out, and a `[dep.std]` dependency on `mach-std` at
`branch/main`; then a starter source file, `src/root.mach` for a
binary (`use std.runtime; use print: std.print;` and a `#[symbol("main")]`
entry) or `src/lib.mach` for a library; then, as a separate stage, it
initializes the directory as a git repository if it is not one and realizes
`dep/std` exactly as `mach dep pull` would, staging `.gitmodules` and the
gitlink:

```
$ mach init p1 --name p1
created project 'p1'
realized dependency 'std'
```

The compiler emits no entry point or startup code of its own: `std.runtime`
supplies `_start`, and the scaffold's `main` is an ordinary function that
exports the `main` symbol.

| Flag           | Value | Effect |
|----------------|-------|--------|
| `--name <id>`  | id    | project id (default: the last path component of `[dir]`, so `mach init /work/ia`, `mach init ib/`, and `mach init .` name the project `ia`, `ib`, and the current directory's name) |
| `--force`      | —     | scaffold even when `mach.toml` or `src` already exists |
| `--lib`        | —     | library layout: `src/lib.mach` and one `static` `[artifact.<id>]` marked `default = true` |
| `--no-deps`    | —     | publish the scaffold and declare its dependencies without realizing them; a later `mach dep pull` realizes them (`dependencies declared but not realized; run `mach dep pull <path>` to realize them`) |
| `--no-git`     | —     | skip repository initialization and submodule registration, using plain dependency checkouts instead |
| `--quiet`, `-q`| —     | suppress non-error output |

By default, `mach init` initializes Git when the destination is outside an
existing worktree. It does not commit files or change existing history.
`--no-git` skips initialization and submodule registration. Combine it with
`--no-deps` to scaffold without invoking Git. `--no-deps` alone still allows
repository initialization.

The first non-flag argument after `init` is the target directory. Scaffolding
into a directory that already holds unrelated files keeps them; an existing
`mach.toml` is refused without `--force` (`mach.toml already exists (use
--force to overwrite)`), and every collision is checked before any file is
written, so a refused init leaves nothing behind. Files are written through
the same atomic publication path as build outputs. A realization failure
leaves the scaffold and completed Git operations available for inspection, with a diagnostic identifying the failed step.

`mach init` scaffolds a buildable project directly: for a default binary
scaffold, `mach build .` then `mach run .` prints `Hello, World!` without
further manifest edits.

Exit codes: `0` ok, `1` user error, `2` internal error, `3` environmental
error (git or the network was unavailable during realization).

## `mach doc`

```
mach doc <path> [options]
```

Loads the project's module graph and generates Markdown reference docs from
source docstrings — one page per module, for the project and its
dependencies, plus an index. Every `pub` declaration is rendered whether or
not it is documented; a documented one carries the docstring attached to it
(see [language/documentation.md](language/documentation.md)). Pages are
written through the atomic publication path. The hand-written `doc/`
material is never touched.

```
$ mach doc .
documented 613 public entities across 36 modules -> /home/me/p1/doc/api
```

| Flag             | Value | Effect |
|------------------|-------|--------|
| `--out <dir>`    | dir   | output directory: a relative path is rooted at the project, an absolute path is used as given (default `doc/api`) |
| `--target <name>`| name  | select a `[target.<name>]` for module discovery |
| `--quiet`, `-q`  | —     | suppress non-error output |
| `--bin <name>`   | name  | document one binary artifact |
| `--lib <name>`   | name  | document one static or shared library artifact |

The artifact is selected the way `mach build` selects it: an explicit
`--bin`/`--lib` wins, otherwise the sole artifact that supports the target,
otherwise the one marked `default = true`. When several support the target
and none is marked, the command refuses (the 4.30 first-declared fallback was
removed in 5.0.0; table order carries no meaning):

```
error: mach.toml: several artifacts support the selected target and none is marked `default = true`; no artifact is selected by table order: mark exactly one [artifact.<name>] with `default = true` or select one with --bin/--lib (first, second)
```

Exit codes: `0` ok, `1` user error, `2` internal error.

## `mach info`

```
mach info [--version | targets]
```

Prints an at-a-glance identity of the binary: its version, the host (`os/isa`)
it was built for, and the registered capability surface — the instruction sets,
operating systems, ABIs, and object formats it can compose into a target. This
needs no project (it runs from anywhere, with or without a `mach.toml`). The
output is line-oriented and stable for scripts:

```
mach 5.0.0
host: linux/x86_64
isa: x86_64 aarch64 riscv64 riscv32 spirv
os: linux darwin windows freestanding
abi: sysv64 win64 aapcs64 lp64 lp64f lp64d ilp32 ilp32f ilp32d spirv
object: elf coff macho raw spv
```

The version line (the compiler's own version string) and `host:` line fold at
compile time; the four capability lines are read from the binary's target
registries, so they report exactly what this build can target. `mach info --version` prints the version string alone
on one line, for tooling.

`mach info targets` prints the **supported target-tuple matrix** — one
`<os>-<isa>` per line — for exactly the tuples this binary can compose and emit
end-to-end. Run the command to see the current set; it is *derived*, never
curated, so a snapshot printed here would only drift.

Each dimension is orthogonal on its own, but the joint cells are not: an
instruction set emits only with a wired code generator, a calling convention is
per-ISA, an object format relocates and writes only the ISAs it declares, an
operating system runs on only the ISAs it was ported to and links and loads only
its own object formats, and an object format's emission shape must match the
instruction set's back half — a whole-module emitter needs a format that carries
finished modules, a register machine one that carries linkable objects. `mach info
targets` keeps a `<os>-<isa>` tuple only when some registered calling convention and
object format compose an emittable full tuple — so `windows-aarch64` is absent (COFF
covers x86-64 only) and `darwin-riscv64` is absent (Mach-O covers x86-64 and
aarch64), while freestanding tuples appear for every ISA with an encoder.
Selecting an uncovered tuple fails at composition naming the missing capability
(for example `object format 'coff' does not cover aarch64 relocations` or
`operating system 'windows' does not support object format 'elf'`) rather than
deep in codegen or link. Adding a capability declaration to a vtable is the only
step needed for a new tuple to appear.

| Argument    | Value | Effect |
|-------------|-------|--------|
| `--version` | —     | print the version string alone, on one line |
| `targets`   | —     | print the supported target-tuple matrix, one `<os>-<isa>` per line |

Exit codes: `0` ok, `2` internal error.

## `mach help`

```
mach help [command]
```

Prints the top-level usage summary, or — with a known `[command]` — that
command's detail page: its options with their defaults, aliases, and
implications, its constraints, and its exit codes, rendered from the schema
the parser consumes. `mach <command> -h` prints the same page, and bare
`mach --help` the summary. An unknown command prints the top-level usage and
exits `1`.

## See also

- [manifest.md](manifest.md) — the `mach.toml` reference
- [language/test.md](language/test.md) — the `test` declaration and `mach test`
- [tooling/test-json.md](tooling/test-json.md) — the `mach test --format json` event schema
- [language/files.md](language/files.md) — project file layout
