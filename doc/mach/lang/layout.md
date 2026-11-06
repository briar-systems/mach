# mach.lang.layout

aggregate extent: the size and alignment of a type described as a tree of
nodes for one machine, and the layout of a tag's discriminator and payload

## val NODE_UNKNOWN

```mach
pub val NODE_UNKNOWN:   NodeKind = 0
```

## val NODE_SCALAR

```mach
pub val NODE_SCALAR:    NodeKind = 1
```

## val NODE_POINTER

```mach
pub val NODE_POINTER:   NodeKind = 2
```

## val NODE_VECTOR

```mach
pub val NODE_VECTOR:    NodeKind = 3
```

## val NODE_ARRAY

```mach
pub val NODE_ARRAY:     NodeKind = 4
```

## val NODE_AGGREGATE

```mach
pub val NODE_AGGREGATE: NodeKind = 5
```

## val NODE_ZERO

```mach
pub val NODE_ZERO:      NodeKind = 6
```

## val NODE_OPAQUE

```mach
pub val NODE_OPAQUE:    NodeKind = 7
```

## val NODE_TAG

```mach
pub val NODE_TAG:       NodeKind = 8
```

## val INTEGER_COUNT

```mach
pub val INTEGER_COUNT:  usize              = 8
```

the integer and float widths a primitive takes, the rows of `Scalars`

## val INTEGER_WIDTHS

```mach
pub val INTEGER_WIDTHS: [INTEGER_COUNT]u32 = [INTEGER_COUNT]u32;
```

## val FLOAT_WIDTHS

```mach
pub val FLOAT_WIDTHS:   [3]u32             = [3]u32;
```

## rec Scalars

```mach
pub rec Scalars;
```

the extents the target's owner lays each primitive out at: mirl's data layout of
the target's row, read once when the target resolves. a machine that holds none,
a target whose cell no owner has a row for yet (#4779) or a fixture, lays a scalar
out at its size and a pointer at `ptr_width`

held: whether the owner's extents are held
integers: one per INTEGER_WIDTHS row
floats: one per FLOAT_WIDTHS row
pointer: a pointer of the default address space, none where the owner holds
          pointers logically, with no representation in memory
vectors: the vector registers the owner fits a vector's alignment to under
          the build's selection, none where it states no register fit

## rec Registers

```mach
pub rec Registers;
```

the vector registers a vector aligns to: `narrow` the narrowest and `widest`
the widest the build meets

## fun scalars_none

```mach
pub fun scalars_none() Scalars;
```

## rec Machine

```mach
pub rec Machine;
```

`vector_bits` is the narrowest vector register and `register_bits` the
widest the selected extensions give, each a power of two times the one
before: 128 and 256 on x86-64 with avx

## fun machine

```mach
pub fun machine(ptr_width: u32, vector_bits: u32, register_bits: u32) Machine;
```

## fun with_scalars

```mach
pub fun with_scalars(m: Machine, s: Scalars) Machine;
```

`m` laying its primitives out at the owner's extents `s`, and its vectors at
the owner's registers where it states them

## rec Node

```mach
pub rec Node;
```

## def ExtentCause

```mach
pub def ExtentCause: u8
```

## val EXTENT_UNKNOWN_NODE

```mach
pub val EXTENT_UNKNOWN_NODE:   ExtentCause = 1
```

## val EXTENT_FIELD_RANGE

```mach
pub val EXTENT_FIELD_RANGE:    ExtentCause = 5
```

## val EXTENT_SIZE_OVERFLOW

```mach
pub val EXTENT_SIZE_OVERFLOW:  ExtentCause = 6
```

## rec Extent

```mach
pub rec Extent;
```

## fun unknown_extent

```mach
pub fun unknown_extent(cause: ExtentCause) Extent;
```

## fun extent

```mach
pub fun extent(size: u32, align: u32) Extent;
```

## fun node

```mach
pub fun node(kind: NodeKind) Node;
```

## fun checked_round_up

```mach
pub fun checked_round_up(value: u32, align: u32) Extent;
```

## fun extent_of

```mach
pub fun extent_of[T](ctx: *T, describe: fun(*T, u32) Node, field: fun(*T, u32, u32) u32,
id: u32, m: Machine) Extent;
```

## fun field_offset_of

```mach
pub fun field_offset_of[T](ctx: *T, describe: fun(*T, u32) Node,
field: fun(*T, u32, u32) u32, id: u32, field_ix: u32,
m: Machine) Extent;
```

## fun cause_name

```mach
pub fun cause_name(cause: ExtentCause) opt[str];
```

the text of an extent cause; absent for a tag outside the catalog

