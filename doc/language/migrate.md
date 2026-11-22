# Migrating a project

`mach migrate <path> [--check]` moves a project from the previous major to the
current one. It rewrites the root project's `mach.toml`, then every source file
under its `[project].src`, and reports what it cannot rewrite. Each file it rewrites
is named on its own line.

- It is idempotent: a migrated project changes nothing.
- `--check` writes nothing and prints each change as a unified diff with no
  context lines, so CI can confirm a project is migrated. Each hunk header names
  the change's line numbers, and in the manifest, after the `@@`, the table it sits
  in, as `@@ -29,3 +29,3 @@ [target.darwin-aarch64]`. A source file's hunk header
  names only the line numbers. The output applies with
  `patch -p0` from the directory `mach migrate` ran in.
- It works on the root project only. Dependencies under `dep/` belong to other
  projects and are never edited, and one still on the previous major is reported.
- A script that runs `mach` with a removed option is reported with its file and line,
  never rewritten. A script is any file of the project outside `dep/`, the source
  directory, the work directory and `.git` that is named `Makefile`, `makefile` or
  `GNUmakefile`, ends in `.sh`, `.bash`, `.mk`, `.ps1`, `.bat` or `.cmd`, is a `.yml`
  or `.yaml` file under `.github/workflows/`, or is executable and starts with `#!`.
- A removed option in the `argv` of a `[step]` that runs mach is reported the same
  way, with its line in `mach.toml`, unless the migration rewrites it.
- A script or a `[step]` that reads the step environment 6 set, `MACH_TARGET_ISA`
  or `MACH_TARGET_OS`, is reported with its line and the name 7 sets,
  `MACH_TARGET_ARCH` or `MACH_TARGET_SYS`. A script is read for these names
  anywhere in its text, and a step in its `argv` and `env` values, wherever no
  letter, digit or `_` stands on either side.
- A project whose `[project] mach` is neither accepted by this mach nor on the
  previous major is refused, and nothing is written. Migrate it with the previous
  major's mach first.
- The names no rule rewrites are checked before anything is written. A
  `[project].id` or a `[dep.<id>]` key that 7 refuses, as one holding `-`, which 6
  accepted, is refused with its line in `mach.toml`, and the manifest and the
  source are left as they were.
- A bare `use id;` or `fwd id;` whose module the migration cannot tell, as `id`
  names no dependency, `dep/` holds no manifest for it, or it exports no library, is
  reported with its file and line while the rest of the file is rewritten.
- Rules are data rows tagged with the major they migrate from. A release carries the
  rows of the major before it, and the next major deletes them.
- Only tokens of source are read, so a comment or a string that spells an old form is
  left as written.

## How a script is read

A script is read as shell commands, and only the words of a mach invocation are
compared with the removed options:

- A simple command ends at `&&`, `||`, `|`, `;`, `(`, `)`, a backquote, or a line
  break no trailing `\` continues. A `#` where a word would start comments out the
  rest of its line. Quotes keep these bytes inside one word.
- A simple command's command word is its first word after any `NAME=value`
  assignments, and after `-` and `run:` as a workflow step writes them. It names
  mach when it is `mach`, a path ending in `/mach`, `$MACH` or `${MACH}`, any of
  them in quotes, after the `@`, `-` or `+` a make recipe may lead with.
- Every later word of that simple command, without its quotes, is compared whole
  with each removed option, alone or with its value after `=`.
- An option removed from one command only, as `--version` from `mach dep add`
  (now `--range`), is compared only in an invocation whose words before its first
  option start with that command, so `mach --version` is never reported.
- A workflow step's `run: '...'` or `run: "..."` is read inside its quotes, and a
  `run: |` block line by line.
- A workflow expression `${{ ... }}` stays inside the word it is written in, as
  the runner replaces it before the shell reads the line, and each quoted string
  in it, as `'--pie'`, is compared as a word of that command.

Not recognised: mach reached through any other variable or alias, a command word
after `exec`, `env`, `time`, `sudo`, `if` or another prefix, a function or script
that wraps mach, and a file that is not a script by the rules above. A here-document
is not recognised as one: its lines are read as commands like any other line. An
option in another program's words, as `cc -O2`, or in a comment is never reported.

## Exit code

- `0`: the project is in this major's spelling, or was made so. Script and step
  reports leave it at `0`: they name files the migration never edits.
