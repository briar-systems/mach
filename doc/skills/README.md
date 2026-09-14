# Mach language skill

[mach](mach/SKILL.md) is the authoring skill for the Mach (`.mach`) language. It provides a dense single file reference covering the core language, the comptime channel, decorators, and inline assembly. It triggers whenever Mach source is written, edited, or reviewed. It is a reference guide for writing Mach programs rather than working on the compiler.

## Adopting this skill

Drop the `skills/` tree into your project tooling directory, such as `.agents/skills/mach/`, or install it into your global agent configuration directory. The skill is self-contained, referencing only the language itself and the Mach standard library (`std.*`) with no dependency on compiler internals.

Keep it synchronized with your toolchain. When language contracts or standard library APIs move, update the skill to match.

## Authoritative reference

The skill is a practical guide for code generation, not the formal specification. The complete per-feature language reference covering grammar, semantics, and implementation contracts lives in the Mach repository under [`doc/language/`](../language/README.md). When the skill and the reference disagree, the reference wins.
