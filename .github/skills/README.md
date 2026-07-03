# Mach language skills

Authoring skills for the Mach (`.mach`) language. Each skill is a fast path for
one slice of the language; together they cover writing correct Mach without
reaching for the full reference on every line. They are reference/usage guides
for **writing Mach programs**, not for working on the compiler.

## Skills

| Skill | Purpose | Triggers when |
|---|---|---|
| [writing-mach](writing-mach/SKILL.md) | Core language: project/module structure, `use`/`fwd`, the shadow-module pattern, every declaration form (including `test`), variadic packs, the type grammar and the `^` secret qualifier, literals, operators and casts, statements, docstrings, and the entrypoint / `std.print` idioms. | Writing, editing, or reviewing any `.mach` source. The default entry point. |
| [mach-comptime](mach-comptime/SKILL.md) | The comptime channel (`$`): `$mach.*`/`$project.*`/`$bin.*` reads, `#[...]` decorators, the closed intrinsic set (`$size_of`, `$type_of`, `$fields`, `$error`, …), `$if`/`$or` conditional compilation, `$each` unrolls, and `$name: T` comptime parameters. | Code touches `$` or `#[...]` — conditional compilation, target/build queries, decorators, intrinsics, packs, or comptime params. |
| [mach-lowlevel](mach-lowlevel/SKILL.md) | Inline assembly (`asm <isa> { ... }` for x86_64/aarch64/riscv64 with `{name}` substitution and inferred operands/clobbers), multi-arch dispatch, module target guards, and when to write `asm` versus call the standard library. | Writing/reviewing inline `asm`, syscalls, or other target-specific code. |

Scope is partitioned: core language lives in **writing-mach**, the `$` channel
and decorators in **mach-comptime**, and `asm` in **mach-lowlevel**.
writing-mach carries a one-paragraph summary of the `$` channel and defers the
rest to mach-comptime; the low-level skill restates a few core gotchas as
reminders and points back to the others for the full rules.

## Adopting these skills

Drop the `.github/skills/` tree into your own Mach project's tooling directory
(for an agent or assistant that reads skills, the conventional location is
`.github/skills/`). The skills are self-contained — they reference only the
language itself and the public Mach standard library (`std.*`), with no
dependency on this or any other particular repository's internals.

Keep them current with your toolchain: when the locked language or the standard
library moves, update the affected skill in step. When a skill and the official
language reference disagree, the reference wins and the skill is the bug.

## Authoritative reference

The skills are the fast path, not the contract. The complete, per-feature
language reference — grammar, semantics, and implementation status — lives in
the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language).
Each skill points there for exhaustive detail.
