[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **deep**

# mach.front.comptime.deep

the deep value of a constant: an array, record or case literal as a tree of nodes with every
member evaluated, an address as the symbol it points into, and a leaf that has no compile-time
value kept with its refusal. a store holds the trees of one module, every node under a tree's
root in that store, and a value names a node of it by the module's stable id

Source: [`src/front/comptime/deep.mach`](../../../../src/front/comptime/deep.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.comptime.failure`](failure.md), [`mach.front.comptime.value`](value.md), [`mach.type`](../../type.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`NodeKind`](#nodekind) | `def` |  |
| [`Refusal`](#refusal) | `rec` | what reading a leaf with no compile-time value gives, and where that leaf is |
| [`Symbol`](#symbol) | `rec` | the module-level function, `val` or `var` an address points into: its declaration `decl` in the module with stable id `module`, which name it in every module that reads it |
| [`Node`](#node) | `rec` | one node of a value; an aggregate's members are `members[first .. first + len]` |
| [`Member`](#member) | `rec` | a member of an aggregate node: a named field, or an element or positional value with STR_NIL |
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

| On `Store` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release a deep store; a nil store holds nothing |
| [`node_add`](#node_add) | |
| [`members_add`](#members_add) | the members of an aggregate are added together once its children are, so they are contiguous |
| [`node_at`](#node_at), [`member_at`](#member_at) | |
| [`value`](#value) | the value a scalar node holds, or the refusal a refused one gives; an aggregate has no value of its own, and an address has none until the program is linked |
| [`read`](#read) | what reading node `node` of module `module`'s store gives: a scalar's value, a refused leaf's or an address's refusal, or the aggregate itself as a value naming it |
| [`project`](#project) | member `name` of a record or case literal: a record's field, or a case literal's payload when `name` is the case it holds |
| [`element`](#element) | element `index` of an array node |
| [`refused_within`](#refused_within) | the first leaf under `node` that has no compile-time value, none when every leaf has one |
| [`clone`](#clone) | node `node` of `from` with everything under it, added to `to`, which is another store |
| [`zero_node`](#zero_node) | the zero of `ty` in `s`, which a member a literal leaves out holds: a record, union or array with no members, a scalar zero, or a tag's first declared case over the zero of its payload |

| Functions | |
| :--- | :--- |
| [`zero_defined`](#zero_defined) | whether `ty` has a zero: every type the type store holds does, an unresolved or error type does not |

| Constants | |
| :--- | :--- |
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
    kind:  NodeKind;
    ty:    mach.type.kind.TypeId;
    value: mach.front.comptime.value.CTValue;
    head:  mach.base.intern.StrId;
    first: u32;
    len:   u32;
    # an array's declared element count, of which `len` are held and the rest are zero
    declared: u32;
    refusal:  Refusal;
    symbol:   Symbol;
}
```

one node of a value; an aggregate's members are `members[first .. first + len]`

## Member

```mach
pub rec Member {
    name: mach.base.intern.StrId;
    node: u32;
}
```

a member of an aggregate node: a named field, or an element or positional value with STR_NIL

## Store

```mach
pub rec Store {
    nodes:   std.collections.vector.Vector[Node];
    members: std.collections.vector.Vector[Member];
    # the root of the zero of each type built in this store, which every reader of that zero shares
    zeros: std.collections.map.Map[mach.type.kind.TypeId, u32];
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

### node_add

```mach
pub fun node_add(s: *Store, n: Node) res[u32, mach.base.fail.Fail]
```

### members_add

```mach
pub fun members_add(s: *Store, node: u32, members: *Member, len: u32) err[mach.base.fail.Fail]
```

the members of an aggregate are added together once its children are, so they are contiguous

### node_at

```mach
pub fun node_at(s: *Store, node: u32) *Node
```

### member_at

```mach
pub fun member_at(s: *Store, n: *Node, ord: u32) opt[Member]
```

### value

```mach
pub fun value(s: *Store, itn: *mach.base.intern.Interner, node: u32) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

the value a scalar node holds, or the refusal a refused one gives; an aggregate has no
value of its own, and an address has none until the program is linked

### read

```mach
pub fun read(s: *Store, itn: *mach.base.intern.Interner, module: u32, node: u32) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

what reading node `node` of module `module`'s store gives: a scalar's value, a refused leaf's
or an address's refusal, or the aggregate itself as a value naming it

### project

```mach
pub fun project(s: *Store, node: u32, name: mach.base.intern.StrId) Projection
```

member `name` of a record or case literal: a record's field, or a case literal's payload when
`name` is the case it holds

### element

```mach
pub fun element(s: *Store, node: u32, index: u64) Element
```

element `index` of an array node

### refused_within

```mach
pub fun refused_within(s: *Store, node: u32) opt[u32]
```

the first leaf under `node` that has no compile-time value, none when every leaf has one

### clone

```mach
pub fun clone(to: *Store, from: *Store, node: u32) res[u32, mach.base.fail.Fail]
```

node `node` of `from` with everything under it, added to `to`, which is another store. the
copy recurses as deep as the value nests, so it asks for stack before it descends and refuses
a value nested deeper (`nesting`), which the node that asked reports. a failure leaves `to`
as it was

### zero_node

```mach
pub fun zero_node(s: *Store, types: *mach.type.TypeInterner, pointer_width: u32, ty: mach.type.kind.TypeId) res[u32, mach.base.fail.Fail]
```

the zero of `ty` in `s`, which a member a literal leaves out holds: a record, union or array
with no members, a scalar zero, or a tag's first declared case over the zero of its payload.
It is built the first time it is asked for and shared by every later reader, so `s` holds at
most one tree per distinct type. Nothing writes a node once its members are linked, which is
what makes the sharing safe. The tag's field table must be ready, and a tag with no cases is
an internal failure. A failure leaves `s` as it was.

## Projection

```mach
pub rec Projection {
    kind: ProjectionKind;
    node: u32;
    head: mach.base.intern.StrId;
}
```

## Element

```mach
pub rec Element {
    kind: ElementKind;
    node: u32;
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

