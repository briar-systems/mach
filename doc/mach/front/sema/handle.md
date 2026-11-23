[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **handle**

# mach.front.sema.handle

`#[handle]` declarations, the types a target mints, and where a handle may be held

a bodyless `def` marked `#[handle(target, constructor, operands...)]` is typed from its
constructor and operands when the build selects its target, and is inert otherwise. a handle's
representation is the target's, not the program's, so it may not be a field or payload, sit in
an array or behind a pointer to a pointer, and a module binding of one must carry its
descriptor decorator. only a handle binding takes `gpu.attachment`, which the target
holds to the handles it declares subpass inputs.

Source: [`src/front/sema/handle.mach`](../../../../src/front/sema/handle.mach)

Uses types from [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.sema.context`](context.md), [`mach.type.kind`](../../type/kind.md), [`mach.type.scan`](../../type/scan.md).

| Type | | |
| :--- | :--- | :--- |
| [`ReadStatus`](#readstatus) | `def` |  |

| On `ReadStatus` | |
| :--- | :--- |
| [`READ_OK`](#readstatus) | |

| Functions | |
| :--- | :--- |
| [`arg_is_type`](#arg_is_type) | whether a handle operand names a type, which is read as one and never as a value |
| [`read`](#read) | reads the `handle` decorator `dec` on `d`: the kind its target mints and its operands, each a type or a constant word |
| [`read_dnit`](#read_dnit) | releases the operands of a read |
| [`validate`](#validate) | reports every problem `read` finds in the `handle` decorator `dec` on `d` |
| [`type_of`](#type_of) | the handle type the declaration `d` mints, inert when its target is not the one selected |
| [`check_annotation_handles_all`](#check_annotation_handles_all) | every annotation's check shares one walk, so a type that several annotations spell, as a chain of defs each over the one before does, is walked once rather than once per annotation |
| [`scan_begin`](#scan_begin) | begins a scan over the session's type table, reporting a failure as internal |
| [`type_carries_handle`](#type_carries_handle) | whether `t` holds a handle, directly or under a pointer, array or secret |
| [`annotation_refuses_indirection`](#annotation_refuses_indirection) | a binding whose written annotation holds a chain of pointers over a handle has drawn that annotation's refusal, and a binding whose type is inferred has no annotation to draw it |
| [`check_handle_fields_all`](#check_handle_fields_all) | reports each field of the module's records and unions, and each payload of its tags, that holds a handle |
| [`check_handle_signature`](#check_handle_signature) | a function's parameters and result are values the program passes, and a handle reaches one by value or behind a pointer to its binding, never inside an array |
| [`check_handle_decl`](#check_handle_decl) | holds a module binding to the rules of handle storage: a handle stands only as itself, is bound with `gpu.sampler` or `gpu.storage`, and `gpu.sampler` and `gpu.attachment` bind nothing else |
| [`check_annotation_handles`](#check_annotation_handles) | a generic record, union or tag instantiated so that a field or payload holds a handle, directly or through a pointer, array or secret: the instance is the declaration its arguments spell, and the same declaration written out is refused for that field |

| Constants | |
| :--- | :--- |
| [`MAX_COMPOSE_DEPTH`](#constants) | |

---

## ReadStatus

```mach
pub def ReadStatus: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `READ_OK` | `0` |  |

## Functions

### arg_is_type

```mach
pub fun arg_is_type(sc: *mach.front.sema.context.SemaContext, dec: *mach.front.ast.decl.Decorator, index: u32) bool
```

whether a handle operand names a type, which is read as one and never as a value

### read

```mach
pub fun read(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator, report: bool, depth: u32) Read
```

reads the `handle` decorator `dec` on `d`: the kind its target mints and its operands, each a
type or a constant word. one naming a target the build does not select is inert

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `d` | the declaration carrying the decorator |
| `dec` | the `handle` decorator |
| `report` | whether a refusal is reported, or only returned |
| `depth` | how many handles deep the read is, bounded by MAX_COMPOSE_DEPTH |
| **returns** | the read, refused after a misplaced or malformed decorator or an internal failure, whose operands the caller releases with `read_dnit` |

### read_dnit

```mach
pub fun read_dnit(sc: *mach.front.sema.context.SemaContext, r: Read)
```

releases the operands of a read

| Parameter | |
| :--- | :--- |
| `sc` | the sema context whose allocator holds them |
| `r` | the read to release |

### validate

```mach
pub fun validate(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl, dec: *mach.front.ast.decl.Decorator)
```

reports every problem `read` finds in the `handle` decorator `dec` on `d`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `d` | the declaration carrying the decorator |
| `dec` | the `handle` decorator |

### type_of

```mach
pub fun type_of(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl) mach.type.kind.TypeId
```

the handle type the declaration `d` mints, inert when its target is not the one selected

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `did` | the declaration's id |
| `d` | the bodyless `def` carrying the decorator |
| **returns** | the handle type, or the error type when `d` has no readable `handle` decorator or interning the type failed |

### check_annotation_handles_all

```mach
pub fun check_annotation_handles_all(sc: *mach.front.sema.context.SemaContext)
```

every annotation's check shares one walk, so a type that several annotations spell, as a
chain of defs each over the one before does, is walked once rather than once per annotation

### scan_begin

```mach
pub fun scan_begin(sc: *mach.front.sema.context.SemaContext, scan: *mach.type.scan.Scan) bool
```

begins a scan over the session's type table, reporting a failure as internal

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `scan` | the scan to begin, which the caller ends |
| **returns** | true when the scan began |

### type_carries_handle

```mach
pub fun type_carries_handle(sc: *mach.front.sema.context.SemaContext, t: mach.type.kind.TypeId) bool
```

whether `t` holds a handle, directly or under a pointer, array or secret

| Parameter | |
| :--- | :--- |
| `sc` | the sema context whose type table is read |
| `t` | the type asked about |
| **returns** | true when it carries a handle |

### annotation_refuses_indirection

```mach
pub fun annotation_refuses_indirection(sc: *mach.front.sema.context.SemaContext, ann: mach.front.ast.id.TypeId, t: mach.type.kind.TypeId) bool
```

a binding whose written annotation holds a chain of pointers over a handle has drawn that
annotation's refusal, and a binding whose type is inferred has no annotation to draw it

### check_handle_fields_all

```mach
pub fun check_handle_fields_all(sc: *mach.front.sema.context.SemaContext)
```

reports each field of the module's records and unions, and each payload of its tags, that
holds a handle

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |

### check_handle_signature

```mach
pub fun check_handle_signature(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

a function's parameters and result are values the program passes, and a handle reaches
one by value or behind a pointer to its binding, never inside an array

### check_handle_decl

```mach
pub fun check_handle_decl(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl)
```

holds a module binding to the rules of handle storage: a handle stands only as itself, is
bound with `gpu.sampler` or `gpu.storage`, and `gpu.sampler` and `gpu.attachment` bind
nothing else. a uniform, storage or push block of an ordinary type is an unpacked record

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `did` | the declaration's id |
| `d` | the declaration checked, which is passed over unless it is a module binding |

### check_annotation_handles

```mach
pub fun check_annotation_handles(sc: *mach.front.sema.context.SemaContext, walked: *mach.type.scan.Scan, ast_tid: mach.front.ast.id.TypeId)
```

a generic record, union or tag instantiated so that a field or payload holds a handle,
directly or through a pointer, array or secret: the instance is the declaration its
arguments spell, and the same declaration written out is refused for that field. a
pointer to a pointer to a handle is refused at the annotation that spells or forms it. the
check of one annotation, whose walk skips the types `walked` holds and adds those it walks

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MAX_COMPOSE_DEPTH` | `u32` | `8` |  |