- `1`: under `--check`, a file would change. In either mode, the manifest still
  writes a removed key no rewrite migrates, each one reported, a source could
  not be read as source, or a source holds a bare `use` the migration could not
  rewrite, each one reported. The manifest reader refuses such a project, so the input
  is what is wrong. So does a system library 6 found that the manifest does not
  yet say how 7 finds, each one reported, since the link refuses it. A missing or
  invalid path, a `[project].id` or `[dep]` key the reader refuses, or a
  `[project] mach` that names a range this release does not migrate from also exits
  `1`, as for every command, with nothing written.
- `2` and `3`: an internal or environment failure, as for every command.

## The changes from 6 to 7

Rewritten by `mach migrate`:

| 6 | 7 |
|---|---|
| `opt = 0` | `optimize = false` |
| `opt = 1` or `opt = 2` | `optimize = true` |
| `float_reassoc = true` | `relax = ["float-reassoc"]` |
| `float_reassoc = false` | removed: float arithmetic is exact unless a profile relaxes it |
| `vectorize = true` or `vectorize = false` in a profile | removed: `optimize` schedules the pass, and `#[vectorize("never")]` opts one function out |
| `[project] out`, `{project.out}` | `work`, `{project.work}` |
| an artifact `out` such as `"bin/app"`, which v6 rooted under `[project] out` | `"{project.work}/bin/app"`, the same file name: 6 wrote `out` as written, adding no prefix and no suffix for any kind on any target, as 7 does, and `{artifact.suffix}` stays where it was written |
| `out` on an `object` artifact | removed: its objects stay under `obj/` |
| `default` on a target | removed: with no `-t` a command takes the target matching the host |
| `{target.isa}`, `{target.os}` | `{target.arch}`, `{target.sys}` |
| an `isa` naming an architecture | `arch` |
| an `isa` naming a RISC-V selection, as `isa = "rv32imc"` | `arch = "riscv32"` beside it |
| a freestanding target's `isa` naming an architecture, as `isa = "riscv64"` | `arch` with `isa` set to the floor 6 built for (`x86-64`, `armv8-a+fp+simd`, `rv64gc`, `rv32imac`), since 7 has no freestanding default |
| a freestanding target with no `of`, which 6 wrote as a flat image | `of = "flat"`, since 7 writes ELF by default |
| `abi = "aapcs64"` on a darwin aarch64 target | `abi = "darwinpcs"` |
| `of = "raw"`, `of = "spv"` | `of = "flat"`, `of = "spirv"` |
| a spirv target's `os = "freestanding"`, `abi = "spirv"` | `sys = "vulkan"`, `abi = "shader"` |
| a spirv target's v6 `ext` names, as `buffer_device_address`, `int64`, `subgroup_ballot` | the SPIR-V capability names, with the SPIR-V extension where the capability is not core, as `PhysicalStorageBufferAddresses`, `Int64`, `GroupNonUniformBallot`, each listed once |
| `kind = "bin"` on an artifact whose targets are all spirv | `kind = "module"`, the one kind a spirv target builds |
| `default = true` on a library artifact, which 6 made the export surface whose `need` travelled to consumers | `export = true` beside it, when no artifact states `export` and one library is the default |
| `subsystem = "gui"` | `subsystem = "windows"` |
| `os`, `baseline`, `extensions` | `sys`, `isa`, `ext` |
| link `os`, `isa` | link `sys`, `arch` |
| a link filter written as one string, as `os = "linux"` or `abi = "*"` | an array of it, as `sys = ["linux"]` or `abi = ["*"]` |
| `$mach.os`, `$mach.build.os`, `$mach.build.extensions`, `$project.target.os` | `$mach.sys`, `$mach.build.sys`, `$mach.build.ext`, `$project.target.sys` |
| `#[op]`, `#[stage]` and the shader decorators | `#[intrinsic]`, `#[gpu.stage]` and the `gpu.` family |
| `#[inline]`, `#[noinline]`, `#[scalar]` | `#[inline("always")]`, `#[inline("never")]`, `#[vectorize("never")]` |
| `#[oblivious]` | `#[constant_time("soft")]` |
| `#[ext(sha)]`, `#[extensions(sha)]` | `#[extension("sha")]` |
| `asm x86_64`, `asm aarch64`, `asm riscv64`, `asm riscv32` | `asm x86-64`, `asm armv8-a`, `asm rv64i`, `asm rv32i` |
| `#` to the end of a line in an aarch64 or WebAssembly `asm` body, a comment on every instruction set in 6 | `//` and `;;`, the comment of the body's own dialect ([asm.md](asm.md#how-a-block-is-read)) |
| `def bool: u8;` | `def bool: u1;` |
| `use id;`, `fwd id;` | the path of the dependency's exported artifact entry |
| `--jobs`, `--jobs=n` in the `argv` of a `[step]` that runs mach | `--threads`, `--threads=n` |
| `[project] mach` on the previous major | `^7`, the range `mach init` writes |
| `fwd.instances`, `debug.dropped`, `target.default_deprecated`, `target.native_fallback` in a profile's `allow` | removed: 7 never raises these warnings |

