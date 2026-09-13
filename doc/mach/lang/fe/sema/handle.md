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
pub fun decorator_of(sc: *context.SemaContext, d: *decl.Decl) *decl.Decorator;
```

## fun declares_handle

```mach
pub fun declares_handle(sc: *context.SemaContext, d: *decl.Decl) bool;
```

## fun arg_is_type

```mach
pub fun arg_is_type(sc: *context.SemaContext, dec: *decl.Decorator, index: u32) bool;
```

## fun read

```mach
pub fun read(sc: *context.SemaContext, d: *decl.Decl, dec: *decl.Decorator, report: bool, depth: u32) Read;
```

## fun release

```mach
pub fun release(sc: *context.SemaContext, r: Read);
```

## fun validate

```mach
pub fun validate(sc: *context.SemaContext, d: *decl.Decl, dec: *decl.Decorator);
```

## fun type_of

```mach
pub fun type_of(sc: *context.SemaContext, did: id.DeclId, d: *decl.Decl) type.TypeId;
```

