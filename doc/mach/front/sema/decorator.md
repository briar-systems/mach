[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **decorator**

# mach.front.sema.decorator

checking each decorator against its declaration: its places, arguments and rule

Source: [`src/front/sema/decorator.mach`](../../../../src/front/sema/decorator.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`infer_decorator_args`](#infer_decorator_args) | types every expression argument of `d`'s decorators, leaving out the arguments a `handle` decorator takes as types and every argument of an unknown decorator |
| [`validate_decorators`](#validate_decorators) | checks each decorator of `d` against its registry row and then against the rule its id adds, with the decorators of a tag's cases, a `def` that states no type, and the symbols `d` defines |

---

## Functions

### infer_decorator_args

```mach
pub fun infer_decorator_args(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

types every expression argument of `d`'s decorators, leaving out the arguments a
`handle` decorator takes as types and every argument of an unknown decorator

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `d` | the declaration whose decorators are typed |

### validate_decorators

```mach
pub fun validate_decorators(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

checks each decorator of `d` against its registry row and then against the rule its
id adds, with the decorators of a tag's cases, a `def` that states no type, and the
symbols `d` defines. problems are reported to `sc`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `d` | the declaration whose decorators are checked |

