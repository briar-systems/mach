[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **variadic**

# mach.front.sema.variadic

whether a function body reaches a C variadic call, directly or through what it calls,
which decides how a pack spreads into one

Source: [`src/front/sema/variadic.mach`](../../../../src/front/sema/variadic.mach)

Uses types from [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.session.module`](../../session/module.md).

| Functions | |
| :--- | :--- |
| [`decl_body_spreads_pack_to_c_variadic`](#decl_body_spreads_pack_to_c_variadic) | |

---

## Functions

### decl_body_spreads_pack_to_c_variadic

```mach
pub fun decl_body_spreads_pack_to_c_variadic(sc: *mach.front.sema.context.SemaContext, origin: mach.session.module.ModuleId, did: mach.front.ast.id.DeclId) bool
```

