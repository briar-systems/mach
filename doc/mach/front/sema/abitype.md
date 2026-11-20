[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **abitype**

# mach.front.sema.abitype

`#[abi_type]` declarations: C types whose layout the target supplies, such as `va_list`

a bodyless `def` marked `#[abi_type("va_list")]` takes its size and alignment from the selected
target's calling convention. a value of it can only be received as a parameter and passed on, so
no field, payload, result or module binding may hold one, and no parameter may hold one under a
pointer, array or secret.

Source: [`src/front/sema/abitype.mach`](../../../../src/front/sema/abitype.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`ReadStatus`](#readstatus) | `def` |  |

| On `ReadStatus` | |
| :--- | :--- |
| [`READ_OK`](#readstatus) | |

| Functions | |
| :--- | :--- |
| [`declares_abi_type`](#declares_abi_type) | whether `d` is a bodyless `def` marked `#[abi_type]` |
| [`read`](#read) | read the layout an `#[abi_type]` decorator on `d` names from the selected target |
| [`validate`](#validate) | report everything wrong with an `#[abi_type]` decorator on `d` |
| [`type_of`](#type_of) | the interned ABI type an `#[abi_type]` declaration stands for |
| [`exclusion_message`](#exclusion_message) | the message every misplaced ABI type value is reported with |
| [`type_carries_abi_type`](#type_carries_abi_type) | whether `t` is an ABI type, or one under any chain of secret, pointer and array |
| [`check_abi_type_placement`](#check_abi_type_placement) | report every place in declaration `d` that holds an ABI type where it cannot: a tag payload, a record or union field, a function result, a module binding, or a parameter that holds one under a wrapper, since only a parameter of the ABI type itself is allowed |

---

## ReadStatus

```mach
pub def ReadStatus: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `READ_OK` | `0` |  |

## Functions

### declares_abi_type

```mach
pub fun declares_abi_type(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl) bool
```

whether `d` is a bodyless `def` marked `#[abi_type]`

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the declaration asked about |
| **returns** | true for a bodyless `def` that carries the decorator |

### read

```mach
pub fun read(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator, report: bool) Read
```

read the layout an `#[abi_type]` decorator on `d` names from the selected target

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the declaration the decorator is on |
| `dec` | the `abi_type` decorator |
| `report` | whether a misplaced or malformed decorator, or an unavailable layout, is reported |
| **returns** | the read, with status READ_REFUSED and zero layout when anything is wrong |

### validate

```mach
pub fun validate(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

report everything wrong with an `#[abi_type]` decorator on `d`

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the declaration the decorator is on |
| `dec` | the `abi_type` decorator |

### type_of

```mach
pub fun type_of(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl) mach.type.kind.TypeId
```

the interned ABI type an `#[abi_type]` declaration stands for

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `d` | the declaration |
| **returns** | the type, or TYPE_ERROR when `d` has no such decorator or its layout is refused, which reports nothing here, or interning it fails, which fails the phase as internal |

### exclusion_message

```mach
pub fun exclusion_message() str
```

the message every misplaced ABI type value is reported with

### type_carries_abi_type

```mach
pub fun type_carries_abi_type(sc: *mach.front.sema.context.SemaContext, t: mach.type.kind.TypeId) bool
```

whether `t` is an ABI type, or one under any chain of secret, pointer and array

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `t` | the type asked about |
| **returns** | true when an ABI type is reached through those wrappers |

### check_abi_type_placement

```mach
pub fun check_abi_type_placement(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl)
```

report every place in declaration `d` that holds an ABI type where it cannot: a tag payload,
a record or union field, a function result, a module binding, or a parameter that holds one
under a wrapper, since only a parameter of the ABI type itself is allowed

| Parameter | |
| :--- | :--- |
| `sc` | the walk |
| `did` | the id of `d`, which keys the type of a module binding |
| `d` | the declaration checked |

