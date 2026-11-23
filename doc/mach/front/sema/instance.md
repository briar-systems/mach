[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **instance**

# mach.front.sema.instance

the instances a module's sema discovers and the typing of each, which lowering reads

an instance is one monomorphisation of a declaration: a generic at its type arguments, a
function with comptime value parameters at their values, or a function with a pack at its
element types. a frame is one typing of a stretch of syntax: frame 0 is the module's own
typing, which the module's typing tables hold, each instance has a root frame, and each
`$each` iteration has a frame under the frame its statement was typed in. a frame holds
what its typing decided differently from its parent's: expression and binding types, the
instance each call names, the verdict of each comptime gate, the constant array each
`$each` over one reads, and each type node a comptime parameter or loop variable sizes. a lookup walks a frame and its parents, and a miss falls to the
module's typing tables

Source: [`src/front/sema/instance.mach`](../../../../src/front/sema/instance.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.session.module`](../../session/module.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` |  |
| [`Instance`](#instance) | `rec` | one instance of a declaration, keyed by its kind, origin, declaration and arguments |
| [`Frame`](#frame) | `rec` | one typing of a stretch of syntax, holding what it decided apart from its parent |
| [`Set`](#set) | `rec` | a module's instances and every frame of their typing |
| [`Added`](#added) | `rec` | the instance an `add` names |

| On `Kind` | |
| :--- | :--- |
| [`KIND_TYPE`](#kind), [`KIND_VALUE`](#kind), [`KIND_PACK`](#kind) | |

| On `Set` | |
| :--- | :--- |
| [`init`](#init) | an instance set holding only frame 0, the module's own typing |
| [`dnit`](#dnit) | release an instance set; a nil set, or one never made, holds nothing |
| [`count`](#count) | how many instances `set` holds, zero for a nil set |
| [`at`](#at) | the instance `id`, which must be below `count` |
| [`add`](#add) | the instance `item` names, added when no instance has its key; the arrays it points at are copied, and its frame and chain are the set's to fill |
| [`find`](#find) | the instance with `item`'s key, if the set holds one |
| [`iteration_at`](#iteration_at) | the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent` |
| [`iteration_add`](#iteration_add) | the frame of an iteration, made the first time it is typed |
| [`frame_is_iteration`](#frame_is_iteration) | an iteration's frame, rather than an instance's or the module's |
| [`expr_record`](#expr_record) | records `ty` as the type of the expression `eid` in `frame` |
| [`decl_record`](#decl_record) | records `ty` as the type of the binding `did` in `frame` |
| [`type_record`](#type_record) | records the type the frame resolves the type node `tid` to, which its comptime bindings size |
| [`call_record`](#call_record) | records that the call or generic reference `eid` names the instance `id` in `frame` |
| [`gate_record`](#gate_record) | records the verdict of the comptime gate whose condition is `cond` in `frame` |
| [`case_record`](#case_record) | records the case `code` a walk found fixed of the tag place `eid` read whole in `frame`, met with what an earlier walk of it in that frame found |
| [`case_at`](#case_at) | the case code of the tag place `eid` in `frame` or the nearest parent that records one, CASE_UNTYPED when none does |
| [`each_record`](#each_record) | the published constant value of the array the `$each` at `sid` reads |
| [`expr_type_at`](#expr_type_at) | the type of the expression `eid` in `frame` or the nearest parent that records one |
| [`decl_type_at`](#decl_type_at) | the type of the binding `did` in `frame` or the nearest parent that records one |
| [`type_in_frame`](#type_in_frame) | the type frame `frame` itself resolved the type node `tid` to, not one of its parents |
| [`type_at`](#type_at) | the type frame `frame` or the nearest of its parents resolved the type node `tid` to |
| [`call_at`](#call_at) | the instance the call or generic reference `eid` names in frame `frame` |
| [`gate_at`](#gate_at) | the verdict of the gate on `cond` in `frame` or the nearest parent that records one |
| [`each_at`](#each_at) | the constant array the `$each` at `sid` reads in `frame` or the nearest parent that records one |

| Functions | |
| :--- | :--- |
| [`case_code`](#case_code) | the code of the case `name` a live guard fixes, CASE_NONE for none |
| [`case_of`](#case_of) | the case a code names, none for CASE_NONE and CASE_UNTYPED |
| [`case_meet`](#case_meet) | what two walks of one place say together: a case only where every walk that typed it found the same one, since a place a `$if` or an iteration types twice is read in each |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | |
| [`FRAME_TEMPLATE`](#constants) | the module's own typing, held by its typing tables |
| [`CASE_UNTYPED`](#constants) | a tag place read whole that no walk has typed yet |
| [`CASE_NONE`](#constants) | a tag place read whole where no live guard fixes its case, on some walk that typed it |

---

## Kind

```mach
pub def Kind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `KIND_TYPE` | `0` |  |
| `KIND_VALUE` | `1` |  |
| `KIND_PACK` | `2` |  |

## Instance

```mach
pub rec Instance {
    kind:   Kind;
    origin: mach.session.module.ModuleId;
    decl:   mach.front.ast.id.DeclId;
    # the type arguments, for a generic
    args:    *mach.type.kind.TypeId;
    arg_len: u32;
    # the comptime value arguments and the parameters they bind
    values:    *mach.front.comptime.value.CTValue;
    names:     *mach.base.intern.StrId;
    value_len: u32;
    # the pack's element types
    pack:     *mach.type.kind.TypeId;
    pack_len: u32;
    # the declaration's name as written and as its module defines it, and the signature
    # the instance was asked for at
    bare:  mach.base.intern.StrId;
    canon: mach.base.intern.StrId;
    sig:   mach.type.kind.TypeId;
    # the growing cycle its declaration lies on, by the cycle's members, where `growing` holds
    growing: bool;
    cycle:   mach.base.intern.StrId;
    # the instance whose typing asked for this one first, how many such steps lie below
    # it, and the place it was asked for at
    parent:    u32;
    depth:     u32;
    site:      mach.base.source.Span;
    site_file: mach.base.source.FileId;
    frame:     u32;
    # the next instance whose key hashes alike
    next: u32;
}
```

one instance of a declaration, keyed by its kind, origin, declaration and arguments

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `Kind` | whether it instantiates type arguments, comptime values or a pack |
| `origin` | `mach.session.module.ModuleId` | the module that declares the template |
| `decl` | `mach.front.ast.id.DeclId` | the template's declaration in that module |
| `frame` | `u32` | the instance's root frame |

## Frame

```mach
pub rec Frame {
    parent: u32;
    stmt:   mach.front.ast.id.StmtId;
    index:  u32;
    exprs:  std.collections.map.Map[u32, mach.type.kind.TypeId];
    decls:  std.collections.map.Map[u32, mach.type.kind.TypeId];
    types:  std.collections.map.Map[u32, mach.type.kind.TypeId];
    calls:  std.collections.map.Map[u32, u32];
    gates:  std.collections.map.Map[u32, bool];
    arrays: std.collections.map.Map[u32, mach.front.comptime.value.CTValue];
    cases:  std.collections.map.Map[u32, u32];
    # the next iteration frame whose key hashes alike
    next: u32;
}
```

one typing of a stretch of syntax, holding what it decided apart from its parent

| Field | Type | |
| :--- | :--- | :--- |
| `parent` | `u32` | the frame an iteration was typed under, NONE for the module's and an instance's root |
| `stmt` | `mach.front.ast.id.StmtId` | the `$each` an iteration belongs to, nil for a root frame |
| `index` | `u32` | the iteration's index, or the instance id of a root frame |
| `exprs` | `std.collections.map.Map[u32, mach.type.kind.TypeId]` | expression types by expression index |
| `decls` | `std.collections.map.Map[u32, mach.type.kind.TypeId]` | binding types by declaration index |
| `types` | `std.collections.map.Map[u32, mach.type.kind.TypeId]` | the types of the type nodes the frame's comptime bindings size, by type node index |
| `calls` | `std.collections.map.Map[u32, u32]` | the instance each call names, by expression index |
| `gates` | `std.collections.map.Map[u32, bool]` | the verdict of each comptime gate, by condition expression index |
| `arrays` | `std.collections.map.Map[u32, mach.front.comptime.value.CTValue]` | the constant array each `$each` reads, by statement index |
| `cases` | `std.collections.map.Map[u32, u32]` | the case a live guard fixes of each tag place read whole, by expression index, as `case_code` encodes it |

## Set

```mach
pub rec Set {
    alloc:     *std.allocator.Allocator;
    items:     std.collections.vector.Vector[Instance];
    frames:    std.collections.vector.Vector[Frame];
    by_key:    std.collections.map.Map[u64, u32];
    iteration: std.collections.map.Map[u64, u32];
}
```

a module's instances and every frame of their typing

| Field | Type | |
| :--- | :--- | :--- |
| `alloc` | `*std.allocator.Allocator` | the allocator everything is held in, nil once released |
| `items` | `std.collections.vector.Vector[Instance]` | the instances, by id |
| `frames` | `std.collections.vector.Vector[Frame]` | the frames, by id, frame 0 the module's own typing |
| `by_key` | `std.collections.map.Map[u64, u32]` | the latest instance of each key hash, earlier ones chained through `next` |
| `iteration` | `std.collections.map.Map[u64, u32]` | the latest iteration frame of each iteration hash, earlier ones chained through `next` |

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) res[Set, mach.base.fail.Fail]
```

an instance set holding only frame 0, the module's own typing

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator the set is held in |
| **returns** | the set, released with `dnit`, or the failure of allocating its first frame |

### dnit

```mach
pub fun dnit(set: *Set)
```

release an instance set; a nil set, or one never made, holds nothing

### count

```mach
pub fun count(set: *Set) u32
```

how many instances `set` holds, zero for a nil set

| Parameter | |
| :--- | :--- |
| `set` | the set asked about |
| **returns** | the instance count |

### at

```mach
pub fun at(set: *Set, id: u32) *Instance
```

the instance `id`, which must be below `count`

| Parameter | |
| :--- | :--- |
| `set` | the set holding it |
| `id` | the instance's id |
| **returns** | the instance, owned by the set |

### add

```mach
pub fun add(set: *Set, item: Instance) res[Added, mach.base.fail.Fail]
```

the instance `item` names, added when no instance has its key; the arrays it points at are
copied, and its frame and chain are the set's to fill

### find

```mach
pub fun find(set: *Set, item: *Instance) opt[u32]
```

the instance with `item`'s key, if the set holds one

### iteration_at

```mach
pub fun iteration_at(set: *Set, parent: u32, stmt: mach.front.ast.id.StmtId, index: u32) opt[u32]
```

the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent`

### iteration_add

```mach
pub fun iteration_add(set: *Set, parent: u32, stmt: mach.front.ast.id.StmtId, index: u32) res[u32, mach.base.fail.Fail]
```

the frame of an iteration, made the first time it is typed

### frame_is_iteration

```mach
pub fun frame_is_iteration(set: *Set, frame: u32) bool
```

an iteration's frame, rather than an instance's or the module's

### expr_record

```mach
pub fun expr_record(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId, ty: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

records `ty` as the type of the expression `eid` in `frame`

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame typing it |
| `eid` | the expression |
| `ty` | its type in that frame |
| **returns** | ok, or the failure of the allocation |

### decl_record

```mach
pub fun decl_record(set: *Set, frame: u32, did: mach.front.ast.id.DeclId, ty: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

records `ty` as the type of the binding `did` in `frame`

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame typing it |
| `did` | the binding's declaration |
| `ty` | its type in that frame |
| **returns** | ok, or the failure of the allocation |

### type_record

```mach
pub fun type_record(set: *Set, frame: u32, tid: mach.front.ast.id.TypeId, ty: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

records the type the frame resolves the type node `tid` to, which its comptime bindings size

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame typing it |
| `tid` | the type node |
| `ty` | the type it resolves to in that frame |
| **returns** | ok, or the failure of the allocation |

### call_record

```mach
pub fun call_record(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId, id: u32) err[mach.base.fail.Fail]
```

records that the call or generic reference `eid` names the instance `id` in `frame`

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame typing it |
| `eid` | the call or reference |
| `id` | the instance it names |
| **returns** | ok, or the failure of the allocation |

### gate_record

```mach
pub fun gate_record(set: *Set, frame: u32, cond: mach.front.ast.id.ExprId, active: bool) err[mach.base.fail.Fail]
```

records the verdict of the comptime gate whose condition is `cond` in `frame`

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame typing it |
| `cond` | the gate's condition |
| `active` | whether the gate's arm is taken |
| **returns** | ok, or the failure of the allocation |

### case_record

```mach
pub fun case_record(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId, code: u32) err[mach.base.fail.Fail]
```

records the case `code` a walk found fixed of the tag place `eid` read whole in `frame`, met
with what an earlier walk of it in that frame found

### case_at

```mach
pub fun case_at(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId) u32
```

the case code of the tag place `eid` in `frame` or the nearest parent that records one,
CASE_UNTYPED when none does

### each_record

```mach
pub fun each_record(set: *Set, frame: u32, sid: mach.front.ast.id.StmtId, array: mach.front.comptime.value.CTValue) err[mach.base.fail.Fail]
```

the published constant value of the array the `$each` at `sid` reads

### expr_type_at

```mach
pub fun expr_type_at(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId) opt[mach.type.kind.TypeId]
```

the type of the expression `eid` in `frame` or the nearest parent that records one

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame asked |
| `eid` | the expression |
| **returns** | its type, or none when no frame up the chain records it |

### decl_type_at

```mach
pub fun decl_type_at(set: *Set, frame: u32, did: mach.front.ast.id.DeclId) opt[mach.type.kind.TypeId]
```

the type of the binding `did` in `frame` or the nearest parent that records one

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame asked |
| `did` | the binding's declaration |
| **returns** | its type, or none when no frame up the chain records it |

### type_in_frame

```mach
pub fun type_in_frame(set: *Set, frame: u32, tid: mach.front.ast.id.TypeId) opt[mach.type.kind.TypeId]
```

the type frame `frame` itself resolved the type node `tid` to, not one of its parents

### type_at

```mach
pub fun type_at(set: *Set, frame: u32, tid: mach.front.ast.id.TypeId) opt[mach.type.kind.TypeId]
```

the type frame `frame` or the nearest of its parents resolved the type node `tid` to

### call_at

```mach
pub fun call_at(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId) opt[u32]
```

the instance the call or generic reference `eid` names in frame `frame`

### gate_at

```mach
pub fun gate_at(set: *Set, frame: u32, cond: mach.front.ast.id.ExprId) opt[bool]
```

the verdict of the gate on `cond` in `frame` or the nearest parent that records one

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame asked |
| `cond` | the gate's condition |
| **returns** | whether its arm is taken, or none when no frame up the chain records it |

### each_at

```mach
pub fun each_at(set: *Set, frame: u32, sid: mach.front.ast.id.StmtId) opt[mach.front.comptime.value.CTValue]
```

the constant array the `$each` at `sid` reads in `frame` or the nearest parent that records one

| Parameter | |
| :--- | :--- |
| `set` | the set holding the frame |
| `frame` | the frame asked |
| `sid` | the `$each` statement |
| **returns** | the array's value, or none when no frame up the chain records it |

## Added

```mach
pub rec Added {
    id:    u32;
    fresh: bool;
}
```

the instance an `add` names

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `u32` | the instance's id in the set |
| `fresh` | `bool` | whether the call added it, rather than finding it already there |

## Functions

### case_code

```mach
pub fun case_code(name: mach.base.intern.StrId) u32
```

the code of the case `name` a live guard fixes, CASE_NONE for none

### case_of

```mach
pub fun case_of(code: u32) opt[mach.base.intern.StrId]
```

the case a code names, none for CASE_NONE and CASE_UNTYPED

### case_meet

```mach
pub fun case_meet(had: u32, found: u32) u32
```

what two walks of one place say together: a case only where every walk that typed it found
the same one, since a place a `$if` or an iteration types twice is read in each

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` |  |
| `FRAME_TEMPLATE` | `u32` | `0` | the module's own typing, held by its typing tables |
| `CASE_UNTYPED` | `u32` | `0` | a tag place read whole that no walk has typed yet |
| `CASE_NONE` | `u32` | `1` | a tag place read whole where no live guard fixes its case, on some walk that typed it |

