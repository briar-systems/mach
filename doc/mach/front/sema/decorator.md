[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **decorator**

# mach.front.sema.decorator

checking each decorator against its declaration: its places, arguments and rule

Source: [`src/front/sema/decorator.mach`](../../../../src/front/sema/decorator.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`infer_decorator_args`](#infer_decorator_args), [`validate_decorators`](#validate_decorators) | |

---

## Functions

### infer_decorator_args

```mach
pub fun infer_decorator_args(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

### validate_decorators

```mach
pub fun validate_decorators(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

