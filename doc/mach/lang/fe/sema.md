# mach.lang.fe.sema

Semantic analysis. Consumes a [`ResolveResult`](resolve.md#resolveresult)
plus the parsed [`Ast`](ast.md#ast) and produces a
[`SemaResult`](#semaresult) — parallel arrays mapping every AST node id
to its semantic [`TypeId`](../type.md#typeid). All interned types live
in the shared [`TypeInterner`](../type.md#typeinterner) on the
[session](../session.md); `SemaResult` owns only the per-AST id-to-type
mappings.

Sub-modules:

- [`sema.infer`](sema/infer.md) — bottom-up type inference for expressions and type-references.
- [`sema.check`](sema/check.md) — declared-vs-inferred compatibility checks; assignment and call validation.
- [`sema.coerce`](sema/coerce.md) — implicit coercion rules (literals to declared types; nothing else).
- [`sema.generics`](sema/generics.md) — per-use-site monomorphisation; interns [`TYPE_INSTANCE`](../type.md#typekind) entries.

## Types

### `SemaResult`

```mach
pub rec SemaResult {
    alloc:            *Allocator;
    expr_type:        *type.TypeId;
    type_resolved_ty: *type.TypeId;
    decl_type:        *type.TypeId;
}
```

Per-module typing data.

| Field             | Type                                          | Description                                                              |
|-------------------|-----------------------------------------------|--------------------------------------------------------------------------|
| alloc             | `*Allocator`                                  | Allocator backing every owned array on the result.                       |
| expr_type         | [`*type.TypeId`](../type.md#typeid)           | Parallel to [`ast.exprs`](ast.md#ast); the semantic type of each expression. [`TYPE_ERROR`](../type.md#typekind) at positions that failed to typecheck. |
| type_resolved_ty  | [`*type.TypeId`](../type.md#typeid)           | Parallel to [`ast.types`](ast.md#ast); the interned semantic [`TypeId`](../type.md#typeid) for each syntactic [`AstType`](ast/type.md#type). |
| decl_type         | [`*type.TypeId`](../type.md#typeid)           | Parallel to [`ast.decls`](ast.md#ast); the declared (or inferred) type of each binding decl. [`TYPE_NIL`](../type.md#type_nil) for decls that don't carry a type (use, comptime_if, comptime_attr, test, error). |

### `ModuleSema`

```mach
pub rec ModuleSema {
    path:       intern.StrId;
    module:     sess.ModuleId;
    decl_type:  *type.TypeId;
    decl_count: u32;
}
```

A snapshot of one previously-sema'd module's `decl_type` array,
exposed to dependents so cross-module references can look up signature
[`TypeId`](../type.md#typeid)s without dragging in the whole
[`SemaResult`](#semaresult).

| Field      | Type                                          | Description                                       |
|------------|-----------------------------------------------|---------------------------------------------------|
| path       | [`intern.StrId`](../intern.md#strid)          | Interned FQN of the module.                       |
| module     | [`sess.ModuleId`](../session.md#moduleid)     | ModuleId of the owning module.                    |
| decl_type  | [`*type.TypeId`](../type.md#typeid)           | Borrowed reference to the producer's [`SemaResult.decl_type`](#semaresult). |
| decl_count | `u32`                                         | Length of the borrowed array.                     |

### `SemaDeps`

```mach
pub rec SemaDeps {
    entries: *ModuleSema;
    count:   u32;
}
```

Every dependency's typing snapshot, supplied by the
[driver](../driver.md). Sema looks these up by interned path when
resolving cross-module symbol types.

| Field   | Type                                          | Description                          |
|---------|-----------------------------------------------|--------------------------------------|
| entries | [`*ModuleSema`](#modulesema)                  | Array of [`ModuleSema`](#modulesema) snapshots, one per loaded dep. |
| count   | `u32`                                         | Number of entries.                   |

### `SemaContext`

```mach
rec SemaContext {
    s:          *sess.Session;
    a:          *ast.Ast;
    rr:         *resolve.ResolveResult;
    deps:       *SemaDeps;
    ctx:        *comptime.ComptimeCtx;
    own_module: sess.ModuleId;
    source:     str;

    expr_type:        *type.TypeId;
    type_resolved_ty: *type.TypeId;
    decl_type:        *type.TypeId;
}
```

Sema's private working state. Owned by [`sema`](#sema) for the duration
of one module-resolve; never escapes. [`build_result`](#internal-helpers)
transfers the three parallel arrays into the returned
[`SemaResult`](#semaresult).

## Functions

### `sema`

```mach
pub fun sema(
    s:          *sess.Session,
    a:          *ast.Ast,
    rr:         *resolve.ResolveResult,
    deps:       *SemaDeps,
    ctx:        *comptime.ComptimeCtx,
    own_module: sess.ModuleId,
) Result[SemaResult, str]
```

Runs semantic analysis on one module. Phases run in order:

1. Allocate the three parallel arrays sized to [`ast.expr_len`](ast.md#ast)
   / [`ast.type_len`](ast.md#ast) / [`ast.decl_len`](ast.md#ast),
   prefilled with [`TYPE_NIL`](../type.md#type_nil).
2. Resolve every syntactic [`AstType`](ast/type.md#type) into the
   shared [`TypeInterner`](../type.md#typeinterner) via
   [`infer.resolve_type_ref`](sema/infer.md#resolve_type_ref); record
   the result in [`type_resolved_ty`](#semacontext).
3. Compute the declared type of every decl via
   [`infer.infer_decl`](sema/infer.md#infer_decl); record in
   [`decl_type`](#semacontext).
4. Walk every expression in source order via
   [`infer.infer_expr`](sema/infer.md#infer_expr); record results in
   [`expr_type`](#semacontext). [`check`](sema/check.md) functions are
   invoked at each composing-site (call args, assignment targets,
   return values).

Type errors are pushed into [`s.diags`](../session.md#session) as
[`SEVERITY_ERROR`](../diagnostic.md#constants) diagnostics; the
position with the failure has its `expr_type` (or `decl_type` or
`type_resolved_ty`) set to [`TYPE_ERROR`](../type.md#typekind). Sema
does not halt on the first error.

| Param      | Type                                                  | Description                                                |
|------------|-------------------------------------------------------|------------------------------------------------------------|
| s          | [`*sess.Session`](../session.md#session)              | Shared session (allocator, sources, diagnostics, interner, type universe). |
| a          | [`*ast.Ast`](ast.md#ast)                           | Parsed AST for the module being analysed.                  |
| rr         | [`*resolve.ResolveResult`](resolve.md#resolveresult)  | Name-resolution output for the same module.                |
| deps       | [`*SemaDeps`](#semadeps)                              | Typing snapshots of every dependency, populated by the driver. |
| ctx        | [`*comptime.ComptimeCtx`](comptime.md#comptimectx)    | Comptime context for the module — required to evaluate array-length expressions during [`resolve_type_ref`](sema/infer.md#resolve_type_ref). |
| own_module | [`sess.ModuleId`](../session.md#moduleid)             | Project-wide ModuleId of the module being analysed.        |

Returns the populated [`SemaResult`](#semaresult), or an allocation
error.

### `dnit_result`

```mach
pub fun dnit_result(r: *SemaResult)
```

Releases the parallel arrays held by the result and zeroes its fields.
`nil` is a no-op. Called by the [driver](../driver.md) when a module's
sema result is no longer needed.

| Param | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| r     | [`*SemaResult`](#semaresult)          | Result to tear down. `nil` is a no-op. |

## Phase order

```
allocate parallel arrays
  ↓
for each AstType in source order:
  type_resolved_ty[ti] = infer.resolve_type_ref(sc, ti)
  ↓
for each Decl in source order:
  decl_type[di] = infer.infer_decl(sc, di)
  ↓
for each Decl in source order (whose decl_type is set):
  walk body / initializer with infer.infer_expr; check.* validates at composing sites
  ↓
build_result transfers parallel arrays into SemaResult
```

Phase 2 (decl types) precedes phase 3 (body inference) so that
recursive declarations work: a function whose body references itself
must have its signature interned before the body is walked. The order
within each phase is source order; combined with resolve's
[binding-before-use](resolve.md#pass-2-bind) discipline, this gives
let-not-letrec semantics for locals.

## Diagnostics

Sema produces these categories of diagnostics; consumers may filter or
group by message prefix.

| Category prefix                    | When emitted                                                  |
|------------------------------------|---------------------------------------------------------------|
| `type mismatch:`                   | An expression's inferred type is not compatible with the expected one (declared annotation, call parameter, assignment LHS, return type). |
| `cannot infer type:`               | A position needs a type but neither annotation nor evidence is available. |
| `not callable:`                    | A call target is not a function-typed value.                  |
| `arity mismatch:`                  | A call supplies the wrong number of arguments for the callee's signature. |
| `no such field:`                   | Member access on a record/union type with no matching field.  |
| `not indexable:`                   | An index expression on a non-array, non-pointer type.         |
| `cast:` ... `invalid`              | A `::` cast violates the [cast rules](#cast-resolution).      |
| `generic arg count:`               | A generic name reference supplies the wrong number of args.   |
| `unresolved type:`                 | Sema encountered a [`SYMBOL_NIL`](resolve.md#constants) in a type position; the upstream resolve error already reported the root cause. |

## Cast resolution

The `::` cast operator follows two rules drawn from the
[language reference](../../../language/types.md):

- **Primitive numeric** (int↔int, float↔float, int↔float) — performs
  the corresponding conversion; the type-system check is that both
  source and destination are primitives.
- **Non-primitive bit reinterpretation** — `from::To` is permitted iff
  `from` and `to` have the same size in bytes. Sema computes the size
  from the [`TypeInterner`](../type.md#typeinterner) via
  [`check.size_of`](sema/check.md#internal-helpers). Mismatched sizes
  emit `cast: size mismatch (M vs N bytes)`.

## Internal helpers

File-private; listed for reference.

| Function        | Role                                                                       |
|-----------------|----------------------------------------------------------------------------|
| `init_context`  | Allocate the three parallel arrays + seed [`SemaContext`](#semacontext).   |
| `dnit_context`  | Reverse of `init_context`. Called on error paths; folded into `build_result` on success. |
| `fill_nil`      | Pre-fills a `*TypeId` array of `len` slots with [`TYPE_NIL`](../type.md#type_nil). |
| `build_result`  | Allocates output, copies arrays in, transfers ownership, dnits context.    |
| `find_module_sema` | Linear scan of [`deps.entries`](#semadeps) by interned path.            |
| `decl_type_for` | Cross-module dispatch — local resolve when [`SYM_*`](resolve.md#constants) is from `own_module`, dep snapshot lookup otherwise. |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`,
[`mach.lang.session`](../session.md),
[`mach.lang.diagnostic`](../diagnostic.md),
[`mach.lang.intern`](../intern.md), [`mach.lang.type`](../type.md),
[`mach.lang.fe.ast`](ast.md),
[`mach.lang.fe.ast.id`](ast/id.md),
[`mach.lang.fe.ast.expr`](ast/expr.md),
[`mach.lang.fe.ast.stmt`](ast/stmt.md),
[`mach.lang.fe.ast.decl`](ast/decl.md),
[`mach.lang.fe.ast.type`](ast/type.md),
[`mach.lang.fe.ast.module`](ast/module.md),
[`mach.lang.fe.token`](token.md),
[`mach.lang.fe.comptime`](comptime.md),
[`mach.lang.fe.resolve`](resolve.md),
[`mach.lang.fe.sema.infer`](sema/infer.md),
[`mach.lang.fe.sema.check`](sema/check.md),
[`mach.lang.fe.sema.coerce`](sema/coerce.md),
[`mach.lang.fe.sema.generics`](sema/generics.md).
