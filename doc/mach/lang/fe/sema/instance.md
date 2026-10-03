# mach.lang.fe.sema.instance

the instances a module's sema discovers and the typing of each, which lowering reads

an instance is one monomorphisation of a declaration: a generic at its type arguments, a
function with comptime value parameters at their values, or a function with a pack at its
element types. a frame is one typing of a stretch of syntax: frame 0 is the module's own
typing, which the module's typing tables hold, each instance has a root frame, and each
`$each` iteration has a frame under the frame its statement was typed in. a frame holds
what its typing decided differently from its parent's: expression and binding types, the
instance each call names, and the verdict of each comptime gate. a lookup walks a frame
and its parents, and a miss falls to the module's typing tables

## def Kind

```mach
pub def Kind: u8
```

## val KIND_TYPE

```mach
pub val KIND_TYPE:  Kind = 0
```

## val KIND_VALUE

```mach
pub val KIND_VALUE: Kind = 1
```

## val KIND_PACK

```mach
pub val KIND_PACK:  Kind = 2
```

## def Reach

```mach
pub def Reach: u8
```

the objects an instance is emitted in: the module's normal object, its test object, or both

## val REACH_NONE

```mach
pub val REACH_NONE:    Reach = 0
```

## val REACH_NORMAL

```mach
pub val REACH_NORMAL:  Reach = 1
```

## val REACH_TESTING

```mach
pub val REACH_TESTING: Reach = 2
```

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

## val FRAME_TEMPLATE

```mach
pub val FRAME_TEMPLATE: u32 = 0
```

the module's own typing, held by its typing tables

## val FROM_TEMPLATE

```mach
pub val FROM_TEMPLATE: u32 = 0xFFFFFFFF
```

who asked for an instance: an instance id, or one of these

## val FROM_NORMAL

```mach
pub val FROM_NORMAL:   u32 = 0xFFFFFFFE
```

## val FROM_TESTING

```mach
pub val FROM_TESTING:  u32 = 0xFFFFFFFD
```

## rec Instance

```mach
pub rec Instance;
```

## rec Frame

```mach
pub rec Frame;
```

## rec Set

```mach
pub rec Set;
```

## rec Added

```mach
pub rec Added;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator) res[Set, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(set: *Set);
```

## fun count

```mach
pub fun count(set: *Set) u32;
```

## fun at

```mach
pub fun at(set: *Set, id: u32) *Instance;
```

## fun add

```mach
pub fun add(set: *Set, item: Instance) res[Added, fail.Fail];
```

the instance `item` names, added when no instance has its key; the arrays it points at are
copied, and its frame, reach and chain are the set's to fill

## fun find

```mach
pub fun find(set: *Set, item: *Instance) opt[u32];
```

the instance with `item`'s key, if the set holds one

## fun iteration_at

```mach
pub fun iteration_at(set: *Set, parent: u32, stmt: ast_id.StmtId, index: u32) opt[u32];
```

the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent`

## fun iteration_add

```mach
pub fun iteration_add(set: *Set, parent: u32, stmt: ast_id.StmtId, index: u32) res[u32, fail.Fail];
```

the frame of an iteration, made the first time it is typed

## fun frame_is_iteration

```mach
pub fun frame_is_iteration(set: *Set, frame: u32) bool;
```

an iteration's frame, rather than an instance's or the module's

## fun expr_record

```mach
pub fun expr_record(set: *Set, frame: u32, eid: ast_id.ExprId, ty: type.TypeId) err[fail.Fail];
```

## fun decl_record

```mach
pub fun decl_record(set: *Set, frame: u32, did: ast_id.DeclId, ty: type.TypeId) err[fail.Fail];
```

## fun call_record

```mach
pub fun call_record(set: *Set, frame: u32, eid: ast_id.ExprId, id: u32) err[fail.Fail];
```

## fun gate_record

```mach
pub fun gate_record(set: *Set, frame: u32, cond: ast_id.ExprId, active: bool) err[fail.Fail];
```

## fun expr_type_at

```mach
pub fun expr_type_at(set: *Set, frame: u32, eid: ast_id.ExprId) opt[type.TypeId];
```

## fun decl_type_at

```mach
pub fun decl_type_at(set: *Set, frame: u32, did: ast_id.DeclId) opt[type.TypeId];
```

## fun call_at

```mach
pub fun call_at(set: *Set, frame: u32, eid: ast_id.ExprId) opt[u32];
```

the instance the call or generic reference `eid` names in frame `frame`

## fun gate_at

```mach
pub fun gate_at(set: *Set, frame: u32, cond: ast_id.ExprId) opt[bool];
```

## fun edge_add

```mach
pub fun edge_add(set: *Set, from: u32, to: u32) err[fail.Fail];
```

`from` asked for instance `to`: an instance id, or who asked from outside any instance

## fun reach_settle

```mach
pub fun reach_settle(set: *Set) err[fail.Fail];
```

decides which objects each instance is emitted in: those of the declarations whose bodies
reach it, through any chain of instances. an instance only a template reaches is emitted
in none

