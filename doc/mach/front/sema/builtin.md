[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **builtin**

# mach.front.sema.builtin

the built-ins a `#[gpu.builtin]` names: the spir-v built-in each stands for, whether a stage
writes it, and the type the pipeline binds it at, which type checking holds a declaration to
and lowering spells the built-in by

Source: [`src/front/sema/builtin.mach`](../../../../src/front/sema/builtin.mach)

Uses types from [`mach.base.textbuild`](../../base/textbuild.md), [`mach.type`](../../type.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Builtin`](#builtin) | `rec` | one built-in |

| On `Builtin` | |
| :--- | :--- |
| [`of`](#of) | the built-in `name` names |
| [`binds`](#binds) | whether `ty` is the type the pipeline binds `b` at |
| [`required`](#required) | the type the pipeline binds `b` at, as a diagnostic spells it |

| Functions | |
| :--- | :--- |
| [`names_append`](#names_append) | every name `#[gpu.builtin]` takes, quoted, as `"position", ... or "num_subgroups"`; a failed append is held by the builder |

---

## Builtin

```mach
pub rec Builtin {
    name:   str;
    spirv:  str;
    output: bool;
    lanes:  u32;
    float:  bool;
}
```

one built-in

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the name `#[gpu.builtin]` takes |
| `spirv` | `str` | the spir-v built-in it stands for, as masc's grammar spells it |
| `output` | `bool` | whether a stage writes it, where the pipeline supplies every other |
| `lanes` | `u32` | the lanes of the type it is bound at, 1 for a scalar, each lane 32 bits wide |
| `float` | `bool` | whether those lanes are floats; an integer one is either sign, since spir-v's type is sign-less and the vulkan environment asks only for its width |

### of

```mach
pub fun of(name: str) opt[*Builtin]
```

the built-in `name` names

### binds

```mach
pub fun binds(ti: *mach.type.TypeInterner, b: *Builtin, ty: mach.type.kind.TypeId) bool
```

whether `ty` is the type the pipeline binds `b` at

### required

```mach
pub fun required(b: *Builtin) str
```

the type the pipeline binds `b` at, as a diagnostic spells it

## Functions

### names_append

```mach
pub fun names_append(b: *mach.base.textbuild.TextBuilder)
```

every name `#[gpu.builtin]` takes, quoted, as `"position", ... or "num_subgroups"`; a
failed append is held by the builder

