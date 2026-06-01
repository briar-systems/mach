# Mach language skills

Authoring skills for the Mach (`.mach`) language. Each skill is a fast path for
one slice of the language; together they cover writing correct Mach without
reaching for the full reference on every line.

## Skills

| Skill | Purpose | Triggers when |
|---|---|---|
| [writing-mach](writing-mach/SKILL.md) | Core language: project/module structure, `use`/`fwd`, the shadow-module pattern, every declaration form, the type grammar, literals, operators, statements, expressions, docstrings. | Writing, editing, or reviewing any `.mach` source. The default entry point. |
| [mach-comptime](mach-comptime/SKILL.md) | The comptime channel (`$`): `$mach.*` reads, `$sym.attr` symbol attributes, the closed intrinsic set, `$if`/`$or` control flow, and `$name: T` comptime parameters. | Code touches `$` — conditional compilation, target/build queries, attributes, intrinsics, or comptime params. |
| [mach-lowlevel](mach-lowlevel/SKILL.md) | Inline assembly (`asm <isa> { ... }` with `{name}` substitution, multi-arch dispatch, inferred operands/clobbers) and the compiler-vs-stdlib placement policy. | Writing/reviewing inline `asm`, or deciding whether an operation belongs in the compiler or stdlib. |

Scope is partitioned: core language lives in **writing-mach**, the `$` channel in
**mach-comptime**, and `asm` plus the placement policy in **mach-lowlevel**.
writing-mach carries a one-paragraph summary of the `$` channel and defers the
rest to mach-comptime; the low-level skill restates a few core gotchas as
reminders and points back to writing-mach for the full rules.

## Authoritative references

The skills are the fast path, not the contract. For exhaustive detail:

- [`doc/language/*.md`](../../doc/language/) — the complete per-feature reference.
  Each skill links the relevant files in its own Reference section.
- [`SPEC.md`](../../SPEC.md) — locked-decision source of truth. When a skill and
  SPEC.md disagree, SPEC.md wins, and the skill is the bug.
