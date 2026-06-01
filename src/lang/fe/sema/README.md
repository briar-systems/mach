# lang/fe/sema

Semantic analysis: name-bound AST to typed AST. `sema.mach` in the parent directory is the public entry point.

## Files

- `infer.mach` — type inference for expressions and bindings.
- `check.mach` — type checking. Validates that inferred and declared types are compatible and reports mismatches.
- `coerce.mach` — implicit and explicit coercion rules.
- `generics.mach` — monomorphization of generic functions and types.

Semantic analysis tolerates holes: unresolved names produce an error type that absorbs further operations without cascading diagnostics, so a single upstream mistake does not drown the reporter in derived errors.
