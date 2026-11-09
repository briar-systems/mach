# mach.lang.type.shape

the mirl shape of a source type: the one place mach decides how a type is held in mirl's
type table, which lowering interns into its module's table and sema into the table it lays
types out over, so the size sema answers and the type codegen builds cannot disagree

a record or union is its fields in order at the alignment and packing it declares, a tag
its discriminator then the union of its cases' payloads, an opaque abi type a structure of
its bytes at its alignment. what reaching a member type means, substitution, a memo, a
cycle on the path, is the caller's, through `Reach`

## tag Unshaped

```mach
pub tag Unshaped: u8 {
    member;
    unknown;
    empty_tag;
    discriminator;
    table: types.Error;
    alloc: A.Error;
}
```

why a source type has no shape

member: reaching a member type refused it, the caller's reason held by its context
unknown: a kind that has no shape: a type parameter, a pack, an unresolved nominal
empty_tag: a tag with no cases
discriminator: a tag with no discriminator type
table: mirl's type table refused a type
alloc: no storage for a member list

## rec Reach

```mach
pub rec Reach[T];
```

how a caller reaches what the shape of a type is made of

ctx: the caller's context
member: the type of a member, element or payload, interned in the same table, none when
         the caller refuses it with a reason it holds
handle: the type of a target handle
pointer: the type a pointer is held in

## fun integer

```mach
pub fun integer(t: *types.Table, bits: u32) res[Type, Unshaped];
```

a signless integer of `bits`, rounded up to a width mirl holds

## fun format

```mach
pub fun format(bits: u32) opt[types_format.Format];
```

the format of a float of `bits`, none for a width mirl has no format for

## fun scalar

```mach
pub fun scalar(t: *types.Table, d: type_prim.PrimDesc) res[Type, Unshaped];
```

an integer or float primitive

## fun opaque

```mach
pub fun opaque(t: *types.Table, size: u32, align: u32) res[Type, Unshaped];
```

an opaque type of `size` bytes at the alignment `align`

## fun of

```mach
pub fun of[T](t: *types.Table, ti: *type.TypeInterner, tid: type_kind.TypeId, r: *Reach[T]) res[Type, Unshaped];
```

the shape of the source type `tid`, its secrecy and its substitution already the caller's.
a nil type is an empty structure

