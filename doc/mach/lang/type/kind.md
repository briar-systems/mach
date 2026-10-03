# mach.lang.type.kind

## rec TypeId

```mach
pub rec TypeId;
```

the identity of one type in the type table

## val TYPE_NIL

```mach
pub val TYPE_NIL:   TypeId = TypeId;
```

## val TYPE_ERROR

```mach
pub val TYPE_ERROR: TypeId = TypeId;
```

## fun id

```mach
pub fun id(index: u32) TypeId;
```

## fun index

```mach
pub fun index(t: TypeId) u32;
```

## fun same

```mach
pub fun same(left: TypeId, right: TypeId) bool;
```

## fun is_nil

```mach
pub fun is_nil(t: TypeId) bool;
```

## fun is_error

```mach
pub fun is_error(t: TypeId) bool;
```

## val TABLE_INDEX_TEXT

```mach
pub val TABLE_INDEX_TEXT: str = "type table index out of bounds"
```

an index the type table issued no longer names a slot: a defect, never an input fault

## def TypeKind

```mach
pub def TypeKind: u8
```

## val TYPE_U8

```mach
pub val TYPE_U8:  TypeKind = 0
```

## val TYPE_U16

```mach
pub val TYPE_U16: TypeKind = 1
```

## val TYPE_U32

```mach
pub val TYPE_U32: TypeKind = 2
```

## val TYPE_U64

```mach
pub val TYPE_U64: TypeKind = 3
```

## val TYPE_I8

```mach
pub val TYPE_I8:  TypeKind = 4
```

## val TYPE_I16

```mach
pub val TYPE_I16: TypeKind = 5
```

## val TYPE_I32

```mach
pub val TYPE_I32: TypeKind = 6
```

## val TYPE_I64

```mach
pub val TYPE_I64: TypeKind = 7
```

## val TYPE_F32

```mach
pub val TYPE_F32: TypeKind = 8
```

## val TYPE_F64

```mach
pub val TYPE_F64: TypeKind = 9
```

## val TYPE_PTR

```mach
pub val TYPE_PTR: TypeKind = 10
```

## val TYPE_POINTER

```mach
pub val TYPE_POINTER:       TypeKind = 11
```

## val TYPE_ARRAY

```mach
pub val TYPE_ARRAY:         TypeKind = 12
```

## val TYPE_FUN

```mach
pub val TYPE_FUN:           TypeKind = 13
```

## val TYPE_REC

```mach
pub val TYPE_REC:           TypeKind = 14
```

## val TYPE_UNI

```mach
pub val TYPE_UNI:           TypeKind = 15
```

## val TYPE_GENERIC_PARAM

```mach
pub val TYPE_GENERIC_PARAM: TypeKind = 16
```

## val TYPE_INSTANCE

```mach
pub val TYPE_INSTANCE:      TypeKind = 17
```

## val TYPE_PACK

```mach
pub val TYPE_PACK: TypeKind = 18
```

## val TYPE_SECRET

```mach
pub val TYPE_SECRET: TypeKind = 19
```

## val TYPE_VECTOR

```mach
pub val TYPE_VECTOR: TypeKind = 20
```

## val TYPE_HANDLE

```mach
pub val TYPE_HANDLE: TypeKind = 21
```

## val TYPE_ABI

```mach
pub val TYPE_ABI: TypeKind = 22
```

## val TYPE_TAG

```mach
pub val TYPE_TAG: TypeKind = 23
```

## val TYPE_CASE_SELECTOR

```mach
pub val TYPE_CASE_SELECTOR: TypeKind = 24
```

## val TYPE_U128

```mach
pub val TYPE_U128: TypeKind = 25
```

the 128-bit integers are appended kinds: the catalog, never the
numbering, decides what is primitive

## val TYPE_I128

```mach
pub val TYPE_I128: TypeKind = 26
```

## val TYPE_F16

```mach
pub val TYPE_F16: TypeKind = 27
```

IEEE binary16, appended like the 128-bit integers

## val TYPE_KIND_COUNT

```mach
pub val TYPE_KIND_COUNT: u32 = TYPE_F16::u32 + 1
```

one past the last kind, so an appended kind takes over this reference. the
type table refuses a kind at or past it, so a walker over 0..TYPE_KIND_COUNT
meets every kind a type can carry

