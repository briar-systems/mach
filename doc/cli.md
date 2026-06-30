# `mach` — the command-line interface

```
mach <command> [options]
```

The compiler dispatches on `argv[1]`. With no command, or an unknown one, it
prints usage and exits non-zero. The project commands — `build`, `run`, `test`,
and `doc` — take the project root as a **required** positional (`mach build
<path>`) and walk up from it until a `mach.toml` is found; a bare invocation with
no path is a user error.

This page documents the flags the current binary actually parses. Flags are
matched exactly: `--flag value` (a value follows in the next argument) or a bare
`--flag` toggle. The combined `--flag=value` form and bundled short flags are
**not** recognized.

## Commands

| Command | Summary |
|---------|---------|
| `build` | compile the project to objects and (for a `[bin.*]`) a linked binary |
| `run`   | execute the already-built binary (a post-`build` convenience, not a rebuild) |
| `test`  | build the test binary and run the collected tests |
| `dep`   | manage git-backed dependencies (clone, lock, vendor) under `<dep>` |
| `init`  | scaffold a new project |
| `doc`   | generate Markdown reference docs from source doc-comments |
| `info`  | print compiler version, build host, and registered target capabilities |
| `help`  | print usage; `mach help <command>` for detail |

## Global flags

Read by `build`, `run`, `test`, and `doc` (they share one config parser).
`--verbose` and `--quiet` together is a parse error.

| Flag             | Value            | Effect |
|------------------|------------------|--------|
| `--verbose`, `-v`| —                | write extra detail (per-stage compile progress) to stderr |
| `--quiet`, `-q`  | —                | suppress non-error output |
| `--color <mode>` | `auto`\|`always`\|`never` | color preference for terminal output (default `auto`); an unknown mode is a parse error |
| `--target <name>`| target name      | select a declared target; absent, defers to `[project].target` |
| `--profile <name>`| profile name    | select a `[profile.<name>]` build variant; absent, the first declared profile |
| `--bin <name>`   | artifact name    | narrow the build to one `[bin.<name>]` artifact |
| `--lib <name>`   | artifact name    | narrow the build to one `[lib.<name>]` artifact (mutually exclusive with `--bin`) |
| `-o <path>`      | path             | override the artifact path, rooted at the project root (build/run/test) |
| `--all-targets`  | —                | build every declared `[target.*]`, not just the default |
| `--emit-asm`     | —                | emit per-module assembly text (`.s`); forces the selected profile's `emit_asm` on |
| `--emit-ir`      | —                | emit per-module SSA IR text (`.ir`); forces the selected profile's `emit_ir` on |
| `--no-emit-asm`  | —                | force per-module assembly emission off, overriding the profile's `emit_asm` |
| `--no-emit-ir`   | —                | force per-module IR emission off, overriding the profile's `emit_ir` |
| `--verify-ir`    | —                | run the IR verifier after each optimisation pass |

> `mach dep` and `mach init` do not use the shared config parser; they read only
> their own flags listed below.

## `mach build`

```
mach build <path> [options]
```

Compiles the project rooted at `<path>` (e.g. `mach build .`). With no
`--bin`/`--lib`, it builds every declared
artifact for the default target and profile. Every reachable module is driven
through sema → lower → optimise → codegen to one relocatable object, written
under the manifest's resolved `obj` template at `<obj>/<fqn-as-path>.o`. For a
`[bin.*]` the objects are linked into the resolved artifact path (the `out`
template); for a `[lib.*]` (or with `--emit obj`) the objects are the deliverable
and nothing is linked.

| Flag           | Value          | Effect |
|----------------|----------------|--------|
| `--release`    | —              | select the release optimisation pipeline |
| `-O0`          | —              | force the debug pipeline (overrides `--release`) |
| `-O1`          | —              | select the release pipeline |
| `-O2`          | —              | select the release pipeline |
| `--emit <kind>`| `obj`\|`exe`   | `obj` stops at the relocatable objects; `exe` (default) links a binary |
| `--pie`        | —              | emit a position-independent (ET_DYN) executable for ASLR instead of the default fixed-address one; opt-in (see below) |
| `-L <dir>`     | dir            | add a search directory for `-l`-resolved inputs; repeatable |
| `-l <name>`    | name           | link a named object, archive, or shared library, resolved through the `-L` dirs (see below); repeatable |
| *(positional)* | input path     | a bare argument that contains `/`, ends in `.o` / `.a`, or names a `.so` is linked verbatim |

