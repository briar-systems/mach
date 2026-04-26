# lang/me/ir

IR data structures and operations. `ir.mach` in the parent directory re-exports the public surface.

## Files

- `type.mach` — IR type system. Structurally interned via `lang/intern.mach` so type equality is a `TypeId` comparison.
- `value.mach` — SSA values, uses, and use-def chains.
- `instruction.mach` — the instruction set: opcodes, operands, and results.
- `builder.mach` — ergonomic construction API for IR modules and functions.
- `printer.mach` — textual rendering of IR for debugging, diagnostics, and tests.
- `verify.mach` — structural invariants and well-formedness checks.
