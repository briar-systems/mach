[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **deep**

# mach.front.comptime.deep

the deep value of a constant: an array, record or case literal with every member evaluated, an
address as the symbol it points into, and a leaf that has no compile-time value kept with its
refusal. a value is a shape and a payload. the shape is everything about it but the data of its
scalar leaves: its kind, its type, a record's field names, an array's element count, a scalar's
value kind and width, and the shapes of its members, each at its offset in the payload. the
payload is the data of its scalar leaves, in member order, each at the width its shape states.
a store holds the shapes and payloads of one module's values: equal shapes are held once, so a
table of records costs one shape per distinct form and its data bytes, and a member's payload
lies inside its aggregate's. a node names a value of a store by its shape and where its payload
starts, and a value names a node of the store of the module with that stable id

Source: [`src/front/comptime/deep.mach`](../../../../src/front/comptime/deep.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.comptime.failure`](failure.md), [`mach.front.comptime.value`](value.md), [`mach.type`](../../type.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`NodeKind`](#nodekind) | `def` |  |
| [`Refusal`](#refusal) | `rec` | what reading a leaf with no compile-time value gives, and where that leaf is |
| [`Symbol`](#symbol) | `rec` | the module-level function, `val` or `var` an address points into: its declaration `decl` in the module with stable id `module`, which name it in every module that reads it |
| [`Node`](#node) | `rec` | a value of a store: its shape, and the offset of its payload in the store's bytes |
| [`Shape`](#shape) | `rec` | the form of a value apart from its scalar data |
| [`Member`](#member) | `rec` | a member of an aggregate value |
| [`Store`](#store) | `rec` |  |
| [`Projection`](#projection) | `rec` |  |
| [`Element`](#element-1) | `rec` |  |

| On `NodeKind` | |
| :--- | :--- |
| [`NODE_SCALAR`](#nodekind) | a value the evaluator holds whole |
| [`NODE_ARRAY`](#nodekind) | an array literal: its elements, in order |
| [`NODE_LITERAL`](#nodekind) | a record or case literal: the case its head selects, STR_NIL for a record, then its named fields as written and its positional elements |
| [`NODE_REFUSED`](#nodekind) | a leaf that has no compile-time value, with the refusal reading it gives |
| [`NODE_ADDRESS`](#nodekind) | the address of `symbol`, then of the place its members step to: a field, named, or an element, unnamed, each a scalar holding its ordinal at the type it steps into |

| On `Refusal` | |
| :--- | :--- |
| [`refusal_of`](#refusal_of) | |

| On `Node` | |
| :--- | :--- |
| [`NONE`](#node) | the node of no value |
| [`is_none`](#is_none), [`same`](#same) | |
| [`node_ref`](#node_ref) | the value a value names: node `node` of the store of the module with stable id `module`, read at type `ty` |
| [`node_of`](#node_of) | the node a value of kind CT_KIND_NODE names |

| On `Store` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release a deep store; a nil store holds nothing |
| [`shape_at`](#shape_at) | |
| [`hint_member`](#hint_member) | the shape member `ord`, named `name`, of a value of shape `hint` has: what a builder expects of the same member of the next value it builds of that form |
| [`scalar_add`](#scalar_add) | the scalar `v` of type `ty`; `hint` is the shape the builder expects it to have, which a column of a table shares from row to row, so it is taken without a lookup when it fits |
| [`refused_add`](#refused_add) | a leaf of type `ty` with no compile-time value, which gives `r` when read |
| [`aggregate_add`](#aggregate_add) | an array or a record or case literal of type `ty` with the members `members[0 .. len]`, each a node of `s`: the case `head` selects, and an array's declared element count |
| [`address_add`](#address_add) | the address of `symbol` of type `ty`, then of the place the steps `steps[0 .. len]` reach |
| [`member`](#member-1) | member `ord` of the aggregate or address `n`, which holds more than `ord` members |
| [`member_at`](#member_at) | |
| [`scalar`](#scalar) | the value of the scalar `n` |
| [`value`](#value) | the value a scalar node holds, or the refusal a refused one gives; an aggregate has no value of its own, and an address has none until the program is linked |
| [`read`](#read) | what reading node `node` of module `module`'s store gives: a scalar's value, a refused leaf's or an address's refusal, or the aggregate itself as a value naming it |
| [`project`](#project) | member `name` of a record or case literal: a record's field, or a case literal's payload when `name` is the case it holds |
| [`element`](#element) | element `index` of an array node |
| [`refused_within`](#refused_within) | the first leaf under `node` that has no compile-time value, none when every leaf has one |
| [`clone`](#clone) | node `node` of `from` with everything under it, added to `to`, which is another store: its shape and those under it, and its payload |
| [`zero_node`](#zero_node) | the zero of `ty` in `s`, which a member a literal leaves out holds: a record, union or array with no members, a scalar zero, or a tag's first declared case over the zero of its payload |

| Functions | |
| :--- | :--- |
| [`zero_defined`](#zero_defined) | whether `ty` has a zero: every type the type store holds does, an unresolved or error type does not |

| Constants | |
| :--- | :--- |
| [`NO_SHAPE`](#constants) | no shape a builder expects of a value |
| [`PROJECT_MEMBER`](#constants) | |
| [`PROJECT_UNSELECTED`](#constants) | a case literal holding another case, `head` |
| [`PROJECT_NO_PAYLOAD`](#constants) | a case literal holding the case but no payload for it |
| [`PROJECT_OMITTED`](#constants) | a record literal that leaves the field out, which holds its zero |
| [`ELEMENT_MEMBER`](#constants), [`ELEMENT_PAST_END`](#constants), [`INDEX_PAST_END_MSG`](#constants), [`ZERO_UNKNOWN_MSG`](#constants) | |
| [`SCRATCH_MODULE`](#constants) | the module index a scratch store of zeros answers to when a node of it is read as a value |
| [`INSTANCE_MODULE`](#constants) | the module index the store of the instance being typed or lowered answers to: the values its body builds belong to the instance, not to any module |

---

## NodeKind

```mach
pub def NodeKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `NODE_SCALAR` | `0` | a value the evaluator holds whole |
| `NODE_ARRAY` | `1` | an array literal: its elements, in order |
| `NODE_LITERAL` | `2` | a record or case literal: the case its head selects, STR_NIL for a record, then its named fields as written and its positional elements |
| `NODE_REFUSED` | `3` | a leaf that has no compile-time value, with the refusal reading it gives |
| `NODE_ADDRESS` | `4` | the address of `symbol`, then of the place its members step to: a field, named, or an element, unnamed, each a scalar holding its ordinal at the type it steps into |

## Refusal

```mach
pub rec Refusal {
    kind:    mach.front.comptime.failure.EvalFailKind;
    diag:    mach.base.diagnostic.kind.Kind;
    message: mach.base.intern.StrId;
    at:      mach.base.source.Location;
}
```

what reading a leaf with no compile-time value gives, and where that leaf is

### refusal_of

```mach
pub fun refusal_of(itn: *mach.base.intern.Interner, r: Refusal) mach.front.comptime.failure.EvalFail
```

## Symbol

```mach
pub rec Symbol {
    module:   u32;
    decl:     u32;
    canon:    mach.base.intern.StrId;
    function: bool;
}
```

the module-level function, `val` or `var` an address points into: its declaration `decl` in
the module with stable id `module`, which name it in every module that reads it

## Node

```mach
pub rec Node {
    shape: u32;
    at:    u32;
}
```

a value of a store: its shape, and the offset of its payload in the store's bytes

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `Node{shape: 0xFFFFFFFF, at: 0}` | the node of no value |

### is_none

```mach
pub fun is_none(n: Node) bool
```

### same

```mach
pub fun same(l: Node, r: Node) bool
```

### node_ref

```mach
pub fun node_ref(module: u32, node: Node, ty: u32) mach.front.comptime.value.CTValue
```

the value a value names: node `node` of the store of the module with stable id `module`, read
at type `ty`

### node_of

```mach
pub fun node_of(v: mach.front.comptime.value.CTValue) Node
```

the node a value of kind CT_KIND_NODE names

## Shape

```mach
pub rec Shape {
    kind:     NodeKind;
    width:    u8;
    signed:   bool;
    refuses:  bool;
    uniform:  bool;
    ty:       mach.type.kind.TypeId;
    head:     mach.base.intern.StrId;
    first:    u32;
    len:      u32;
    declared: u32;
    size:     u32;
    hash:     u64;
    value:    mach.front.comptime.value.CTValue;
    refusal:  Refusal;
    symbol:   Symbol;
}
```

the form of a value apart from its scalar data

| Field | Type | |
| :--- | :--- | :--- |
| `width` | `u8` | a scalar's payload is the low `width` bytes of its value's data, the bytes above them filled from its top bit when `signed` and with zero when not |
| `refuses` | `bool` | a refused leaf lies within it |
| `uniform` | `bool` | an array whose members all have the shape `first`, member `i` at `i` times its size; any other aggregate's members are the slots `first .. first + len` |
| `declared` | `u32` | an array's declared element count, of which `len` are held and the rest are zero |
| `size` | `u32` | the bytes of its payload |
| `value` | `mach.front.comptime.value.CTValue` | a scalar's value with its data zero |

## Member

```mach
pub rec Member {
    name: mach.base.intern.StrId;
    node: Node;
}
```

a member of an aggregate value

## Store

```mach
pub rec Store {
    shapes: std.collections.vector.Vector[Shape];
    slots:  std.collections.vector.Vector[Slot];
    bytes:  std.collections.vector.Vector[u8];
    # the last shape added under each fingerprint, which an equal shape reuses
    known: std.collections.map.Map[u64, u32];
    # the zero of each type built in this store, which every reader of that zero shares
    zeros: std.collections.map.Map[mach.type.kind.TypeId, Node];
}
```

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) Store
```

### dnit

```mach
pub fun dnit(s: *Store)
```

release a deep store; a nil store holds nothing

### shape_at

```mach
pub fun shape_at(s: *Store, n: Node) *Shape
```

### hint_member

```mach
pub fun hint_member(s: *Store, hint: u32, ord: u32, name: mach.base.intern.StrId) u32
```

the shape member `ord`, named `name`, of a value of shape `hint` has: what a builder expects of
the same member of the next value it builds of that form. NO_SHAPE when `hint` is none or holds
no such member

### scalar_add

```mach
pub fun scalar_add(s: *Store, ty: mach.type.kind.TypeId, v: mach.front.comptime.value.CTValue, hint: u32) res[Node, mach.base.fail.Fail]
```

the scalar `v` of type `ty`; `hint` is the shape the builder expects it to have, which a column of
a table shares from row to row, so it is taken without a lookup when it fits

### refused_add

```mach
pub fun refused_add(s: *Store, ty: mach.type.kind.TypeId, r: Refusal) res[Node, mach.base.fail.Fail]
```

a leaf of type `ty` with no compile-time value, which gives `r` when read

### aggregate_add

```mach
pub fun aggregate_add(s: *Store, kind: NodeKind, ty: mach.type.kind.TypeId, head: mach.base.intern.StrId, declared: u32, members: *Member, len: u32, hint: u32) res[Node, mach.base.fail.Fail]
```

an array or a record or case literal of type `ty` with the members `members[0 .. len]`, each a
node of `s`: the case `head` selects, and an array's declared element count. `hint` is the shape
the builder expects it to have, taken without a lookup when it fits

### address_add

```mach
pub fun address_add(s: *Store, ty: mach.type.kind.TypeId, symbol: Symbol, steps: *Member, len: u32) res[Node, mach.base.fail.Fail]
```

the address of `symbol` of type `ty`, then of the place the steps `steps[0 .. len]` reach

### member

```mach
pub fun member(s: *Store, n: Node, ord: u32) Member
```

member `ord` of the aggregate or address `n`, which holds more than `ord` members

### member_at

```mach
pub fun member_at(s: *Store, n: Node, ord: u32) opt[Member]
```

### scalar

```mach
pub fun scalar(s: *Store, n: Node) mach.front.comptime.value.CTValue
```

the value of the scalar `n`

### value

```mach
pub fun value(s: *Store, itn: *mach.base.intern.Interner, node: Node) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

the value a scalar node holds, or the refusal a refused one gives; an aggregate has no
value of its own, and an address has none until the program is linked

### read

```mach
pub fun read(s: *Store, itn: *mach.base.intern.Interner, module: u32, node: Node) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

what reading node `node` of module `module`'s store gives: a scalar's value, a refused leaf's
or an address's refusal, or the aggregate itself as a value naming it

### project

```mach
pub fun project(s: *Store, node: Node, name: mach.base.intern.StrId) Projection
```

member `name` of a record or case literal: a record's field, or a case literal's payload when
`name` is the case it holds

### element

```mach
pub fun element(s: *Store, node: Node, index: u64) Element
```

element `index` of an array node

### refused_within

```mach
pub fun refused_within(s: *Store, node: Node) opt[Node]
```

the first leaf under `node` that has no compile-time value, none when every leaf has one

### clone

```mach
pub fun clone(to: *Store, from: *Store, node: Node) res[Node, mach.base.fail.Fail]
```

node `node` of `from` with everything under it, added to `to`, which is another store: its
shape and those under it, and its payload. the copy recurses as deep as the shape nests, so it
asks for stack before it descends and refuses a value nested deeper (`nesting`), which the node
that asked reports. a failure leaves `to` as it was

### zero_node

```mach
pub fun zero_node(s: *Store, types: *mach.type.TypeInterner, pointer_width: u32, ty: mach.type.kind.TypeId) res[Node, mach.base.fail.Fail]
```

the zero of `ty` in `s`, which a member a literal leaves out holds: a record, union or array
with no members, a scalar zero, or a tag's first declared case over the zero of its payload.
It is built the first time it is asked for and shared by every later reader, so `s` holds at
most one per distinct type. The tag's field table must be ready, and a tag with no cases is
an internal failure. A failure leaves `s` as it was.

## Projection

```mach
pub rec Projection {
    kind: ProjectionKind;
    node: Node;
    head: mach.base.intern.StrId;
}
```

## Element

```mach
pub rec Element {
    kind: ElementKind;
    node: Node;
}
```

## Functions

### zero_defined

```mach
pub fun zero_defined(types: *mach.type.TypeInterner, ty: mach.type.kind.TypeId) bool
```

whether `ty` has a zero: every type the type store holds does, an unresolved or error type does not

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NO_SHAPE` | `u32` | `0xFFFFFFFF` | no shape a builder expects of a value |
| `PROJECT_MEMBER` | `ProjectionKind` | `1` |  |
| `PROJECT_UNSELECTED` | `ProjectionKind` | `2` | a case literal holding another case, `head` |
| `PROJECT_NO_PAYLOAD` | `ProjectionKind` | `3` | a case literal holding the case but no payload for it |
| `PROJECT_OMITTED` | `ProjectionKind` | `4` | a record literal that leaves the field out, which holds its zero |
| `ELEMENT_MEMBER` | `ElementKind` | `1` |  |
| `ELEMENT_PAST_END` | `ElementKind` | `3` |  |
| `INDEX_PAST_END_MSG` | `str` | `"this index is past the end of the constant it reads"` |  |
| `ZERO_UNKNOWN_MSG` | `str` | `"the zero of this field's type is not known"` |  |
| `SCRATCH_MODULE` | `u32` | `0xFFFFFFFF` | the module index a scratch store of zeros answers to when a node of it is read as a value |
| `INSTANCE_MODULE` | `u32` | `0xFFFFFFFE` | the module index the store of the instance being typed or lowered answers to: the values its body builds belong to the instance, not to any module |