Plus the global flags above. The `-O<n>` flags override `--release` when both
are present; absent any optimisation flag the build uses the debug pipeline
(the bootstrap-stable default). `-O1` and `-O2` currently select the same
release pipeline.

`--pie` is an opt-in: without it, a linux executable links fixed-address
(`ET_EXEC`) exactly as before — a normal build is byte-identical. With it, the
linker emits a position-independent `ET_DYN` image the kernel loads at a
randomized base (ASLR), self-relocated by the runtime before `main` (no `ld.so`).
It applies to a static executable; combining `--pie` with a dynamic `-l<lib>`
dependency is rejected.

### External link inputs

`ext fun` declarations are forward references whose definitions are supplied at
link time by external precompiled code — a loose `.o` object, a static `.a`
archive, or a shared `.so` library. Those inputs come from the command line and
from the manifest's merged `libs` overlay (`[target.*]`, `[bin.*]`/`[lib.*]`, and
per-cell, see [manifest.md](manifest.md)); both sets are linked. An input that
resolves to no
existing file is a hard error, so a typo never silently drops a dependency.

- **Explicit input path** — a bare (non-flag) argument that contains a `/`, ends
  in `.o` (object) or `.a` (archive), or names a `.so` (shared library) is
  treated as an input path. The first non-flag positional after `build` is the
  project root and is skipped; remaining input-path positionals are link inputs.
  A relative path is tried verbatim against the working directory first, then
  rooted at the project root.
- **`-l <name>`** — resolves to an object, archive, or shared library. Each
  `-L <dir>` is searched for `<dir>/lib<name>.o`, `<dir>/<name>.o`,
  `<dir>/lib<name>.a`, then `<dir>/<name>.a`; if none hit, the same four
  candidates relative to the working directory are tried. Only if no static
  object or archive is found does resolution fall back to a shared
  `lib<name>.so` (searched in the `-L` dirs, the target OS's default library
  directory, then the common system library directories `/lib64`, `/usr/lib64`,
  the multiarch dirs, `/usr/lib`, `/lib`).
- **`-L <dir>`** — adds a search directory for the `-l` resolution above. Both
  `-L` and `-l` may be repeated.

#### Static vs dynamic resolution

How an input resolves decides whether the link is static or dynamic:

- A loose **`.o`** object or static **`.a`** archive is a **static** input,
  merged into the executable at link time. An `.a` contributes every one of its
  member objects (all members are pulled, not just those satisfying an undefined
  symbol). With only static inputs the output is a fully static binary, and any
  undefined `ext` that no input defines is a hard error.
- A shared **`.so`** library is a **dynamic** dependency. Its `DT_SONAME` (read
  from the library, e.g. `libc.so.6` for `-l c`) is recorded as a run-time
  dependency, and any undefined `ext` left after merging is bound against it at
  load time through a PLT the linker emits — producing a dynamically-linked ELF
  with a `PT_INTERP` (the OS dynamic loader) and a `.dynamic`/PLT/GOT. A static
  definition of a symbol always wins over a dynamic import of the same name.

`-l <name>` prefers a static `.o`/`.a` over a shared `.so`, so an `-l name`
that has a local object is resolved statically exactly as before; the `.so`
fallback only applies when no static candidate exists (the common case for
system libraries like libc). Manifest `libs` are resolved before the CLI inputs,
giving a stable, deterministic link order.

