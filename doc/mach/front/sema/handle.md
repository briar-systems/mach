[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **handle**

# mach.front.sema.handle

`#[handle]` declarations, the types a target mints, and where a handle may be held

a bodyless `def` marked `#[handle(target, constructor, operands...)]` is typed from its
constructor and operands when the build selects its target, and is inert otherwise. a handle's
representation is the target's, not the program's, so it may not be a field or payload, sit in
an array or behind a pointer to a pointer, and a module binding of one must carry its
descriptor decorator.

<sub>source: [`src/front/sema/handle.mach`](../../../../src/front/sema/handle.mach) · imported by 6 modules</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md) · `Read` is private · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `type_scan` is [`mach.type.scan`](../../type/scan.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ReadStatus`](#readstatus) | `def` |  |

| On `ReadStatus` | |
| :--- | :--- |
| [`READ_OK`](#readstatus) | |

| Functions | |
| :--- | :--- |
| [`arg_is_type`](#arg_is_type) · [`read`](#read) · [`read_dnit`](#read_dnit) · [`validate`](#validate) · [`type_of`](#type_of) | |
| [`check_annotation_handles_all`](#check_annotation_handles_all) | every annotation's check shares one walk, so a type that several annotations spell, as a chain of defs each over the one before does, is walked once rather than once per annotation |
| [`scan_begin`](#scan_begin) · [`type_carries_handle`](#type_carries_handle) | |
| [`annotation_refuses_indirection`](#annotation_refuses_indirection) | a binding whose written annotation holds a chain of pointers over a handle has drawn that annotation's refusal, and a binding whose type is inferred has no annotation to draw it |
| [`check_handle_fields_all`](#check_handle_fields_all) | |
| [`check_handle_signature`](#check_handle_signature) | a function's parameters and result are values the program passes, and a handle reaches one by value or behind a pointer to its binding, never inside an array |
| [`check_handle_decl`](#check_handle_decl) | |
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
pub fun arg_is_type(sc: *sema_context.SemaContext, dec: *ast_decl.Decorator, index: u32) bool
```

### read

```mach
pub fun read(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator, report: bool, depth: u32) Read
```

### read_dnit

```mach
pub fun read_dnit(sc: *sema_context.SemaContext, r: Read)
```

### validate

```mach
pub fun validate(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator)
```

### type_of

```mach
pub fun type_of(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl) type_kind.TypeId
```

### check_annotation_handles_all

```mach
pub fun check_annotation_handles_all(sc: *sema_context.SemaContext)
```

every annotation's check shares one walk, so a type that several annotations spell, as a
chain of defs each over the one before does, is walked once rather than once per annotation

### scan_begin

```mach
pub fun scan_begin(sc: *sema_context.SemaContext, scan: *type_scan.Scan) bool
```

### type_carries_handle

```mach
pub fun type_carries_handle(sc: *sema_context.SemaContext, t: type_kind.TypeId) bool
```

### annotation_refuses_indirection

```mach
pub fun annotation_refuses_indirection(sc: *sema_context.SemaContext, ann: ast_id.TypeId, t: type_kind.TypeId) bool
```

a binding whose written annotation holds a chain of pointers over a handle has drawn that
annotation's refusal, and a binding whose type is inferred has no annotation to draw it

### check_handle_fields_all

```mach
pub fun check_handle_fields_all(sc: *sema_context.SemaContext)
```

### check_handle_signature

```mach
pub fun check_handle_signature(sc: *sema_context.SemaContext, d: *ast_decl.Decl)
```

a function's parameters and result are values the program passes, and a handle reaches
one by value or behind a pointer to its binding, never inside an array

### check_handle_decl

```mach
pub fun check_handle_decl(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl)
```

### check_annotation_handles

```mach
pub fun check_annotation_handles(sc: *sema_context.SemaContext, walked: *type_scan.Scan, ast_tid: ast_id.TypeId)
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

