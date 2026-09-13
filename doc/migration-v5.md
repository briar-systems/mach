# Migrating a 4.x project to Mach 5.0

Mach 5.0.0 ships paired with std 2.0.0. The language gains tagged values and
loses every form that let a failure be spelled as a string, a sentinel or a
record with a kind beside a union; the manifest and the comptime channel lose
the keys and paths 4.x accepted and never read; std spells every fallible or
absent outcome with three tags declared in `std.types.result`, `std.types.option` and `std.types.error`. This page is
the order of operations for a project that builds with 4.30 today. The
normative language contract is [design/tagged-values.md](design/tagged-values.md)
and the reference is [language/](language/README.md); std's own inventory is
its `MIGRATION.md` at the repository root of `mach-std`.

Every example below carries an expectation on its fence; see
[language/README.md](language/README.md#exercised-examples).

## 1. Get a 5.0 compiler first

A 4.x compiler cannot build 5.0 source. The v5 syntax (`tag`, `sel`,
`Type.case{}`, `:>T` as the only declassification) is unreadable to a 4.x
front end, and std 2.0.0 is written in it, so a 4.x compiler fails at the
first std module it loads. The compiler's own source is in the same position:
the tree that pins std 2.0.0 is built by the v5 stage of the bootstrap chain,
never by the 4.30 seed. Install a 5.0 release binary, or build one through the
pinned chain in [tooling/bootstrap.md](tooling/bootstrap.md), before touching
the project.

The other direction holds too: a 5.0 compiler does not read a project that
still spells the 4.x forms. Every removal below is a located diagnostic that
names the removal and the replacement, so the migration is driven by the
compiler's own messages, in the order it meets them (manifest first, then
dependencies, then source).

## 2. The manifest

`mach.toml` is refused at the first removed key. The keys and spellings 4.30
accepted with a note are refused in 5.0.0 by name:

| 4.x form | 5.0 diagnostic | do this |
| --- | --- | --- |
| `[project] name`, `description`, `mach` | `mach.toml: [project] key 'name' was removed in 5.0.0; it was accepted and never read: remove the key` | delete the key; `id` and `version` identify the project |
| `[profile.*] emit_ir`, `emit_asm` | the same message for the profile key | delete the key; `--emit-ir` and `--emit-asm` on the command line are the only switches |
| several `[target.*]`, `[profile.*]` or `[artifact.*]` with none marked `default = true` | `` mach.toml: several profiles are declared and none is marked `default = true`; no profile is selected by table order: mark exactly one [profile.<name>] with `default = true` or select one with --profile `` (the target and artifact messages have the same shape) | mark one table `default = true`; table order never selects |
| a `[dep.<key>]` whose realized project declares a different id (an alias key) | `[dep.foo] realizes project 'std'; alias keys were removed in 5.0.0: the manifest key, the directory under dep/, and the project id are one name, so rename the table to [dep.std] and the directory to dep/std` | rename the table and the directory to the dependency's own id |
| a nested `dep/<a>/dep/<b>/` realization | `dependency 'a': dep/a/dep/b is a nested realization; nested realizations were removed in 5.0.0 (...): delete dep/a/dep` | delete it; the root's `dep/` holds the whole closure one level deep |
| a `mach.lock` in the project root | `mach.lock was removed in 5.0.0 and is refused; the committed gitlinks under dep/ are the pins: delete mach.lock` | delete it; commit the gitlinks |
| `[target.*] isa = "mos6502"` | `target 'mos6502' was withdrawn and removed in 5.0.0; no isa or abi implementation is registered for it: retarget the [target.*] table to a supported tuple` | retarget; `mach info targets` lists the supported tuples |
| a dependency with no `[artifact.*]` imported bare (`use dep;`) | `project 'x' declares no artifact, so it has no public module; the implicit lib.mach entry of an artifact-less dependency was removed in 5.0.0: import a full path, or declare a static or shared [artifact.*] table marked default = true in its manifest` | import by full path, or have the dependency declare its library artifact `default = true` |
| a `#[embed("...")]` path outside the project root | `` `embed` path escapes the project root; an embedded file must live inside the project (4.30 read it with a warning, 5.0.0 refuses it and does not read the file) `` | move the asset under the project root |

Nothing else in the manifest schema changed between 4.30 and 5.0; every
accepted key is tabled in [manifest.md](manifest.md). `mach dep pull` then
realizes the closure against the committed gitlinks; the scaffold's
`[dep.std] ref = "branch/main"` resolves to std 2.0.0.

## 3. Source: the removed spellings

The compiler refuses each of these at its use site, naming the replacement.

- **`:^` and `:^T`**, the 4.30 declassification spellings, are gone. `x:>T`
  is the only strip and always names the public result type. Inside a generic
  body the target is checked per instance.

  ```mach reject "`:^` and `:^T` were removed in 5.0.0"
  fun publish(a: ^u32) u32 { ret a:^u32; }
  ```

  ```mach accept
  fun publish(a: ^u32) u32 { ret a:>u32; }
  ```

