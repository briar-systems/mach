# lang/fe/ast

AST node definitions, split by syntactic category. `ast.mach` in the parent directory re-exports the public surface.

## Files

- `id.mach` — stable u32 identifier types (`ExprId`, `StmtId`, `DeclId`, `TypeId`, `ModuleId`) with their `*_NIL` sentinels. Kept separate so category modules can cross-reference each other without import cycles.
- `module.mach` — the top-level module container.
- `decl.mach` — declarations: functions, records, bindings, type aliases, `use` imports, tests. Shared `TypedName` record used for parameters and fields.
- `stmt.mach` — statements: blocks, control flow, returns, `fin` defers, wrapped decls.
- `expr.mach` — expressions: literals, operators, calls, indexing, member access, casts, array and struct literals. Binary and unary operator tables. Shared `FieldInit` record used by struct literals.
- `type.mach` — syntactic type references as written in source. The semantic type system lives in `lang/me/ir/type.mach`.

Every AST node carries a source span. Spans are non-optional: position-indexed queries (hover, go-to-definition, find-references) depend on complete span coverage.
