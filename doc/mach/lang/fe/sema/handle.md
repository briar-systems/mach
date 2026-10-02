# mach.lang.fe.sema.handle

## val DIRECTIVE

```mach
pub val DIRECTIVE:  str = "handle"
```

## def ReadStatus

```mach
pub def ReadStatus: u8
```

## val READ_OK

```mach
pub val READ_OK:      ReadStatus = 0
```

## val READ_INERT

```mach
pub val READ_INERT:   ReadStatus = 1
```

## val READ_REFUSED

```mach
pub val READ_REFUSED: ReadStatus = 2
```

## rec Read

```mach
pub rec Read;
```

## fun decorator_of

```mach
pub fun decorator_of(sc: *sema_context.SemaContext, d: *ast_decl.Decl) *ast_decl.Decorator;
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