- **`$project.name` and `$project.description`** went with their manifest
  keys; `$project.id` identifies the project and there is no comptime
  description.

  ```mach reject "`$project.name` was removed in 5.0.0"
  use std.types.string.str;
  val NAME: str = $project.name;
  ```

- **`$mach.abi.sysv`** is refused by name; the registry spells the ABI
  `sysv64`.

  ```mach reject "`$mach.abi.sysv` was removed in 5.0.0"
  val SYSV: u8 = $mach.build.abi == $mach.abi.sysv;
  ```

- **`sel` is a keyword.** A binding, field, function or module member named
  `sel` must be renamed before the file parses under 5.0.

- **`$mach.arch.mos6502`** is an unknown tag: the target is gone.

## 4. Source: tagged values replace `Result`, `Option` and `Void`

std 1.x carried a failure as `Result[T, str]`, an optional error as
`Option[str]`, unit success as `Void` and absence as `Option[T]`, with helper
functions (`ok`, `err`, `is_ok`, `unwrap`, `some`, `none`) over a record. All
of that is deleted in std 2.0.0 (`std.types.result` and `std.types.option` no
longer exist) and replaced by three ordinary tags:

```mach
pub tag res[T, E]: u8 { err: E; ok: T; }
pub tag opt[T]: u8    { none; some: T; }
pub tag err[E]: u8    { err: E; ok; }
```

They are declared in `std.types.result`, `std.types.option` and `std.types.error` and imported like any declaration;
the compiler knows nothing of the three names. A module that spells `res`
without importing it fails with `unresolved type name`.

```mach accept
use std.types.result.res;
use std.types.option.opt;
use std.types.error.err;
```

The mechanical translation of each 1.x site:

| std 1.x | Mach 5.0 with std 2.0.0 |
| --- | --- |
| `R.Result[T, str]` return type | `res[T, E]` with `E` the module's closed error tag, never `str` |
| `R.ok(v)` | `res[T, E].ok{v}` |
| `R.err("text")` | `res[T, E].err{E.case{...}}` |
| `if (R.is_ok(r)) { ... R.unwrap(r) ... }` | `if (sel r.ok) { ... r.ok ... }` (the arm guards the payload) |
| `if (R.is_err(r)) { ret r; }` then `R.unwrap(r)` | `if (sel r.err) { ret res[T, E].err{r.err}; }` then `r.ok` (the exiting chain guards the rest of the block) |
| `R.Result[R.Void, str]`, `ok_void()`, `void_of(...)` | `err[E]`, `err[E].ok{}`, `err[E].err{e}` |
| `O.Option[str]` as an optional error | `err[E]` |
| `O.Option[T]`, `O.some(v)`, `O.none()` | `opt[T]`, `opt[T].some{v}`, `opt[T].none{}` |
| `O.is_some(o)` then `O.unwrap(o)` | `sel o.some` then `o.some` under the guard |
| `R.unwrap_err(f(...))`, a test on a call | bind first: `val r: res[T, E] = f(...);` then `sel r.err`; `sel` takes a place, never a call |
| a kind field beside a union | a `tag` with one case per kind |

Three rules bite during the translation, all consequences of guards being
lexical rather than flow facts:

- an `or` arm is never guarded by the `if` arm's test, so
  `if (sel r.err) {...} or { r.ok }` is rejected; write
  `if (sel r.ok) { r.ok } or {...}`, or exit the error arm;
- `||` opens no guard, so `if (sel r.err || r.ok != 1) { ret 1; }` is
  rejected even though the arm exits; split it into one `if` per term;
- after an exiting chain the tested place is guarded for the rest of the
  block and cannot be whole-assigned, so a `var r` reused across a loop must
  become a fresh `val` per iteration, or be tested inside the loop body.

```mach accept
use std.types.result.res;
use std.types.option.opt;

tag ParseError: u8 { invalid; overflow; }

fun parse(x: i64) res[i64, ParseError] {
    if (x < 0) { ret res[i64, ParseError].err{ParseError.invalid{}}; }
    ret res[i64, ParseError].ok{x};
}

fun first_positive(xs: *i64, n: i64) opt[i64] {
    var i: i64 = 0;
    for (i < n) {
        val r: res[i64, ParseError] = parse(xs[i]);   # a fresh binding per iteration
        if (sel r.ok) { ret opt[i64].some{r.ok}; }
        i = i + 1;
    }
    ret opt[i64].none{};
}
```

Zero initialization changed meaning with the carrier: a zero `res` or `err`
is its `err` case with a zero error payload, where a zero `Option[str]` was
"no error". An accumulator a function keeps across clean-up steps is
initialized to `.ok{}` explicitly.

