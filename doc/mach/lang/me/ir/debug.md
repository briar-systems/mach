# mach.lang.me.ir.debug

the debug types of one ir module: what a debug model says each source type a
variable, function or global names is. lowering builds the table beside the ir
types it lowers, so a debug producer reads it and never the front end's types.
an aggregate names the ir type it is laid out as, and every size and offset a
producer states comes from that one layout

## rec TypeId

```mach
pub rec TypeId;
```

a type's slot in its module's table

## val TYPE_NIL

```mach
pub val TYPE_NIL: TypeId = TypeId;
```

the type nothing describes: a variable or pointee a debug model leaves untyped

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

## def Kind

```mach
pub def Kind: u8
```

## val KIND_BASE

```mach
pub val KIND_BASE: Kind = 0
```

a scalar, by name, encoding and width

## val KIND_POINTER

```mach
pub val KIND_POINTER: Kind = 1
```

`elem` is the pointee, TYPE_NIL for an address of no described type

## val KIND_ARRAY

```mach
pub val KIND_ARRAY: Kind = 2
```

`count` elements of `elem`

## val KIND_VECTOR

```mach
pub val KIND_VECTOR: Kind = 3
```

`count` lanes of the base `elem`

## val KIND_STRUCT

```mach
pub val KIND_STRUCT: Kind = 4
```

the members are the fields in declaration order, member i at field i of `layout`

## val KIND_UNION

```mach
pub val KIND_UNION:  Kind = 5
```

## val KIND_TAG

```mach
pub val KIND_TAG: Kind = 6
```

the members are the cases in code order, a payloadless one typed TYPE_NIL, each
payload at the payload offset of `layout`

## def Encoding

```mach
pub def Encoding: u8
```

## val ENCODING_SIGNED

```mach
pub val ENCODING_SIGNED:   Encoding = 0
```

## val ENCODING_UNSIGNED

```mach
pub val ENCODING_UNSIGNED: Encoding = 1
```

## val ENCODING_FLOAT

```mach
pub val ENCODING_FLOAT:    Encoding = 2
```

## val ENCODING_ADDRESS

```mach
pub val ENCODING_ADDRESS: Encoding = 3
```

an address, as wide as the target's pointers

## rec Type

```mach
pub rec Type;
```

## rec Member

```mach
pub rec Member;
```

## rec Table

```mach
pub rec Table;
```

## fun init

```mach
pub fun init(a: *A.Allocator) Table;
```

## fun dnit

```mach
pub fun dnit(t: *Table);
```

## fun blank

```mach
pub fun blank(kind: Kind, name: intern.StrId) Type;
```

a type with no members, its kind and name set and every other field empty

## fun add

```mach
pub fun add(t: *Table, ty: Type) res[TypeId, fail.Fail];
```

## fun get

```mach
pub fun get(t: *Table, ty: TypeId) opt[*Type];
```

## fun members_add

```mach
pub fun members_add(t: *Table, ty: TypeId, count: u32) res[u32, fail.Fail];
```

gives `ty` `count` untyped, unnamed members, contiguous so each is member_at an
offset from the first, and returns where they start

## fun member_at

```mach
pub fun member_at(t: *Table, ty: *Type, i: u32) *Member;
```

member `i` of `ty`, nil when it has fewer

