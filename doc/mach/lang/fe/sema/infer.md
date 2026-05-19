# mach.lang.fe.sema.infer

Bottom-up type inference. Computes the semantic [`TypeId`](../../type.md#typeid)
of every expression and the interned [`TypeId`](../../type.md#typeid)
that each syntactic [`AstType`](../../ast/type.md#type) refers to.
Drives the rest of sema by calling [`check`](check.md) at composing
sites and [`coerce`](coerce.md) where the language allows implicit
narrowing.

## Functions

### `infer_decl`

```mach
pub fun infer_decl(sc: *sema.SemaContext, did: id.DeclId) type.TypeId
```

Computes the declared type of one decl. Per [`DeclKind`](../../ast/decl.md#declkind):

| Kind                       | Returned type                                                              |
|----------------------------|----------------------------------------------------------------------------|
| `FUN`                      | An interned [`TYPE_FUN`](../../type.md#typekind) for the signature.        |
| `REC`                      | An interned [`TYPE_REC`](../../type.md#typekind) nominal type.             |
| `VAL` / `VAR`              | The annotated [`TypeId`](../../type.md#typeid), or — when the annotation is [`TYPE_NIL`](../../ast/id.md#constants) — the inferred type of the initializer. |
| `DEF`                      | Resolves to the underlying type; **transparent** — no `TYPE_DEF` produced (see [type.md](../../type.md)). |
| `TEST`                     | [`TYPE_NIL`](../../type.md#type_nil) (tests aren't typed; the runner inspects them through resolve). |
| `USE` / `COMPTIME_*`       | [`TYPE_NIL`](../../type.md#type_nil).                                      |

| Param | Type                                          | Description                  |
|-------|-----------------------------------------------|------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                |
| did   | [`id.DeclId`](../../ast/id.md#declid)         | Decl to compute the type of. |

### `infer_expr`

```mach
pub fun infer_expr(sc: *sema.SemaContext, eid: id.ExprId) type.TypeId
```

Computes the type of one expression. Recursively infers
sub-expressions, dispatches per [`ExprKind`](../../ast/expr.md#exprkind),
writes the result into [`sc.expr_type[eid]`](../sema.md#semacontext),
and returns it.

| Param | Type                                          | Description                  |
|-------|-----------------------------------------------|------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                |
| eid   | [`id.ExprId`](../../ast/id.md#exprid)         | Expression to type.          |

### `resolve_type_ref`

```mach
pub fun resolve_type_ref(sc: *sema.SemaContext, tid: id.TypeId) type.TypeId
```

Translates a syntactic [`AstType`](../../ast/type.md#type) into an
interned semantic [`TypeId`](../../type.md#typeid). Walks the
[`TypeKind`](../../ast/type.md#typekind) variants and dispatches
to the matching [`type.intern_*`](../../type.md#functions). Writes
the result into
[`sc.type_resolved_ty[tid]`](../sema.md#semacontext).

| Param | Type                                          | Description                          |
|-------|-----------------------------------------------|--------------------------------------|
| sc    | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                        |
| tid   | [`id.TypeId`](../../ast/id.md#typeid)         | Syntactic type to resolve.           |

## Inference rules

### Per `ExprKind`

| `ExprKind`              | Type rule                                                                                                                                                                |
|-------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `LIT_INT`               | Defaults to [`TYPE_I64`](../../type.md#constants); coerced to a different integer type by [`coerce`](coerce.md#try_coerce_literal) when a target type is supplied.       |
| `LIT_FLOAT`             | Defaults to [`TYPE_F64`](../../type.md#constants); coercible to `TYPE_F32` at a typed site.                                                                              |
| `LIT_CHAR`              | [`TYPE_U8`](../../type.md#constants).                                                                                                                                    |
| `LIT_STR`               | [`TYPE_STR`](../../type.md#constants) singleton.                                                                                                                         |
| `LIT_ZSTR`              | [`*u8`](../../type.md#typepointer) — interns via [`type.intern_pointer(TYPE_U8)`](../../type.md#intern_pointer).                                                         |
| `LIT_NIL`               | An untyped-pointer marker; takes its type from the typed site (assignment LHS / parameter type / cast target). When no site is available, defaults to `*u8` and emits an info diagnostic. |
| `IDENT`                 | The type of the resolved [`SymbolId`](../resolve.md#symbolid) (looked up via [`sc.rr.expr_resolved[eid]`](../resolve.md#resolveresult) and [`decl_type_for`](../sema.md#internal-helpers)). |
| `COMPTIME_IDENT`        | Not allowed in value position — emits `"comptime identifier in value context"`.                                                                                          |
| `BINARY`                | Per-operator type table; see [Binary operators](#binary-operators).                                                                                                      |
| `UNARY`                 | See [Unary operators](#unary-operators).                                                                                                                                 |
| `CALL`                  | Callee must be [`TYPE_FUN`](../../type.md#typefun); checks arity + each arg via [`check.check_assignable`](check.md#check_assignable); result is the function's return type. |
| `INDEX`                 | Object must be [`TYPE_POINTER`](../../type.md#typekind) or [`TYPE_ARRAY`](../../type.md#typekind); index must be an integer; result is the element type.                 |
| `MEMBER`                | Object type must be [`TYPE_REC`](../../type.md#typekind) or [`TYPE_UNI`](../../type.md#typekind); look up the field by name; result is the field's type. Field tables for built-in records (e.g. `str`) are handled by [field-table](#field-tables). |
| `CAST`                  | [`check.check_cast`](check.md#check_cast).                                                                                                                               |
| `ARRAY_LIT`             | Result is [`TYPE_ARRAY(elem, count)`](../../type.md#typearray) of the declared element type; each element checked against `elem` via [`check.check_assignable`](check.md#check_assignable). |
| `STRUCT_LIT`            | Result is the named [`TYPE_REC`](../../type.md#typekind); each [`FieldInit`](../../ast/expr.md#fieldinit) checked against the record's field of the same name. |
| `ERROR`                 | [`TYPE_ERROR`](../../type.md#typekind) — absorbs further operations without cascading diagnostics.                                                                       |

### Binary operators

| [`BinOp`](../../ast/expr.md#binop)                          | Operand rule                                | Result                          |
|-------------------------------------------------------------|---------------------------------------------|---------------------------------|
| `BIN_ADD`, `BIN_SUB`, `BIN_MUL`, `BIN_DIV`, `BIN_MOD`       | both operands same numeric type             | that type                       |
| `BIN_EQ`, `BIN_NEQ`                                         | both operands same type                     | [`TYPE_U8`](../../type.md#constants) (boolean) |
| `BIN_LT`, `BIN_LEQ`, `BIN_GT`, `BIN_GEQ`                    | both operands same numeric type             | [`TYPE_U8`](../../type.md#constants)            |
| `BIN_AND`, `BIN_OR`                                         | both operands [`TYPE_U8`](../../type.md#constants) | [`TYPE_U8`](../../type.md#constants)            |
| `BIN_BIT_AND`, `BIN_BIT_OR`, `BIN_BIT_XOR`, `BIN_SHL`, `BIN_SHR` | both operands same integer type        | that type                       |
| `BIN_ASSIGN`                                                | LHS is assignable; RHS assignable to LHS    | LHS type                        |

### Unary operators

| [`UnOp`](../../ast/expr.md#unop) | Operand rule                                  | Result                                                |
|----------------------------------|-----------------------------------------------|-------------------------------------------------------|
| `UN_NEG`                         | operand is a numeric type                     | the operand's type                                    |
| `UN_NOT`                         | operand is [`TYPE_U8`](../../type.md#constants) | [`TYPE_U8`](../../type.md#constants)                |
| `UN_BIT_NOT`                     | operand is an integer type                    | the operand's type                                    |
| `UN_ADDR`                        | operand is an lvalue                          | [`type.intern_pointer(operand_type)`](../../type.md#intern_pointer) |
| `UN_DEREF`                       | operand is [`TYPE_POINTER`](../../type.md#typekind) | the pointee type                              |

### Type-reference dispatch

[`resolve_type_ref`](#resolve_type_ref) per [`AstType.kind`](../../ast/type.md#typekind):

| `TYPE_KIND_*` | Action                                                                                                  |
|---------------|---------------------------------------------------------------------------------------------------------|
| `NAMED`       | Looks up the dotted path via [`resolve`](../resolve.md)'s recorded symbol; if generic args present, [`generics.instantiate`](generics.md#instantiate). |
| `PTR`         | Recurses on `pointee`; returns [`intern_pointer`](../../type.md#intern_pointer).                        |
| `ARRAY`       | Recurses on `element` for the elem type; evaluates `length` via [`comptime.eval`](../comptime.md#eval) and demands `CT_KIND_INT`; returns [`intern_array`](../../type.md#intern_array). |
| `FUN`         | Recurses on each param + ret; returns [`intern_function`](../../type.md#intern_function).               |
| `REC` / `UNI` | Anonymous record/union types — synthesises an unnamed nominal entry (see [Anonymous record/union types](#anonymous-recunion-types)). |
| `ERROR`       | [`TYPE_ERROR`](../../type.md#typekind).                                                                 |

## Field tables

For nominal types (records, unions), sema needs a structural view of
the fields to type member-access expressions. Each `TYPE_REC` /
`TYPE_UNI` instance is augmented with a side-table indexed by
[`TypeId`](../../type.md#typeid) → array of `(name, TypeId)` pairs.

For the builtin [`TYPE_STR`](../../type.md#constants) singleton, the
table is seeded at sema startup:

| Field  | Type                                          |
|--------|-----------------------------------------------|
| `len`  | [`TYPE_U32`](../../type.md#constants)         |
| `data` | [`*u8`](../../type.md#intern_pointer)         |

Field tables live on the [`SemaContext`](../sema.md#semacontext) (not
on the shared [`TypeInterner`](../../type.md#typeinterner)) because
they're per-module — different modules may resolve nominal types to
the same `(origin, decl)` pair but they each compute the field table
during their own sema pass. (Field tables are append-only and
identity-equal across modules; the duplication is small and avoids
cross-module sharing of mutable state.)

## Anonymous rec/union types

Anonymous `rec { ... }` / `uni { ... }` types written inline (e.g. in
a function parameter `arg: rec { x: i32; y: i32; }`) get fresh
nominal entries keyed by `(own_module, DECL_NIL)` with a synthetic
positional name (`__anon_rec_<n>`). Two textually-identical inline
records in different positions get distinct [`TypeId`](../../type.md#typeid)s
(nominal typing).

## Internal helpers

File-private; listed for reference.

| Function                | Role                                                                       |
|-------------------------|----------------------------------------------------------------------------|
| `infer_lit_int` / `_float` / `_char` / `_str` / `_zstr` / `_nil` | Per-literal type rule.        |
| `infer_ident`           | Consults [`sc.rr.expr_resolved`](../resolve.md#resolveresult); maps the [`SymbolId`](../resolve.md#symbolid) to its declared type. |
| `infer_binary`          | Dispatches per [`BinOp`](../../ast/expr.md#binop).                         |
| `infer_unary`           | Dispatches per [`UnOp`](../../ast/expr.md#unop).                           |
| `infer_call`            | Resolves the callee's signature; arity-checks; checks each arg.            |
| `infer_index`           | Validates indexed expression / index type.                                 |
| `infer_member`          | Resolves member via field-table lookup.                                    |
| `infer_cast`            | Delegates to [`check.check_cast`](check.md#check_cast).                    |
| `infer_array_lit`       | Validates each element against the declared element type.                  |
| `infer_struct_lit`      | Validates each field initializer against the named type's field of the same name. |
| `resolve_type_named`    | Walks a dotted [`TypeNamed`](../../ast/type.md#typenamed) reference (including generics). |
| `intern_anonymous_rec_uni` | Synthesises a `(own_module, DECL_NIL, synthetic_name)` nominal entry.   |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.intern`](../../intern.md), [`mach.lang.type`](../../type.md),
[`mach.lang.fe.ast`](../../ast.md),
[`mach.lang.fe.ast.id`](../../ast/id.md),
[`mach.lang.fe.ast.expr`](../../ast/expr.md),
[`mach.lang.fe.ast.decl`](../../ast/decl.md),
[`mach.lang.fe.ast.type`](../../ast/type.md),
[`mach.lang.fe.comptime`](../comptime.md),
[`mach.lang.fe.resolve`](../resolve.md),
[`mach.lang.fe.sema`](../sema.md),
[`mach.lang.fe.sema.check`](check.md),
[`mach.lang.fe.sema.coerce`](coerce.md),
[`mach.lang.fe.sema.generics`](generics.md).
