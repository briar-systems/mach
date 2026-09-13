# mach.lang.fe.ast.type

## def TypeKind

```mach
pub def TypeKind: u8
```

## val TYPE_KIND_NAMED

```mach
pub val TYPE_KIND_NAMED:           TypeKind = 0
```

## val TYPE_KIND_PTR

```mach
pub val TYPE_KIND_PTR:             TypeKind = 1
```

## val TYPE_KIND_ARRAY

```mach
pub val TYPE_KIND_ARRAY:           TypeKind = 2
```

## val TYPE_KIND_FUN

```mach
pub val TYPE_KIND_FUN:             TypeKind = 3
```

## val TYPE_KIND_REC

```mach
pub val TYPE_KIND_REC:             TypeKind = 4
```

## val TYPE_KIND_UNI

```mach
pub val TYPE_KIND_UNI:             TypeKind = 5
```

## val TYPE_KIND_PACK

```mach
pub val TYPE_KIND_PACK:            TypeKind = 6
```

## val TYPE_KIND_SECRET

```mach
pub val TYPE_KIND_SECRET:          TypeKind = 7
```

## val TYPE_KIND_FIELD_TYPE

```mach
pub val TYPE_KIND_FIELD_TYPE:      TypeKind = 8
```

## val TYPE_KIND_POINTEE_OF

```mach
pub val TYPE_KIND_POINTEE_OF:      TypeKind = 9
```

## val TYPE_KIND_DISCRIMINANT_OF

```mach
pub val TYPE_KIND_DISCRIMINANT_OF: TypeKind = 10
```

## val TYPE_KIND_ERROR

```mach
pub val TYPE_KIND_ERROR:           TypeKind = 255
```

## rec TypeNamed

```mach
pub rec TypeNamed;
```

## rec TypePtr

```mach
pub rec TypePtr;
```

## rec TypeSecret

```mach
pub rec TypeSecret;
```

## rec TypeArray

```mach
pub rec TypeArray;
```

## rec TypeFun

```mach
pub rec TypeFun;
```

## rec TypeRec

```mach
pub rec TypeRec;
```

## rec TypeUni

```mach
pub rec TypeUni;
```

## rec TypeFieldType

```mach
pub rec TypeFieldType;
```

## rec TypePointeeOf

```mach
pub rec TypePointeeOf;
```

## rec TypeDiscriminantOf

```mach
pub rec TypeDiscriminantOf;
```

## rec Type

```mach
pub rec Type;
```