Whole-tag `==`, ordering, a `.kind` field, a `match` construct and an
automatic conversion between error types do not exist. Domain errors,
formatting and conversions are ordinary functions in std or the project.

## 5. std 2.0.0 by domain

std's `MIGRATION.md` holds the frozen signature of every public API in
"Frozen core signatures" and the per-domain tables in "Domain inventory"; its
`CHANGELOG.md` names the 2.0.0 surface by domain. The outcome forms, by domain
and the `MIGRATION.md` section that owns them:

| domain | outcome forms | section |
| --- | --- | --- |
| allocator, backends, collections | `res[_, allocator.Error]`, `err[allocator.Error]`, `opt` for absence, `SearchPosition` for `binary_search`; backends and containers initialized in place | "Frozen core signatures": `std.allocator`, Allocator backends, Collections |
| types and text foundations (`types.string`, `view`, `path`, `semver`, `text.string`) | `StrError`, `SemverError`; `res`/`opt` constructors and searches | "Frozen core signatures": Types |
| text, encoding, data, compression | `ParseError`, `FormatError`, `InputError`, `EncodeError`, `DecodeError`, `JsonError`, `TomlError`, `InflateError`; the gzip lifecycle settles a stored failure | "Text, encoding, data and compression (S2)", "Compression" |
| I/O | `ReadError`, `WriteError`, `StateError`; readers, writers, handles, the file adapter and the completion runtime over `io_error.Error` | "I/O (S3a)" |
| filesystem | `res[T, io_error.Error]` / `err[io_error.Error]` for handle operations, `FsError`, `removal.Error`, `transaction.Error` for composites | "Filesystem (S3b)" |
| process and network | `EnvError`, `exec.Error` (with `retained` naming an unreaped child), `ip.ParseError`, the socket families over `io_error.Error` and `types.Error`, `lookup.Outcome` | "Process and network (S3c)" |
| synchronization and clocks | `ThreadError`, `StateError`, `InitError`, `channel.Status[T]`, `worker_pool.Status`, `condition.WaitStatus`, `cancel.Reason`; `time.Clock` answers `res[Time, io_error.Error]` | "Synchronization and clocks (S4a)" |
| crypto and random | `crypto.rand.fill` is `err[io_error.Error]` with the completed prefix left in the buffer; hashes, `ct` and `rand` unchanged | "Crypto and random (S4a)" |
| terminal and logging | `TermError`, `SinkError`, `WriteStatus`, `EncodeStatus`, `ClockError`; sinks initialized in place; every `ERR_*` string gone | "Terminal and logging (S4b)" |
| runtime and OS, math, SIMD | the native boundary (foreign ABI widths, native constants, negative errno, nil sentinels) unchanged on purpose; `math.mat4.mat4_inverse` is `opt[Mat4]` | "Runtime and OS", "Math and SIMD" |

Two std facts to know before reading a signature:

- an address-bound owner (an allocator, a cancellation scope, a queued sink,
  a runtime registration) is initialized in place, `init(*T, ...) err[E]`,
  into caller-owned final storage; it is never returned by value inside a
  `res`;
- std 2.0.0 declares its one artifact without `default = true`, so a bare
  `use std;` has no module to bind and is refused; import std's modules by
  full path (`use std.print;`, `use std.types.result.res;`).

## 6. Tooling that arrived with 5.0

- `mach check <path>` runs the frontend over the selected artifacts and stops:
  the same diagnostics and classification a build would give, nothing
  generated or written. Use it as the migration's inner loop.
- `mach build <path> --plan` prints the effective build (cells, steps in
  execution order, link requirements) without running anything.
- `mach dep verify <path>` runs the build's dependency checks alone and prints
  `ok` or the first refusal; `mach dep sync` is a deprecated alias of `pull`.
- `mach fmt` is in a parallel lane and not part of this document until it
  merges; until then the reference formatter is the layout of the compiler's
  own source.
- `#[deprecated]` and `#[deprecated("msg")]` mark a declaration (every kind,
  including a `tag` and a single case) so that external uses warn once per
  site with the message; a library that retires a surface marks it for one
  minor before removing it. See
  [language/decorators.md](language/decorators.md#deprecated--deprecatedstr--source-use-notice).

## 7. Order of operations

1. Install a 5.0 compiler. `mach info --version` prints it.
2. Delete `mach.lock`, rename alias dependency keys, delete nested `dep/*/dep`
   trees, remove the five unread manifest keys, mark defaults. `mach dep pull
   .` then `mach dep verify .` until it prints `ok`.
3. Rename every identifier spelled `sel`. Replace `:^`/`:^T` with `:>T`.
4. Import `std.types.result.res`, `std.types.option.opt` and `std.types.error.err` where a module spells them and
   translate each `Result`/`Option`/`Void` site with the table in section 4,
   module by module, running `mach check .` after each.
5. Follow each std signature change through `MIGRATION.md`'s domain table
   for the modules the project imports.
6. `mach build .` and `mach test .`.
