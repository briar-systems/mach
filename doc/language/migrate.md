# Migrating a project

`mach migrate [<path>] [--check]` moves a project from the previous major to the
current one. It rewrites the root project's `mach.toml`, then every source file
under its `[project].src`, and reports what it cannot rewrite.

- It is idempotent: a migrated project changes nothing.
- `--check` writes nothing, prints each change as a diff, and exits non-zero when
  anything would change, so CI can confirm a project is migrated.
- It works on the root project only. Dependencies under `dep/` belong to other
  projects and are never edited, and one still on the previous major is reported.
- A script that runs `mach` with a removed option (a `Makefile`, `build.sh`, or
  `.github/workflows/ci.yml` and `cd.yml`) is reported with its file and line, never
  rewritten.
- Rules are data rows tagged with the major they migrate from. A release carries the
  rows of the major before it, and the next major deletes them.
- Only tokens of source are read, so a comment or a string that spells an old form is
  left as written.

## The changes from 6 to 7

Rewritten by `mach migrate`:

| 6 | 7 |
|---|---|
| `opt`, `float_reassoc` | `optimize`, `relax` |
| `vectorize = true` or `vectorize = false` in a profile | removed: `optimize` schedules the pass, and `#[vectorize("never")]` opts one function out |
| `[project] out`, `{project.out}` | `work`, `{project.work}` |
| `os`, `baseline`, `extensions` | `sys`, `isa`, `ext` |
| link `os`, `isa` | link `sys`, `arch` |
| `$mach.os`, `$mach.build.os`, `$mach.build.extensions`, `$project.target.os` | `$mach.sys`, `$mach.build.sys`, `$mach.build.ext`, `$project.target.sys` |
| `#[op]`, `#[stage]` and the shader decorators | `#[intrinsic]`, `#[gpu.stage]` and the `gpu.` family |
| `#[inline]`, `#[noinline]`, `#[scalar]` | `#[inline("always")]`, `#[inline("never")]`, `#[vectorize("never")]` |
| `#[oblivious]` | `#[constant_time("soft")]` |
| `#[ext(sha)]`, `#[extensions(sha)]` | `#[extension("sha")]` |
| `asm x86_64`, `asm aarch64`, `asm riscv64`, `asm riscv32` | `asm x86-64`, `asm armv8-a`, `asm rv64i`, `asm rv32i` |
| `def bool: u8;` | `def bool: u1;` |
| `use id;`, `fwd id;` | the path of the dependency's exported artifact entry |
| `--jobs`, `--jobs=n` in the `argv` of a `[step]` that runs mach | `--threads`, `--threads=n` |
| `[project] mach` on the previous major | the running major |

Reported, for you to change:

| 6 | 7 |
|---|---|
| a condition of type `u8`, as `if (flags & 1)` | a `u1`, as `if ((flags & 1) != 0)` |
| a `u1` stored in a `u8` or under arithmetic | `value::u8`, or a `u1` destination |
| `-g`, `-O0`, `-O2`, `--emit-asm`, `--emit-ir`, `--pie`, `--simd`, `--jobs`, `--arch`, `--sys`, `--of`, `--platform`, `--subsystem`, `--base`, `--stack-reserve`, `--stack-commit`, `-d`, `-e`, `-s` in a script | the spelling the report names |

The compiler refuses the first two, naming the replacement.

New in 7, with nothing to migrate: [`u1`](types.md), [`$zeroize`](comptime-intrinsics.md)
and the [selection heads](asm.md) of `asm`.
