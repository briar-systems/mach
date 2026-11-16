[mach](../README.md) / [type](README.md) / **kind**

# mach.type.kind

type ids and the kinds of type the type table holds

a `TypeId` is a type's index in the type table, with two reserved values for no type and the
error type. a new kind is appended after the last, the catalog of `mach.type.prim` decides
which kinds are primitive, and `TYPE_KIND_COUNT` bounds them.

Source: [`src/type/kind.mach`](../../../src/type/kind.mach)

| Type | | |
| :--- | :--- | :--- |
| [`TypeId`](#typeid) | `def` | the identity of one type in the type table |
| [`TypeKind`](#typekind) | `def` |  |

| On `TypeId` | |
| :--- | :--- |
| [`TYPE_NIL`](#typeid), [`TYPE_ERROR`](#typeid), [`id`](#id), [`index`](#index), [`same`](#same), [`is_nil`](#is_nil), [`is_error`](#is_error) | |

| On `TypeKind` | |
| :--- | :--- |
| [`TYPE_U8`](#typekind), [`TYPE_U16`](#typekind), [`TYPE_U32`](#typekind), [`TYPE_U64`](#typekind), [`TYPE_I8`](#typekind), [`TYPE_I16`](#typekind), [`TYPE_I32`](#typekind), [`TYPE_I64`](#typekind), [`TYPE_F32`](#typekind), [`TYPE_F64`](#typekind), [`TYPE_PTR`](#typekind), [`TYPE_POINTER`](#typekind), [`TYPE_ARRAY`](#typekind), [`TYPE_FUN`](#typekind), [`TYPE_REC`](#typekind), [`TYPE_UNI`](#typekind), [`TYPE_GENERIC_PARAM`](#typekind), [`TYPE_INSTANCE`](#typekind), [`TYPE_PACK`](#typekind), [`TYPE_SECRET`](#typekind), [`TYPE_VECTOR`](#typekind), [`TYPE_HANDLE`](#typekind), [`TYPE_ABI`](#typekind), [`TYPE_TAG`](#typekind), [`TYPE_CASE_SELECTOR`](#typekind) | |
| [`TYPE_U128`](#typekind) | the 128-bit integers are appended kinds: the catalog, never the numbering, decides what is primitive |
| [`TYPE_I128`](#typekind) | |
| [`TYPE_F16`](#typekind) | IEEE binary16, appended like the 128-bit integers |
| [`TYPE_U256`](#typekind) | the 256 and 512-bit integers, appended like the 128-bit integers |
| [`TYPE_I256`](#typekind), [`TYPE_U512`](#typekind), [`TYPE_I512`](#typekind), [`TYPE_U1`](#typekind) | |

| Constants | |
| :--- | :--- |
| [`TABLE_INDEX_TEXT`](#constants) | an index the type table issued no longer names a slot: a defect, never an input fault |
| [`TYPE_KIND_COUNT`](#constants) | one past the last kind, so an appended kind takes over this reference |

---

## TypeId

```mach
pub def TypeId: u32
```

the identity of one type in the type table

| Name | Value | |
| :--- | :--- | :--- |
| `TYPE_NIL` | `0xFFFFFFFF` |  |
| `TYPE_ERROR` | `0xFFFFFFFE` |  |

### id

```mach
pub fun id(index: u32) TypeId
```

### index

```mach
pub fun index(t: TypeId) u32
```

### same

```mach
pub fun same(left: TypeId, right: TypeId) bool
```

### is_nil

```mach
pub fun is_nil(t: TypeId) bool
```

### is_error

```mach
pub fun is_error(t: TypeId) bool
```

## TypeKind

```mach
pub def TypeKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `TYPE_U8` | `0` |  |
| `TYPE_U16` | `1` |  |
| `TYPE_U32` | `2` |  |
| `TYPE_U64` | `3` |  |
| `TYPE_I8` | `4` |  |
| `TYPE_I16` | `5` |  |
| `TYPE_I32` | `6` |  |
| `TYPE_I64` | `7` |  |
| `TYPE_F32` | `8` |  |
| `TYPE_F64` | `9` |  |
| `TYPE_PTR` | `10` |  |
| `TYPE_POINTER` | `11` |  |
| `TYPE_ARRAY` | `12` |  |
| `TYPE_FUN` | `13` |  |
| `TYPE_REC` | `14` |  |
| `TYPE_UNI` | `15` |  |
| `TYPE_GENERIC_PARAM` | `16` |  |
| `TYPE_INSTANCE` | `17` |  |
| `TYPE_PACK` | `18` |  |
| `TYPE_SECRET` | `19` |  |
| `TYPE_VECTOR` | `20` |  |
| `TYPE_HANDLE` | `21` |  |
| `TYPE_ABI` | `22` |  |
| `TYPE_TAG` | `23` |  |
| `TYPE_CASE_SELECTOR` | `24` |  |
| `TYPE_U128` | `25` | the 128-bit integers are appended kinds: the catalog, never the numbering, decides what is primitive |
| `TYPE_I128` | `26` |  |
| `TYPE_F16` | `27` | IEEE binary16, appended like the 128-bit integers |
| `TYPE_U256` | `28` | the 256 and 512-bit integers, appended like the 128-bit integers |
| `TYPE_I256` | `29` |  |
| `TYPE_U512` | `30` |  |
| `TYPE_I512` | `31` |  |
| `TYPE_U1` | `32` |  |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `TABLE_INDEX_TEXT` | `str` | `"type table index out of bounds"` | an index the type table issued no longer names a slot: a defect, never an input fault |
| `TYPE_KIND_COUNT` | `u32` | `TYPE_U1::u32 + 1` | one past the last kind, so an appended kind takes over this reference |

