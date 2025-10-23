# Mach documentation

This directory is the authoritative home of the Mach language specification and project guides. The self-hosted compiler will live in this repository; keeping the language docs alongside the source ensures they evolve together.

## Contents

- [getting-started.md](./getting-started.md) &mdash; Installation, build instructions, and running your first Mach program.
- [language-spec.md](./language-spec.md) &mdash; Complete language reference covering syntax, semantics, intrinsics, and module conventions.
- [project-layout.md](./project-layout.md) &mdash; How `mach.toml`, module mapping, and the `Makefile` fit together.

When the language changes, update `language-spec.md` first, then adjust the supplemental guides as needed. The bootstrap compiler (`cmach`) should continue to match the spec until the self-hosted compiler takes over.
