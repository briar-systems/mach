# mach.lang.fe.sema.handle

## def ReadStatus

```mach
pub def ReadStatus: u8
```

## val READ_OK

```mach
pub val READ_OK:      ReadStatus = 0
```

## fun arg_is_type

```mach
pub fun arg_is_type(sc: *sema_context.SemaContext, dec: *ast_decl.Decorator, index: u32) bool;
```

## val MAX_COMPOSE_DEPTH

```mach
pub val MAX_COMPOSE_DEPTH: u32 = 8
```

## fun read

```mach
pub fun read(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator, report: bool, depth: u32) Read;
```

## fun release

```mach
pub fun release(sc: *sema_context.SemaContext, r: Read);
```

## fun validate

```mach
pub fun validate(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator);
```

## fun type_of

```mach
pub fun type_of(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl) type.TypeId;
```

## fun check_annotation_handles_all

```mach
pub fun check_annotation_handles_all(sc: *sema_context.SemaContext);
```

## fun type_carries_handle

```mach
pub fun type_carries_handle(sc: *sema_context.SemaContext, t: type.TypeId) bool;
```

## fun annotation_refuses_indirection

```mach
pub fun annotation_refuses_indirection(sc: *sema_context.SemaContext, ann: ast_id.TypeId, t: type.TypeId) bool;
```

a binding whose written annotation holds a chain of pointers over a handle has drawn that
annotation's refusal, and a binding whose type is inferred has no annotation to draw it

## fun check_handle_fields_all

```mach
pub fun check_handle_fields_all(sc: *sema_context.SemaContext);
```

## fun check_handle_signature

```mach
pub fun check_handle_signature(sc: *sema_context.SemaContext, d: *ast_decl.Decl);
```

a function's parameters and result are values the program passes, and a handle reaches
one by value or behind a pointer to its binding, never inside an array

## fun check_handle_decl

```mach
pub fun check_handle_decl(sc: *sema_context.SemaContext, did: ast_id.DeclId, d: *ast_decl.Decl);
```

## fun check_annotation_handles

```mach
pub fun check_annotation_handles(sc: *sema_context.SemaContext, ast_tid: ast_id.TypeId);
```

a generic record, union or tag instantiated so that a field or payload holds a handle,
directly or through a pointer, array or secret: the instance is the declaration its
arguments spell, and the same declaration written out is refused for that field. a
pointer to a pointer to a handle is refused at the annotation that spells or forms it

