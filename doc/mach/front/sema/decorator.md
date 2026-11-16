[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **decorator**

# mach.front.sema.decorator

checking each decorator against its declaration: its places, arguments and rule

<sub>source: [`src/front/sema/decorator.mach`](../../../../src/front/sema/decorator.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md)</sub>

| Functions | |
| :--- | :--- |
| [`infer_decorator_args`](#infer_decorator_args) · [`validate_decorators`](#validate_decorators) | |

---

## Functions

### infer_decorator_args

```mach
pub fun infer_decorator_args(sc: *sema_context.SemaContext, d: *ast_decl.Decl)
```

### validate_decorators

```mach
pub fun validate_decorators(sc: *sema_context.SemaContext, d: *ast_decl.Decl)
```

