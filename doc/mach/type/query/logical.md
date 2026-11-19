[mach](../../README.md) / [type](../README.md) / [query](README.md) / **logical**

# mach.type.query.logical

the types a logical-addressing target can declare. such a target has no untyped pointer,
so a pointer is a reference to the type it points at and every type is declared in full.
a cycle closes only where a pointer member of a record or union points back at a record
or union on the cycle, which names that record rather than spelling it again, and a
reference into a generic whose argument grows through a pointer names a fresh type at every
step, so it has no declaration

Source: [`src/type/query/logical.mach`](../../../../src/type/query/logical.mach)

Uses types from [`mach.type.kind`](../kind.md), [`mach.type.query`](../query.md).

| Type | | |
| :--- | :--- | :--- |
| [`Refusal`](#refusal) | `def` |  |
| [`Path`](#path) | `rec` | the types a declaration is being spelled through, innermost first |

| On `Refusal` | |
| :--- | :--- |
| [`REFUSAL_RECURSIVE`](#refusal) | a type reaches itself other than through a pointer that names a record on the cycle |
| [`REFUSAL_UNBOUNDED`](#refusal) | a reference into a generic whose argument grows through a pointer |
| [`refusal`](#refusal-1) | why a logical-addressing target cannot declare `tid`, REFUSAL_NONE when it can |

| On `Path` | |
| :--- | :--- |
| [`pointer_names`](#pointer_names) | the record or union a pointer to `ptr`'s base names instead of spelling it, when that base lies on a cycle with the record or union `path` is spelling it inside |

| Functions | |
| :--- | :--- |
| [`on_cycle`](#on_cycle) | whether record or union `tid` reaches itself through its members |

| Constants | |
| :--- | :--- |
| [`RECURSIVE_MSG`](#constants), [`UNBOUNDED_MSG`](#constants) | |

---

## Refusal

```mach
pub def Refusal: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `REFUSAL_RECURSIVE` | `1` | a type reaches itself other than through a pointer that names a record on the cycle |
| `REFUSAL_UNBOUNDED` | `2` | a reference into a generic whose argument grows through a pointer |

### refusal

```mach
pub fun refusal(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) Refusal
```

why a logical-addressing target cannot declare `tid`, REFUSAL_NONE when it can

## Path

```mach
pub rec Path {
    ty:       mach.type.kind.TypeId;
    previous: *Path;
}
```

the types a declaration is being spelled through, innermost first

### pointer_names

```mach
pub fun pointer_names(q: *mach.type.query.Query, path: *Path, ptr: mach.type.kind.TypeId) opt[mach.type.kind.TypeId]
```

the record or union a pointer to `ptr`'s base names instead of spelling it, when that
base lies on a cycle with the record or union `path` is spelling it inside

## Functions

### on_cycle

```mach
pub fun on_cycle(q: *mach.type.query.Query, tid: mach.type.kind.TypeId) bool
```

whether record or union `tid` reaches itself through its members

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `RECURSIVE_MSG` | `str` | `"recursive reference types require pointer capabilities absent from the Logical Shader environment"` |  |
| `UNBOUNDED_MSG` | `str` | `"a reference into a generic whose argument grows through a pointer names a new type at every step, which the Logical Shader environment cannot declare. keep the type out of shader code"` |  |