> Dynamic linking is implemented for the ELF (Linux) and PE (Windows) targets;
> the Mach-O (Darwin) import path is not yet implemented (#1176).

Exit codes: `0` ok, `1` user error (missing project path, no `mach.toml`,
unknown target, compile errors, an unresolvable link input), `2` internal error.

## `mach run`

```
mach run <path> [options] [-- args...]
```

Executes the binary `mach build` already produced for `<path>` — a post-build
convenience, **not** a rebuild. The same selection flags (`--target`,
`--profile`/`--release`, `--bin`) that narrow a build resolve which artifact to
run; its path is read from the manifest's `out` template and the existing file
is exec'd. When the artifact does not exist yet, `mach run` errors and points at
`mach build` rather than building it.
Arguments after a `--` separator are forwarded to the child as its `argv`. The
child's exit code becomes this command's exit code.

`--emit` is rejected — running a relocatable object is not meaningful. The
`build` selection and global flags apply; build-only flags (`-O*`, `--emit-*`)
are accepted but have no effect, since nothing is built.

| Flag             | Value   | Effect |
|------------------|---------|--------|
| `--runner <cmd>` | command | execute the binary as `<cmd> <binary> <args...>` instead of directly |

`--runner` names a host-side launcher for binaries the host cannot exec
directly — e.g. `mach run . --target windows --runner wine`. `<cmd>` is a single
command name or path (no shell-style word splitting); a bare name is resolved on
`PATH`. Without the flag the binary is exec'd directly, and a launch failure
(such as a foreign-format binary on a host without a binfmt handler) is reported
as a failure — exit `127` when `execve` rejects the binary in the spawned
child — with no auto-detection.

Exit codes: the child's exit code, `1` on a resolution/user error (including a
missing artifact), `2` on internal error.

## `mach test`

```
mach test <path> [options]
```

