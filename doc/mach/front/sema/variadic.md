[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **variadic**

# mach.front.sema.variadic

whether a function body reaches a C variadic call, directly or through what it calls,
which decides how a pack spreads into one

Source: [`src/front/sema/variadic.mach`](../../../../src/front/sema/variadic.mach)

Uses types from [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.session.module`](../../session/module.md).

| Functions | |
| :--- | :--- |
| [`decl_body_spreads_pack_to_c_variadic`](#decl_body_spreads_pack_to_c_variadic) | whether the body of function `did` spreads a pack into a call that reaches a C variadic function, directly or through the functions it calls |

---

## Functions

### decl_body_spreads_pack_to_c_variadic

```mach
pub fun decl_body_spreads_pack_to_c_variadic(sc: *mach.front.sema.context.SemaContext, origin: mach.session.module.ModuleId, did: mach.front.ast.id.DeclId) bool
```

whether the body of function `did` spreads a pack into a call that reaches a C
variadic function, directly or through the functions it calls

| Parameter | |
| :--- | :--- |
| `sc` | semantic context the walk reads definitions through |
| `origin` | module that declares the function |
| `did` | the function declaration |
| **returns** | true when such a spread is found, false otherwise or when a part of the walk could not be reached |

