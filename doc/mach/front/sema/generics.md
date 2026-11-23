[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **generics**

# mach.front.sema.generics

the instantiation of a generic the sema context names, and the checks each instance owes
its type annotations

Source: [`src/front/sema/generics.mach`](../../../../src/front/sema/generics.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Functions | |
| :--- | :--- |
| [`instantiate`](#instantiate) | the instance of the generic record, union or tag `sym` names at the type arguments `args` |
| [`check_arity`](#check_arity) | whether `sym` is a function or a type that declares exactly `arg_count` type parameters, reporting the mismatch when it does not |
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

the instance of the generic record, union or tag `sym` names at the type arguments `args`.
a declaration that is not a generic type, a wrong argument count or an argument that is not
a type is reported and gives the error type

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `sym` | the symbol the generic is named by |
| `args` | the type arguments, `arg_count` of them |
| `arg_count` | how many type arguments the use states |
| `span` | where the use stands, for diagnostics |
| **returns** | the instance type, the error type after a reported refusal, or the internal failure of interning its nominal or instance |

### check_arity

```mach
pub fun check_arity(
    sc: *mach.front.sema.context.SemaContext,
    sym: *mach.front.resolve.Symbol,
    arg_count: u32,
    span: mach.base.source.Span) bool
```

whether `sym` is a function or a type that declares exactly `arg_count` type parameters,
reporting the mismatch when it does not

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `sym` | the symbol the use names |
| `arg_count` | how many type arguments the use states |
| `span` | where the use stands, for diagnostics |
| **returns** | true when the count matches |

### check_annotation

```mach
pub fun check_annotation(sc: *mach.front.sema.context.SemaContext, ast_tid: mach.front.ast.id.TypeId)
```

the checks an instance owes each type annotation it resolves, which no check of the
generic declaration can answer before its parameters are known

