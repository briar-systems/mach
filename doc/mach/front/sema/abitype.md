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
| [`declares_abi_type`](#declares_abi_type), [`read`](#read), [`validate`](#validate), [`type_of`](#type_of), [`exclusion_message`](#exclusion_message), [`type_carries_abi_type`](#type_carries_abi_type), [`check_abi_type_placement`](#check_abi_type_placement) | |

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

### read

```mach
pub fun read(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator, report: bool) Read
```

### validate

```mach
pub fun validate(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

### type_of

```mach
pub fun type_of(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl) mach.type.kind.TypeId
```

### exclusion_message

```mach
pub fun exclusion_message() str
```

### type_carries_abi_type

```mach
pub fun type_carries_abi_type(sc: *mach.front.sema.context.SemaContext, t: mach.type.kind.TypeId) bool
```

### check_abi_type_placement

```mach
pub fun check_abi_type_placement(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl)
```

