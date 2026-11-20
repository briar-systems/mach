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
- A workflow step's `run: '...'` or `run: "..."` is read inside its quotes, and a
  `run: |` block line by line.

Not recognised: mach reached through any other variable or alias, a command word
after `exec`, `env`, `time`, `sudo`, `if` or another prefix, a function or script
that wraps mach, and a file that is not a script by the rules above. A here-document
is not recognised as one: its lines are read as commands like any other line. An
option in another program's words, as `cc -O2`, or in a comment is never reported.

## Exit code

- `0`: the project is in this major's spelling, or was made so. Script and step
  reports leave it at `0`: they name files the migration never edits.
- `1`: under `--check`, a file would change. In either mode, the manifest still
  writes a removed key no rewrite migrates, each one reported, or a source could
  not be read as source. The manifest reader refuses such a project, so the input
  is what is wrong. So does a system library 6 found that the manifest does not
  yet say how 7 finds, each one reported, since the link refuses it. A missing or
  invalid path, or a migrated manifest the reader refuses, also exits `1`, as for
  every command.
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
| `-g`, `-O0`, `-O2`, `--emit-asm`, `--emit-ir`, `--pie`, `--simd`, `--jobs`, `--arch`, `--sys`, `--of`, `--platform`, `--subsystem`, `--base`, `--stack-reserve`, `--stack-commit`, `-d`, `-e`, `-s` in a script, or in a step's `argv` other than `--jobs` | the spelling the report names |
| a linux target with no `env` that an artifact links a `source = "system"` entry on, which 6 found in the gnu environment's directories | `env = "gnu"` on the target, under which a `bin` links `pie` unless its `linkage` says otherwise, so the report names the target and the entry |
| a darwin `source = "system"` shared entry named by no install name, as `name = "MoltenVK"`, whose file 6 found and linked by the install name it states | the install name, as `name = "@rpath/libMoltenVK.dylib"` or an absolute path, which 7 links from the entry's claim with no file |

The compiler refuses the first two, and `mach migrate` reports the third. A `u8`
condition is refused as `condition.type` and a `u1` under an operator as
`type.mismatch`, each naming the replacement. A `u1` stored in a `u8` is refused as
a plain `type.mismatch`, `expected u8, found u1`. The manifest reader refuses a
removed key with no mechanical rewrite, and `mach migrate` reports each one with
its line: a link's `library`, which the table key replaces, a profile's `pass` and
`skip`, an `opt` other than `0`, `1` or `2`, and a step's `cmd` and `shell`, which
`argv` replaces. A table that writes such a key still gets every other rewrite, and
the key is reported once. The link refuses the last two: neither has one answer
migrate can write, since `env` changes how every `bin` of the target links and the
install name is the library's own. A report names the line of the table it is about.

New in 7, with nothing to migrate: [`u1`](types.md), [`$zeroize`](comptime-intrinsics.md)
and the [selection heads](asm.md) of `asm`.
