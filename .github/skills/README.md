# Mach language skill

[mach](mach/SKILL.md) is the authoring skill for the Mach (`.mach`) language —
a single fast path covering the core language, the comptime channel and
`#[...]` decorators, and inline assembly. It triggers whenever Mach source is
written, edited, or reviewed. It is a reference/usage guide for **writing Mach
programs**, not for working on the compiler.

## Adopting this skill

Drop the `.github/skills/` tree into your own Mach project's tooling directory
(for an agent or assistant that reads skills, the conventional location is
`.github/skills/`). The skill is self-contained — it references only the
language itself and the public Mach standard library (`std.*`), with no
dependency on this or any other particular repository's internals.

Keep it current with your toolchain: when the locked language or the standard
library moves, update the skill in step.

## Authoritative reference

The skill is the fast path, not the contract. The complete, per-feature
language reference — grammar, semantics, and implementation status — lives in
the Mach repository under
[`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language).
When the skill and the reference disagree, the reference wins and the skill is
the bug.
