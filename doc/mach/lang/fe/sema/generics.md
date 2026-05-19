# mach.lang.fe.sema.generics

Per-use-site monomorphisation. When [`infer`](infer.md) encounters a
generic name reference (e.g. `Map[str, u32]`), it calls
[`instantiate`](#instantiate) here to mint a concrete
[`TYPE_INSTANCE`](../../type.md#typekind) in the shared
[`TypeInterner`](../../type.md#typeinterner) and resolve every
[`TYPE_GENERIC_PARAM`](../../type.md#typekind) inside the generic's
body to the matching concrete argument.

Interning by `(generic_decl, [arg TypeIds])` happens in the universe,
so the same `Map[str, u32]` used in 30 modules resolves to the same
[`TypeId`](../../type.md#typeid). Each module's [sema](../sema.md) runs
in topological dep order, which means the generic's definition has
already been processed by the time its first user instantiates it.

## Functions

### `instantiate`

```mach
pub fun instantiate(
    sc:           *sema.SemaContext,
    generic_decl: id.DeclId,
    generic_origin: sess.ModuleId,
    args:         *type.TypeId,
    arg_count:    u32,
    span:         token.Span,
) Result[type.TypeId, str]
```

Interns a [`TYPE_INSTANCE`](../../type.md#typekind) for `generic_decl`
applied to `args`. Steps:

1. Look up the generic's declaration via
   [`sc.deps`](../sema.md#semadeps) (cross-module) or
   [`sc.rr`](../resolve.md#resolveresult) (local) to find its
   [`generics_len`](../../ast/decl.md#declfun) / [`generics_len`](../../ast/decl.md#declrec).
2. Arity-check `arg_count` against the declared generic count;
   emit `"generic arg count: expected N, got M"` and return
   [`TYPE_ERROR`](../../type.md#typekind) on mismatch.
3. Walk each arg, ensuring it's a valid type (not
   [`TYPE_NIL`](../../type.md#type_nil)); failures absorb into
   `TYPE_ERROR`.
4. Call [`type.intern_instance`](../../type.md#intern_instance) with
   `(generic_decl, generic_origin, args)`. The interner returns the
   same [`TypeId`](../../type.md#typeid) for repeated `(target, args)`
   tuples.

| Param          | Type                                          | Description                                              |
|----------------|-----------------------------------------------|----------------------------------------------------------|
| sc             | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                            |
| generic_decl   | [`id.DeclId`](../../ast/id.md#declid)         | The generic decl being instantiated.                     |
| generic_origin | [`sess.ModuleId`](../../session.md#moduleid)  | The module that owns `generic_decl`.                     |
| args           | [`*type.TypeId`](../../type.md#typeid)        | Pointer to a contiguous array of concrete argument types.|
| arg_count      | `u32`                                         | Number of argument types.                                |
| span           | [`token.Span`](../../token.md#span)           | Span of the instantiation site for diagnostics.          |

Returns the interned [`TYPE_INSTANCE`](../../type.md#typekind)
[`TypeId`](../../type.md#typeid), or an error if arity / arg
validation fails.

### `substitute`

```mach
pub fun substitute(
    sc:        *sema.SemaContext,
    body_type: type.TypeId,
    args:      *type.TypeId,
    arg_count: u32,
) type.TypeId
```

Recursively walks `body_type`, replacing every
[`TYPE_GENERIC_PARAM`](../../type.md#typekind) with index `i` by
`args[i]`. Used by [callers that need to look "through"](#substitution-targets)
the generic body during field-table or signature construction.

Substitution recursion is structural; it intern-rebuilds
[`TYPE_POINTER`](../../type.md#typekind), [`TYPE_ARRAY`](../../type.md#typekind),
[`TYPE_FUN`](../../type.md#typekind), and nested
[`TYPE_INSTANCE`](../../type.md#typekind) types to honour their new
content. Primitive types and unrelated nominal types are returned
unchanged.

| Param     | Type                                          | Description                                              |
|-----------|-----------------------------------------------|----------------------------------------------------------|
| sc        | [`*sema.SemaContext`](../sema.md#semacontext) | Sema context.                                            |
| body_type | [`type.TypeId`](../../type.md#typeid)         | A type expression possibly containing generic parameters.|
| args      | [`*type.TypeId`](../../type.md#typeid)        | Concrete substitution arguments, indexed by generic-param position. |
| arg_count | `u32`                                         | Length of `args`.                                        |

Returns the substituted [`TypeId`](../../type.md#typeid).

## Substitution targets

Generics surface in three concrete places that need substitution:

1. **Field tables for record/union instances.** When sema resolves
   `Map[str, u32].entries`, the field-table lookup walks
   `Map`'s declared fields and substitutes `str` for `K` and `u32`
   for `V` in each field's type.
2. **Function-signature instantiation.** Calling `foo[i32]("hello")`
   on a generic `fun foo[T](x: T) T` produces a signature where every
   `T` in the parameter and return-type slots has been replaced with
   `i32`.
3. **Nested generic args.** `Vec[Pair[i32, str]]` recursively
   instantiates the inner `Pair[i32, str]` before passing it as a
   concrete arg to the outer `Vec[_]`.

## Generic-param resolution during decl walk

When [`infer.infer_decl`](infer.md#infer_decl) types a generic
declaration's body, [`TYPE_GENERIC_PARAM`](../../type.md#typekind)
placeholders are interned at the decl site (one per generic param,
keyed by `(owner_decl, index)`). The body is type-checked against
those placeholders. The result is stored in
[`sc.decl_type`](../sema.md#semacontext) as a normal
[`TYPE_FUN`](../../type.md#typekind) /
[`TYPE_REC`](../../type.md#typekind) /
[`TYPE_UNI`](../../type.md#typekind) whose internal
[`TypeId`](../../type.md#typeid)s reference the generic param TypeIds.

Substitution at instantiation time produces a fresh result that no
longer contains [`TYPE_GENERIC_PARAM`](../../type.md#typekind).

## Recursion and cycles

Generic instantiations are interned, so circular instantiations
(`Vec[Vec[T]]` at depth N for the same T) hit the cache after one
pass. Direct self-recursive instantiation
(`rec Node[T] { next: *Node[T]; }`) works because `*Node[T]` is
recognised as a pointer to a not-yet-fully-resolved nominal — the
recursive use is by pointer, so the inner instance is interned with
just the spine before its fields are walked. The interner returns the
same [`TypeId`](../../type.md#typeid) when the recursion comes back
around.

## Diagnostics

| Message                                     | When emitted                                                  |
|---------------------------------------------|---------------------------------------------------------------|
| `generic arg count: expected N, got M`      | A name reference supplies the wrong number of generic args.   |
| `generic arg N: invalid type`               | One of the args resolved to [`TYPE_NIL`](../../type.md#type_nil) or [`TYPE_ERROR`](../../type.md#typekind). |
| `not a generic: <name>`                     | A name without generics is used with `[...]` generic args.    |
| `cannot instantiate generic body`           | The generic's decl_type was never set (likely an upstream type error). |

## Internal helpers

File-private; listed for reference.

| Function              | Role                                                                       |
|-----------------------|----------------------------------------------------------------------------|
| `find_generic_decl`   | Resolves [`SymbolId`](../resolve.md#symbolid) → `(origin, DeclId)` and looks up the decl in the appropriate AST. |
| `arity_check`         | Validates `arg_count` against the decl's `generics_len`.                   |
| `validate_args`       | Walks `args` checking for `TYPE_NIL` / `TYPE_ERROR`.                       |
| `walk_substitute`     | Recursive worker used by [`substitute`](#substitute).                      |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.intern`](../../intern.md), [`mach.lang.session`](../../session.md),
[`mach.lang.type`](../../type.md), [`mach.lang.fe.ast`](../../ast.md),
[`mach.lang.fe.ast.id`](../../ast/id.md),
[`mach.lang.fe.ast.decl`](../../ast/decl.md),
[`mach.lang.fe.token`](../../token.md),
[`mach.lang.fe.resolve`](../resolve.md),
[`mach.lang.fe.sema`](../sema.md).
