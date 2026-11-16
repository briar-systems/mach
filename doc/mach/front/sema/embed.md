[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **embed**

# mach.front.sema.embed

the checks on `#[embed]` declarations and the file each one names

an `embed` `val` has no initializer and is typed `[_]u8` or `[N]u8`. sema reads the path the
load recorded for its argument, refuses one that escapes the project root or that this build did
not read, and fixes the array's length to the file's size.

Source: [`src/front/sema/embed.mach`](../../../../src/front/sema/embed.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md).

| Functions | |
| :--- | :--- |
| [`resolve_one`](#resolve_one) | |

---

## Functions

### resolve_one

```mach
pub fun resolve_one(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl,
    dec: *mach.front.ast.decl.Decorator) err[mach.base.fail.Fail]
```

