[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **noreturn**

# mach.front.sema.noreturn

the `#[noreturn]` rule: a function so marked states no result, holds no `ret`, and its body ends where control never
comes back, so falling off its end is unreachable

a statement ends when control never passes it: a call to a `noreturn` function, an `asm` block marked `noreturn`, a
`$error` directive, a bare `for` no `brk` of its own leaves, a block holding a statement that ends, an `if` chain
with a bare `or` each arm of which ends, and a `$if` each branch of which ends where one branch is bare

Source: [`src/front/sema/noreturn.mach`](../../../../src/front/sema/noreturn.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`validate`](#validate) | hold the function `d` the decorator `dec` marks `noreturn` to the rule |

---

## Functions

### validate

```mach
pub fun validate(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

hold the function `d` the decorator `dec` marks `noreturn` to the rule

