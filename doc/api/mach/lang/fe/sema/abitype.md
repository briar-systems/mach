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
pub fun decorator_of(sc: *context.SemaContext, d: *decl.Decl) *decl.Decorator;
```

## fun declares_abi_type

```mach
pub fun declares_abi_type(sc: *context.SemaContext, d: *decl.Decl) bool;
```

## fun read

```mach
pub fun read(sc: *context.SemaContext, d: *decl.Decl, dec: *decl.Decorator, report: bool) Read;
```

## fun validate

```mach
pub fun validate(sc: *context.SemaContext, d: *decl.Decl, dec: *decl.Decorator);
```

## fun type_of

```mach
pub fun type_of(sc: *context.SemaContext, d: *decl.Decl) type.TypeId;
```

## fun exclusion_message

```mach
pub fun exclusion_message() str;
```

