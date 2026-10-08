# mach.lang.fe.comptime.deep

the deep value of a constant: an array, record or case literal as a tree of nodes with every
member evaluated, an address as the symbol it points into, and a leaf that has no compile-time
value kept with its refusal. a store holds the trees of one module, every node under a tree's
root in that store, and a value names a node of it by the module's stable id

## def NodeKind

```mach
pub def NodeKind: u8
```

## val NODE_SCALAR

```mach
pub val NODE_SCALAR: NodeKind = 0
```

a value the evaluator holds whole

## val NODE_ARRAY

```mach
pub val NODE_ARRAY: NodeKind = 1
```

an array literal: its elements, in order

## val NODE_LITERAL

```mach
pub val NODE_LITERAL: NodeKind = 2
```

a record or case literal: the case its head selects, STR_NIL for a record, then its
named fields as written and its positional elements

## val NODE_REFUSED

```mach
pub val NODE_REFUSED: NodeKind = 3
```

a leaf that has no compile-time value, with the refusal reading it gives

## val NODE_ADDRESS

```mach
pub val NODE_ADDRESS: NodeKind = 4
```

the address of `symbol`, then of the place its members step to: a field, named, or an
element, unnamed, each a scalar holding its ordinal at the type it steps into

## rec Refusal

```mach
pub rec Refusal;
```

what reading a leaf with no compile-time value gives, and where that leaf is

## rec Symbol

```mach
pub rec Symbol;
```

the module-level function, `val` or `var` an address points into: its declaration `decl` in
the module with stable id `module`, which name it in every module that reads it

## rec Node

```mach
pub rec Node;
```

one node of a value; an aggregate's members are `members[first .. first + len]`

## rec Member

```mach
pub rec Member;
```

a member of an aggregate node: a named field, or an element or positional value with STR_NIL

## rec Store

```mach
pub rec Store;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator) Store;
```

## fun dnit

```mach
pub fun dnit(s: *Store);
```

## fun node_add

```mach
pub fun node_add(s: *Store, n: Node) res[u32, fail.Fail];
```

## fun members_add

```mach
pub fun members_add(s: *Store, node: u32, members: *Member, len: u32) err[fail.Fail];
```

the members of an aggregate are added together once its children are, so they are contiguous

## fun node_at

```mach
pub fun node_at(s: *Store, node: u32) *Node;
```

## fun member_at

```mach
pub fun member_at(s: *Store, n: *Node, ord: u32) opt[Member];
```

## fun refusal_of

```mach
pub fun refusal_of(itn: *intern.Interner, r: Refusal) comptime_failure.EvalFail;
```

## fun value

```mach
pub fun value(s: *Store, itn: *intern.Interner, node: u32) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

the value a scalar node holds, or the refusal a refused one gives; an aggregate has no
value of its own, and an address has none until the program is linked

## fun read

```mach
pub fun read(s: *Store, itn: *intern.Interner, module: u32, node: u32) res[comptime_value.CTValue, comptime_failure.EvalFail];
```

what reading node `node` of module `module`'s store gives: a scalar's value, a refused leaf's
or an address's refusal, or the aggregate itself as a value naming it

## def ProjectionKind

```mach
pub def ProjectionKind: u8
```

## val PROJECT_ABSENT

```mach
pub val PROJECT_ABSENT: ProjectionKind = 0
```

the node is no record or case literal

## val PROJECT_MEMBER

```mach
pub val PROJECT_MEMBER: ProjectionKind = 1
```

## val PROJECT_UNSELECTED

```mach
pub val PROJECT_UNSELECTED: ProjectionKind = 2
```

a case literal holding another case, `head`

## val PROJECT_NO_PAYLOAD

```mach
pub val PROJECT_NO_PAYLOAD: ProjectionKind = 3
```

a case literal holding the case but no payload for it

## val PROJECT_OMITTED

```mach
pub val PROJECT_OMITTED: ProjectionKind = 4
```

a record literal that leaves the field out, which holds its zero

## rec Projection

```mach
pub rec Projection;
```

## fun project

```mach
pub fun project(s: *Store, node: u32, name: intern.StrId) Projection;
```

member `name` of a record or case literal: a record's field, or a case literal's payload when
`name` is the case it holds

## def ElementKind

```mach
pub def ElementKind: u8
```

## val ELEMENT_ABSENT

```mach
pub val ELEMENT_ABSENT: ElementKind = 0
```

the node is no array

## val ELEMENT_MEMBER

```mach
pub val ELEMENT_MEMBER: ElementKind = 1
```

## val ELEMENT_OMITTED

```mach
pub val ELEMENT_OMITTED:  ElementKind = 2
```

an index the array declares but its literal leaves out, which holds its zero

## val ELEMENT_PAST_END

```mach
pub val ELEMENT_PAST_END: ElementKind = 3
```

## val INDEX_PAST_END_MSG

```mach
pub val INDEX_PAST_END_MSG: str = "this index is past the end of the constant it reads"
```

## rec Element

```mach
pub rec Element;
```

## fun element

```mach
pub fun element(s: *Store, node: u32, index: u64) Element;
```

element `index` of an array node

## fun refused_within

```mach
pub fun refused_within(s: *Store, node: u32) opt[u32];
```

the first leaf under `node` that has no compile-time value, none when every leaf has one

## fun clone

```mach
pub fun clone(to: *Store, from: *Store, node: u32) res[u32, fail.Fail];
```

node `node` of `from` with everything under it, added to `to`, which is another store

