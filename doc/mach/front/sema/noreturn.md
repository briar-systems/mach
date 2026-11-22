[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **noreturn**

# mach.front.sema.noreturn

the `#[noreturn]` rule: a function so marked states no result and holds no `ret`. that its body never reaches its
end is held where the walk types the body, by the rule a function with a result type is held to

Source: [`src/front/sema/noreturn.mach`](../../../../src/front/sema/noreturn.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`validate`](#validate) | hold the function `d` the decorator `dec` marks `noreturn` to the rule |
| [`call_ends`](#call_ends) | whether the expression `eid` is a call of a function declared `noreturn` |

---

## Functions

### validate

```mach
pub fun validate(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

hold the function `d` the decorator `dec` marks `noreturn` to the rule

### call_ends

```mach
pub fun call_ends(sc: *mach.front.sema.context.SemaContext, eid: mach.front.ast.id.ExprId) bool
```

whether the expression `eid` is a call of a function declared `noreturn`

