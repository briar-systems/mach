[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **generics**

# mach.front.sema.generics

the instantiation of a generic the sema context names, and the checks each instance owes
its type annotations

<sub>source: [`src/front/sema/generics.mach`](../../../../src/front/sema/generics.mach) · imported by 3 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md)</sub>

| Functions | |
| :--- | :--- |
| [`instantiate`](#instantiate) · [`check_arity`](#check_arity) | |
| [`check_annotation`](#check_annotation) | the checks an instance owes each type annotation it resolves, which no check of the generic declaration can answer before its parameters are known |

---

## Functions

### instantiate

```mach
pub fun instantiate(
    sc: *sema_context.SemaContext,
    sym: *resolve.Symbol,
    args: *type_kind.TypeId,
    arg_count: u32,
    span: base_source.Span) res[type_kind.TypeId, fail.Fail]
```

### check_arity

```mach
pub fun check_arity(
    sc: *sema_context.SemaContext,
    sym: *resolve.Symbol,
    arg_count: u32,
    span: base_source.Span) bool
```

### check_annotation

```mach
pub fun check_annotation(sc: *sema_context.SemaContext, ast_tid: ast_id.TypeId)
```

the checks an instance owes each type annotation it resolves, which no check of the
generic declaration can answer before its parameters are known