Left for you to change:

| 6 | 7 |
|---|---|
| a condition of type `u8`, as `if (flags & 1)` | a `u1`, as `if ((flags & 1) != 0)` |
| a `u1` stored in a `u8`, under arithmetic, a shift or `~`, or beside a wider operand of `&`, `\|` or `^` | `value::u8`, or a `u1` destination |
| a secret passed through a function value from `#[oblivious]` code, as `seal(key, data)` for `seal: fun(*^u8, u64) u64` | an alias of the function type marked like the code, `#[constant_time("soft")] def Seal: fun(*^u8, u64) u64;`, as the type of the value, its field and its parameter |
| `-g`, `-O0`, `-O2`, `--emit-asm`, `--emit-ir`, `--pie`, `--simd`, `--jobs`, `--arch`, `--sys`, `--of`, `--platform`, `--subsystem`, `--base`, `--stack-reserve`, `--stack-commit`, `-d`, `-e`, `-s`, and `--version` after `mach dep add`, in a script, or in a step's `argv` other than `--jobs` | the spelling the report names |
| `MACH_TARGET_ISA`, `MACH_TARGET_OS` read by a script or a step | `MACH_TARGET_ARCH`, `MACH_TARGET_SYS` |
| a `[project].id` or a `[dep.<id>]` key holding `-`, which 6 accepted | an id of letters, digits and `_` that starts with a letter or `_`, renamed together with the `dep/<id>/` directory and every consumer's `[dep]` key and `use` paths |
| a bare `use id;` or `fwd id;` whose dependency is not in the project's closure, has no manifest under `dep/`, or exports no library | the path of the module it bound, as `use id.lib.id;` |
| a spirv target's v6 `ext` name that enables no capability: `zero_init_workgroup`, `subgroup_graphics_stages`, `maintenance8` and the plain `*_float16_atomics`, `*_float32_atomics`, `*_float64_atomics` | removed from `ext`, and the report names the target and the entry, since the consumer still enables the device feature |
| a `bin` artifact listing a spirv target beside another | a separate `kind = "module"` artifact for the spirv target, which `mach migrate` reports |
| a linux target with no `env` that an artifact links a `source = "system"` entry on, which 6 found in the gnu environment's directories | `env = "gnu"` on the target, under which a `bin` links `pie` unless its `linkage` says otherwise, so the report names the target and the entry |
| a darwin `source = "system"` shared entry named by no install name, as `name = "MoltenVK"`, whose file 6 found and linked by the install name it states | the install name, as `name = "@rpath/libMoltenVK.dylib"` or an absolute path, which 7 links from the entry's claim with no file |
| a linux artifact whose links reach the C library only through another library's dependency, as `pthread`, `m`, `dl` or `rt` with no `c`, which 6 bound through that library's own dependency | `c` named in the artifact's links, since 7 binds a reference only to a library the link names |

