# lang

The Mach language implementation: source text through object file emission, plus the cross-cutting services that thread through every phase.

## Phases

Compilation flows through three phase groups:

- `fe/` — **frontend.** Source text to typed AST.
- `me/` — **middle-end.** Typed AST to optimized IR.
- `be/` — **backend.** IR to object files.

## Cross-cutting services

These live at the top of `lang/` because every phase consumes them.

- `session.mach` — the driver and query surface. Owns all compilation state and exposes memoized queries consumed by the CLI, LSP, and test infrastructure.
- `query.mach` — the memoization engine underpinning the session's query system.
- `diagnostic.mach` — diagnostic sink, source-span rendering, and reporter interface.
- `source.mach` — source file registry. Maps paths to loaded text and byte offsets to `(line, column)` positions.
- `intern.mach` — interners for identifiers, types, paths, and spans. Produces stable integer handles used throughout the compiler.
- `target.mach` — the `Target` record: a composition of ISA, ABI, OS, and object-format implementations drawn from `target/`.

## Design principles

Phases are pure functions of explicit inputs to explicit outputs. Inter-phase communication flows through data records, never shared mutable state. The session wires phases together by composing queries; no phase calls another phase directly. This shape makes the compiler usable as a batch driver, an LSP backend, and a test fixture from the same codebase.
