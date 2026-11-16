[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **embed**

# mach.front.sema.embed

the checks on `#[embed]` declarations and the file each one names

an `embed` `val` has no initializer and is typed `[_]u8` or `[N]u8`. sema reads the path the
load recorded for its argument, refuses one that escapes the project root or that this build did
not read, and fixes the array's length to the file's size.

<sub>source: [`src/front/sema/embed.mach`](../../../../src/front/sema/embed.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`resolve_one`](#resolve_one) | |

---

## Functions

### resolve_one

```mach
pub fun resolve_one(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl,
    dec: *ast_decl.Decorator) err[fail.Fail]
```