Builds one standalone executable per `test` declaration (the test plus the
project's transitive code, with a synthesized `main` calling just that test),
then spawns each as a separate process, captures its output, and times it.
A test build always links executables, even for a library target.

The default readout collapses every all-passing module to a single roll-up
line and expands only modules that have a failure:

```
mach.lang.intern                     3 ok  568us
mach.lang.driver                    27 ok     1 FAIL  146ms
  FAIL  builds:cyclic_import  ./src/lang/driver.mach:142  (exit 1)
    expected a diagnostic, got none

failures:
  mach.lang.driver.builds:cyclic_import  ./src/lang/driver.mach:142  (exit 1)

437 passed, 1 failed, 438 total  (268ms)
```

A roll-up is `<module>  <ok> ok[  <fail> FAIL]  <duration>`. Each expanded
failure carries the test's `file:line`, its exit code (`(exit N)`) or signal
(`(signal N)`), and the child's captured stdout indented beneath it; a passing
test stays quiet. A crashing test reports its signal and the run continues. The
closing summary re-lists every failure as `<test>  file:line  (reason)`. `-v`
lists every test, grouped by module, with each test's duration. The layout is
fixed-width ASCII (no color, no terminal-width queries).

Only `test` blocks declared in the current project's own modules are collected by
default; tests in dependency modules are excluded unless `--include-deps` is
passed.

| Flag                | Value   | Effect |
|---------------------|---------|--------|
| `--filter <pattern>`| pattern | run only tests whose name contains `<pattern>` |
| `--include-deps`    | —       | also collect tests declared in dependency modules |
| `--list`            | —       | list the collected tests and exit |
| `--runner <cmd>`    | command | launch every test as `<cmd> <exe>` instead of exec'ing it directly |

Plus the `build` and global flags (`-v` lists every test). `--runner` has the
same semantics as on `mach run`: a single command name or path (no shell-style
word splitting), resolved on `PATH`, for foreign-target tests the host cannot
exec directly — e.g. `mach test . --target windows --runner wine`. Without it, a
test executable the host cannot launch reports a per-test failure — `(exit 127)`
when `execve` rejects the binary in the spawned child, `(spawn failed)` when the
spawn itself fails — with no auto-detection.

Exit codes: `0` all passed, `1` any failed, `2` build/internal error.

## `mach dep`

```
mach dep <action> [args]
```

Manages the project's dependency tree under `<dep>`. Dispatches on `argv[2]`.
A dependency has exactly one source form: a **git** URL plus a ref, which mach
acquires into `<dep>/<name>/` with plain git operations, or a **path** to another
project tree, never fetched but materialised at `<dep>/<name>/` as a relative
symlink so the build resolves it by the same vendor layout.

| Action   | Args | Effect |
|----------|------|--------|
| `pull`   | — | realise the manifest: clone missing git deps (transitively), link path deps, re-resolve a changed ref, repair checkout-vs-lock drift, write `mach.lock`. Idempotent; the command routine use needs. |
| `update` | `<name> \| --all` | the only lock-advancer: re-resolve branch refs to current remote tips. Tag/commit refs are an immutable no-op. Never edits the manifest. |
| `add`    | `<name> --git <url> [--ref <ref>] \| --path <dir>` | append a `[deps.<name>]` `git`/`path` stanza to the manifest, then `pull`. |
| `remove` | `<name> [--purge]` | drop the entry from `mach.toml` and `mach.lock`; `--purge` also deletes `<dep>/<name>/`. |
| `list`   | — | print each `[deps.<name>]` entry with its source form, ref, locked commit, and state (`synced`/`missing`/`drifted`/`path`). |

`sync` is the pre-`pull` name, kept one cycle as a hidden alias that prints a
deprecation note and runs `pull`.

### Transport policy

`mach build` never requires git or the network: a project whose dep tree is
present builds on a bare machine. Only the network-shaped commands (`pull`,
`update`, `add`) use git — the single fetch transport, discovered on `PATH`
(scanned directly, since the spawn API uses `execve` with no path search) and
invoked with an allowlisted environment (`PATH`, `HOME`, and the common
git/ssh/proxy/CA variables). Git's absence is a clean error naming the operation
that needed it.

For each git dependency, `pull` clones the `git` URL into `<dep>/<name>/` when
absent, then checks out the resolved commit as a **detached HEAD**
(`git checkout --detach`). A ref resolves as: `branch/<n>` (the remote-tracking
branch tip), `tag/<n>`, `commit/<n>` or a 7–40 char hex SHA (the literal commit),
the empty ref (the remote default branch), or a bare name auto-detected as a
remote branch (tracking its tip) else a literal tag/commit. mach performs only
plain git operations, so a checkout the user also commits as a **submodule**
composes naturally — a moved checkout surfaces as gitlink drift in the parent
repo's `git status`. mach never invokes `git submodule`.

A **non-empty** directory present under `<dep>/<name>/` without a `.git` entry,
while the manifest declares it a git dep, is a hard error: declare it a `path`
dependency if those are vendored files. (`.git` as a file — a submodule gitlink —
counts as a checkout.) An **empty** directory has nothing to vendor and is treated
as absent — `pull` clones into it — so a plain `git clone` (without
`--recurse-submodules`), which leaves a submodule dep dir empty, is repaired by a
plain `mach dep pull` (#1329).

For each **path** dependency, `pull` materialises the source at `<dep>/<name>/` as
a relative symlink, so the build resolves its modules by the same vendor layout as
a git dep. The `path` is resolved relative to the requiring manifest's directory;
the link is relative, so a tree that moves as a whole (a monorepo, a committed
examples dir) stays linked without a re-pull. The step is idempotent: a stale link
is replaced, an already-correct link is left in place, and a source that already
lives at the vendor location (in-tree vendoring) is a no-op. A `path` pointing at a
missing directory, a directory without a `mach.toml`, or a vendor location occupied
by a real directory (a stale git checkout, foreign vendored files) is a hard error
— never silent success (#1370).

### Transitive resolution

Transitive deps resolve into the **flat dep tree**: every git dep, direct or
transitive, lives at `<dep>/<name>/`, so a dependency's own
`[target.*].libs` cascade into the consumer's build (see `manifest.md`). The same
name required from two different sources or refs is a hard error naming both
requirers; there is no version resolution (reserved for the registry era).

### Lockfile (`mach.lock`)

The manifest is intent; the lock is the record of resolving it. After a `pull`,
`mach dep` writes `mach.lock` — a TOML file recording each git dep's `url`, `ref`,
and resolved `commit` (path deps have no lock entry):

```toml
# generated by `mach dep pull`; do not edit by hand.
version = 1

[deps.mach-std]
url = "https://github.com/briar-systems/mach-std"
ref = "branch/dev"
commit = "6b78ae1e8c3c9cc45e4ab4b916fd191d61e76aff"
```

`pull` honours the lock except where the manifest ref was edited — there it
re-resolves loudly (`re-resolved <name> (manifest ref changed: <old> → <new>)`).
A checked-out commit that differs from the lock is drift, repaired by `pull` and
reported, never silent. `update` is the only other writer, advancing branch refs
to their current tips. The lock writer is idempotent: an up-to-date lock is left
untouched. Commit `mach.lock` to pin builds.

`mach dep` reads `mach.toml` from the current directory directly (it does not walk
up to find a project root); each `[deps.<name>]` declares exactly one of
`git`/`path`, with `ref` for git (see `manifest.md`). Exit codes: `0` ok, `1` user
error, `2` internal error.

## `mach init`

```
mach init [dir] [options]
```

Scaffolds a new project in `[dir]` (default: the current directory). Writes a
complete `mach.toml` with a `[project]` block, `[target.*]` platforms for
`linux`/`windows`/`darwin` (on the host ISA), one `[bin.*]`/`[lib.*]` artifact,
`[profile.debug]`/`[profile.release]` variants, a `[deps.mach-std]` dependency, a
starter source file, and `dep/mach-std/` cloned from the declared ref (through
the same path as `mach dep pull`). Refuses to overwrite an existing `mach.toml`,
`src/main.mach`, or `src/lib.mach` unless `--force`; every collision is checked
before any file is written, so a refused init leaves nothing behind.

| Flag           | Value | Effect |
|----------------|-------|--------|
| `--name <name>`| name  | project id (default: the directory base name) |
| `--force`      | —     | scaffold even when `mach.toml`, `src/main.mach`, or `src/lib.mach` already exists |
| `--lib`        | —     | library layout: write `src/lib.mach` instead of `src/main.mach`, and scaffold a `[lib.<id>]` artifact (`kind = "static"`) instead of `[bin.<id>]` |

The first non-flag argument after `init` is the target directory.

`mach init` scaffolds a buildable project directly. For a default binary
scaffold, `mach build .` links and runs without further manifest edits.

Exit codes: `0` ok, `1` user error, `2` internal error.

## `mach doc`

```
mach doc <path> [options]
```

Loads the project's module graph and generates Markdown reference docs from
source doc-comments — one page per module plus an index. Each `pub` declaration
is paired with the run of `#` comment lines immediately preceding it. The
hand-written `doc/language/` material is never touched.

| Flag             | Value | Effect |
|------------------|-------|--------|
| `--out <dir>`    | dir   | output directory, rooted at the project (default `doc/api`) |
| `--target <name>`| name  | select a `[target.<name>]` for module discovery |

Plus the global flags. Exit codes: `0` ok, `1` user error, `2` internal error.

## `mach info`

```
mach info [--version]
```

Prints an at-a-glance identity of the binary: its version, the host (`os/isa`)
it was built for, and the registered capability surface — the instruction sets,
operating systems, ABIs, and object formats it can compose into a target. This
needs no project (it runs from anywhere, with or without a `mach.toml`). The
output is line-oriented and stable for scripts:

```
mach 1.3.0
host: linux/x86_64
isa: x86_64 aarch64
os: linux darwin windows
abi: sysv win64
object: elf macho coff
```

The version line and `host:` line fold at compile time; the four capability
lines are read from the binary's target registries, so they report exactly what
this build can target. `mach info --version` prints the version string alone on
one line, for tooling.

| Flag        | Value | Effect |
|-------------|-------|--------|
| `--version` | —     | print the version string alone, on one line |

Exit codes: `0` ok, `2` internal error.

## `mach help`

```
mach help [command]
```

Prints the top-level usage summary, or — with a known `[command]` — that
command's detail page. An unknown command prints the top-level usage and exits
non-zero.

## See also

- [manifest.md](manifest.md) — the `mach.toml` reference
- [language/test.md](language/test.md) — the `test` declaration and `mach test`
- [language/files.md](language/files.md) — project file layout
