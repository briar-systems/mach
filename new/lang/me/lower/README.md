# lang/me/lower

AST-to-IR lowering, split by AST node category. `lower.mach` in the parent directory is the public entry point.

## Files

- `decl.mach` — lowers function bodies, struct layouts, and global initializers.
- `stmt.mach` — lowers statements into IR control flow.
- `expr.mach` — lowers expressions into IR values and instructions.
