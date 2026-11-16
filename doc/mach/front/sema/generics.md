[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **generics**

# mach.front.sema.generics

the instantiation of a generic the sema context names, and the checks each instance owes
its type annotations

Source: [`src/front/sema/generics.mach`](../../../../src/front/sema/generics.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`instantiate`](#instantiate), [`check_arity`](#check_arity) | |
| [`check_annotation`](#check_annotation) | the checks an instance owes each type annotation it resolves, which no check of the generic declaration can answer before its parameters are known |

---

## Functions

### instantiate

```mach
pub fun instantiate(
    sc: *mach.front.sema.context.SemaContext,
    sym: *mach.front.resolve.Symbol,
    args: *mach.type.kind.TypeId,
    arg_count: u32,
    span: mach.base.source.Span) res[mach.type.kind.TypeId, mach.base.fail.Fail]
```

### check_arity

```mach
pub fun check_arity(
    sc: *mach.front.sema.context.SemaContext,
    sym: *mach.front.resolve.Symbol,
    arg_count: u32,
    span: mach.base.source.Span) bool
```

### check_annotation

```mach
pub fun check_annotation(sc: *mach.front.sema.context.SemaContext, ast_tid: mach.front.ast.id.TypeId)
```

the checks an instance owes each type annotation it resolves, which no check of the
generic declaration can answer before its parameters are known

