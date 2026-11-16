[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **abitype**

# mach.front.sema.abitype

`#[abi_type]` declarations: C types whose layout the target supplies, such as `va_list`

a bodyless `def` marked `#[abi_type("va_list")]` takes its size and alignment from the selected
target's calling convention. a value of it can only be received as a parameter and passed on, so
no field, payload, result or module binding may hold one, and no parameter may hold one under a
pointer, array or secret.

<sub>source: [`src/front/sema/abitype.mach`](../../../../src/front/sema/abitype.mach) · imported by 6 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `Read` is private · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ReadStatus`](#readstatus) | `def` |  |

| On `ReadStatus` | |
| :--- | :--- |
| [`READ_OK`](#readstatus) | |

| Functions | |
| :--- | :--- |
| [`declares_abi_type`](#declares_abi_type) · [`read`](#read) · [`validate`](#validate) · [`type_of`](#type_of) · [`exclusion_message`](#exclusion_message) · [`type_carries_abi_type`](#type_carries_abi_type) · [`check_abi_type_placement`](#check_abi_type_placement) | |

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
pub fun declares_abi_type(sc: *sema_context.SemaContext, d: *ast_decl.Decl) bool
```

### read

```mach
pub fun read(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator, report: bool) Read
```

### validate

```mach
pub fun validate(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator)
```

### type_of

```mach
pub fun type_of(sc: *sema_context.SemaContext, d: *ast_decl.Decl) type_kind.TypeId
```

### exclusion_message

```mach
pub fun exclusion_message() str
```

### type_carries_abi_type

```mach
pub fun type_carries_abi_type(sc: *sema_context.SemaContext, t: type_kind.TypeId) bool
```

### check_abi_type_placement

```mach
pub fun check_abi_type_placement(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl)
```

