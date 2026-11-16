[mach](../README.md) / [lower](README.md) / **decl**

# mach.lower.decl

the lowering of declarations: functions, instances, class copies and globals

it decides how far each definition is seen and where it is placed: an instance merged by source
identity, or weak where the target's objects merge none, a `#[weak]` default, an `#[export]` in
the export table, a class copy local to the module that asked for it, and a global's section and
`#[at]` offset. a global's initial bytes come from the value its module's interface publishes.

<sub>source: [`src/lower/decl.mach`](../../../src/lower/decl.mach) · imported by 1 module</sub>

<sub>`lower_context` is [`mach.lower.context`](context.md) · `ast_id` is [`mach.front.ast.id`](../front/ast/id.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `ast_decl` is [`mach.front.ast.decl`](../front/ast/decl.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `type_kind` is [`mach.type.kind`](../type/kind.md)</sub>

| Functions | |
| :--- | :--- |
| [`lower_decl`](#lower_decl) | |
| [`lower_instance`](#lower_instance) | an instance bound as `defining` states, or with `copy` the class copy of it taking its pointers in the classes `copy` states, under the name `name` each way |
| [`lower_pack_instance`](#lower_pack_instance) | |
| [`lower_class_copy`](#lower_class_copy) | a copy of the function `did` whose pointer parameters take the classes `sig` states, local to the module whose call asked for it |

---

## Functions

### lower_decl

```mach
pub fun lower_decl(ctx: *lower_context.LowerContext, did: ast_id.DeclId) err[fail.Fail]
```

### lower_instance

```mach
pub fun lower_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId, defining: lower_context.Defining, copy: opt[Type]) err[fail.Fail]
```

an instance bound as `defining` states, or with `copy` the class copy of it taking its
pointers in the classes `copy` states, under the name `name` each way. a generic and a
comptime-parameter instance are lowered alike: the substitution or the bound values are the
scope's

### lower_pack_instance

```mach
pub fun lower_pack_instance(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl,
    name: intern.StrId, types: *type_kind.TypeId, type_len: u32, defining: lower_context.Defining, copy: opt[Type]) err[fail.Fail]
```

### lower_class_copy

```mach
pub fun lower_class_copy(ctx: *lower_context.LowerContext, did: ast_id.DeclId, d: *ast_decl.Decl, name: intern.StrId, sig: Type) err[fail.Fail]
```

a copy of the function `did` whose pointer parameters take the classes `sig` states,
local to the module whose call asked for it

