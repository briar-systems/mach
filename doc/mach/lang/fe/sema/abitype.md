# mach.lang.fe.sema.abitype

## val DIRECTIVE

```mach
pub val DIRECTIVE: str = "abi_type"
```

## def ReadStatus

```mach
pub def ReadStatus: u8
```

## val READ_OK

```mach
pub val READ_OK:      ReadStatus = 0
```

## val READ_REFUSED

```mach
pub val READ_REFUSED: ReadStatus = 1
```

## rec Read

```mach
pub rec Read;
```

## fun decorator_of

```mach
pub fun decorator_of(sc: *sema_context.SemaContext, d: *ast_decl.Decl) *ast_decl.Decorator;
```

## fun declares_abi_type

```mach
pub fun declares_abi_type(sc: *sema_context.SemaContext, d: *ast_decl.Decl) bool;
```

## fun read

```mach
pub fun read(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator, report: bool) Read;
```

## fun validate

```mach
pub fun validate(sc: *sema_context.SemaContext, d: *ast_decl.Decl, dec: *ast_decl.Decorator);
```

## fun type_of

```mach
pub fun type_of(sc: *sema_context.SemaContext, d: *ast_decl.Decl) type.TypeId;
```

## fun exclusion_message

```mach
pub fun exclusion_message() str;
```

