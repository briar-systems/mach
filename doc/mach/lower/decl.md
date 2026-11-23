[mach](../README.md) / [lower](README.md) / **decl**

# mach.lower.decl

the lowering of declarations: functions, instances, class copies and globals

it decides how far each definition is seen and where it is placed: an instance merged by source
identity, or weak where the target's objects merge none, a `#[weak]` default, an `#[export]` in
the export table, a class copy local to the module that asked for it, and a global's section and
`#[at]` offset. a global's initial bytes come from the value its module's interface publishes.

Source: [`src/lower/decl.mach`](../../../src/lower/decl.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.front.ast.decl`](../front/ast/decl.md), [`mach.front.ast.id`](../front/ast/id.md), [`mach.lower.context`](context.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`lower_decl`](#lower_decl) | lower the module-level declaration `did` into the module: a function, a test in the test object, a global, a handle def or the active branch of a `$if` |
| [`lower_instance`](#lower_instance) | an instance bound as `defining` states, or with `copy` the class copy of it taking its pointers in the classes `copy` states, under the name `name` each way |
| [`lower_pack_instance`](#lower_pack_instance) | a pack instance of the function `did`, its pack expanded to `types`, bound as `defining` states, or with `copy` the class copy of it, under the name `name` |
| [`lower_class_copy`](#lower_class_copy) | a copy of the function `did` whose pointer parameters take the classes `sig` states, local to the module whose call asked for it |

---

## Functions

### lower_decl

```mach
pub fun lower_decl(ctx: *mach.lower.context.LowerContext, did: mach.front.ast.id.DeclId) err[mach.base.fail.Fail]
```

lower the module-level declaration `did` into the module: a function, a test in the test
object, a global, a handle def or the active branch of a `$if`. a generic, comptime or pack
function and a `#[testing]` declaration in the normal object lower to nothing here

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `did` | the declaration |
| **returns** | nothing, or the refusal or failure met lowering it |

### lower_instance

```mach
pub fun lower_instance(ctx: *mach.lower.context.LowerContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl, name: mach.base.intern.StrId, defining: mach.lower.context.Defining, copy: opt[Type]) err[mach.base.fail.Fail]
```

an instance bound as `defining` states, or with `copy` the class copy of it taking its
pointers in the classes `copy` states, under the name `name` each way. a generic and a
comptime-parameter instance are lowered alike: the substitution or the bound values are the
scope's

### lower_pack_instance

```mach
pub fun lower_pack_instance(ctx: *mach.lower.context.LowerContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl,
    name: mach.base.intern.StrId, types: *mach.type.kind.TypeId, type_len: u32, defining: mach.lower.context.Defining, copy: opt[Type]) err[mach.base.fail.Fail]
```

a pack instance of the function `did`, its pack expanded to `types`, bound as `defining`
states, or with `copy` the class copy of it, under the name `name`

| Parameter | |
| :--- | :--- |
| `ctx` | the lowering context |
| `did` | the declaration |
| `d` | its node |
| `name` | the instance's linkage name |
| `types` | the source type of each element of the pack |
| `type_len` | how many there are |
| `defining` | how its definition is bound |
| `copy` | the copy's signature, none for the instance itself |
| **returns** | nothing, or the failure met lowering it |

### lower_class_copy

```mach
pub fun lower_class_copy(ctx: *mach.lower.context.LowerContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl, name: mach.base.intern.StrId, sig: Type) err[mach.base.fail.Fail]
```

a copy of the function `did` whose pointer parameters take the classes `sig` states,
local to the module whose call asked for it