The compiler refuses the first three, `mach migrate` reports the next eight, and the
last is refused by the link, with no report before it. An id
or a `[dep]` key holding `-` is refused as `manifest.invalid_name` before anything is
written, since the id is one name in three places, the `[dep]` key, the `dep/`
directory and the head of every consumer's `use` path, and migrate edits only the
root project. A bare `use` it cannot rewrite is reported as `use.unresolved` at
its name, saying which of the three it met. A `u8`
condition is refused as `condition.type` and a `u1` under an operator as
`type.mismatch`, each naming the replacement. A `u1` stored in a `u8` is refused as
`type.mismatch`, `expected u8, found u1`, with a help line naming the cast,
``parenthesise the value and cast it with `::u8` ``. A secret passed through a function
value whose type states no constant-time mode is refused as `secret.call_untrusted`,
naming an alias marked `#[constant_time]` as the fix. `mach migrate` rewrites `#[oblivious]` but writes no alias, since only the
author knows which function values the code means to trust: the functions stored in
such a value must be constant time themselves, and a plain or `#[weak]` one is refused
where it is stored ([secrecy.md](secrecy.md#function-types-marked-constant-time)). Several default libraries in one manifest are
reported with their lines, since 7 exports one library and only its `need`
travels. The manifest reader refuses a
removed key with no mechanical rewrite, and `mach migrate` reports each one with
its line: a link's `library`, which the table key replaces, a profile's `pass` and
`skip`, an `opt` other than `0`, `1` or `2`, and a step's `cmd` and `shell`, which
`argv` replaces. A table that writes such a key still gets every other rewrite, and
the key is reported once. The link refuses the last three. The `env` and the
install name have no one answer migrate can write, since `env` changes how every
`bin` of the target links and the install name is the library's own. The C library
is refused as symbols nothing defines. When the named library is a shared library
that depends on the C library, as `m` is, a note says so. On glibc 2.34 and later
`pthread`, `dl` and `rt` define none of their old symbols: their archives are empty,
so a static link's refusal names only the undefined symbols and the files that refer
to them, and their shared objects depend on the C library, so a shared link's refusal
adds the note. A report names the line of the table it is about.

A tag whose payloads disagree on the class of a byte, secret against public or a
pointer that reaches a secret against anything but a pointer to the same storage,
now gives them separate payload areas where 6 put every payload at one offset, as
[tag layout](tag.md#layout-and-representation) describes. Its size, its payload
offsets and the way it is passed by value can grow with no source change, and
nothing is rewritten or reported. Check code that persists such a tag's bytes,
shares it with C, or depends on its `$size_of` or `$offset_of`.

### Names std renamed or moved

`mach migrate` rewrites the language and the manifest, never a name a dependency
renamed or moved: those are the dependency's own changes, and its `MIGRATION.md`
lists them. std 10.0.0, the std that pairs with mach 7, made two such changes that touch
most projects. Both are left for you to change.

Every std record field holding a function for a caller to invoke is named
`fn_<action>`, its signature unchanged. A literal or a read that spells the old
name is refused as `field.unknown`, with no hint of the new one.

| std 9.x | std 10.0.0 |
|---|---|
| `io.reader.Reader{f_read: ...}`, `io.writer.Writer{f_write: ...}` | `fn_read`, `fn_write` |
| `log.record.Clock.f_now`, `log.sink.Sink.f_publish`, `log.sink.Direct.f_write` | `fn_now`, `fn_publish`, `fn_write` |
| `collections.map.MapBy.hash_fn`, `eq_fn` | `fn_hash`, `fn_eq` |
| `sync.worker_pool.Task.run` | `fn_run` |
| `net.resolve.Resolver.adapter` | `fn_adapter` |

Modules left std for packages. Declare the package under its key, as
`[dep.crypto]` with `git = "https://github.com/briar-systems/mach-crypto"`, and
import the module by its path in the package. A `use` of the old path is refused
as `use.unresolved`.

| std 9.x | 10.0.0 | package |
|---|---|---|
| `std.crypto.hash.sha256`, `sha512`, `keccak`, `sha3_256`, `sha3_512`, `shake128`, `shake256` | `crypto.hash.sha256` and the same names under `crypto.hash` | mach-crypto |
| `std.crypto.hash.crc32`, `adler32` | `deflate.crc32`, `deflate.adler32` | mach-deflate |
| `std.compress.inflate`, `deflate`, `zlib`, `gzip`, `format` (and the `compress_format` re-export) | `deflate.inflate` and the same names under `deflate` | mach-deflate |
| `std.data.json` | `json.parse`, `json.value`, `json.decode`, `json.write`, `json.ndjson`, `json.error` | mach-json |
| `std.data.toml` | `toml.parse`, `toml.lookup`, `toml.value`, `toml.tree`, `toml.write`, `toml.expr`, `toml.error` | mach-toml |
| `std.math.mat4`, `std.math.quat` | `math.mat4`, `math.quat` | mach-math |
| `std.encoding.base64` | `encoding.base64` | mach-encoding |

The moved hashes and checksums take the lifecycle names, as `sha256.init` became
`sha256.make`, and std's `MIGRATION.md` maps each name of `std.data.json` and
`std.data.toml` to the module that holds it now. `std.crypto.ct`,
`std.crypto.rand`, `std.crypto.hash.hasher`, `fnv1a` and `siphash` stay in std.
`std.types.semver` left std for no package: version handling is mach's own.

New in 7, with nothing to migrate: [`u1`](types.md), [`$zeroize`](comptime-intrinsics.md)
and the [selection heads](asm.md) of `asm`.
