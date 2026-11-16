[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **variadic**

# mach.front.sema.variadic

whether a function body reaches a C variadic call, directly or through what it calls,
which decides how a pack spreads into one

<sub>source: [`src/front/sema/variadic.mach`](../../../../src/front/sema/variadic.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `module` is [`mach.session.module`](../../session/module.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md)</sub>

| Functions | |
| :--- | :--- |
| [`decl_body_spreads_pack_to_c_variadic`](#decl_body_spreads_pack_to_c_variadic) | |

---

## Functions

### decl_body_spreads_pack_to_c_variadic

```mach
pub fun decl_body_spreads_pack_to_c_variadic(sc: *sema_context.SemaContext, origin: module.ModuleId, did: ast_id.DeclId) bool
```

