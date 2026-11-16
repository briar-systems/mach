[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **instance**

# mach.front.sema.instance

the instances a module's sema discovers and the typing of each, which lowering reads

an instance is one monomorphisation of a declaration: a generic at its type arguments, a
function with comptime value parameters at their values, or a function with a pack at its
element types. a frame is one typing of a stretch of syntax: frame 0 is the module's own
typing, which the module's typing tables hold, each instance has a root frame, and each
`$each` iteration has a frame under the frame its statement was typed in. a frame holds
what its typing decided differently from its parent's: expression and binding types, the
instance each call names, the verdict of each comptime gate, and the constant array each
`$each` over one reads. a lookup walks a frame and its parents, and a miss falls to the
module's typing tables

Source: [`src/front/sema/instance.mach`](../../../../src/front/sema/instance.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.value`](../comptime/value.md), [`mach.session.module`](../../session/module.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` |  |
| [`Instance`](#instance) | `rec` |  |
| [`Frame`](#frame) | `rec` |  |
| [`Set`](#set) | `rec` |  |
| [`Added`](#added) | `rec` |  |

| On `Kind` | |
| :--- | :--- |
| [`KIND_TYPE`](#kind), [`KIND_VALUE`](#kind), [`KIND_PACK`](#kind) | |

| On `Set` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release an instance set; a nil set, or one never made, holds nothing |
| [`count`](#count), [`at`](#at) | |
| [`add`](#add) | the instance `item` names, added when no instance has its key; the arrays it points at are copied, and its frame and chain are the set's to fill |
| [`find`](#find) | the instance with `item`'s key, if the set holds one |
| [`iteration_at`](#iteration_at) | the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent` |
| [`iteration_add`](#iteration_add) | the frame of an iteration, made the first time it is typed |
| [`frame_is_iteration`](#frame_is_iteration) | an iteration's frame, rather than an instance's or the module's |
| [`expr_record`](#expr_record), [`decl_record`](#decl_record), [`call_record`](#call_record), [`gate_record`](#gate_record) | |
| [`each_record`](#each_record) | the published constant value of the array the `$each` at `sid` reads |
| [`expr_type_at`](#expr_type_at), [`decl_type_at`](#decl_type_at) | |
| [`call_at`](#call_at) | the instance the call or generic reference `eid` names in frame `frame` |
| [`gate_at`](#gate_at), [`each_at`](#each_at) | |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | |
| [`FRAME_TEMPLATE`](#constants) | the module's own typing, held by its typing tables |

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

## Frame

```mach
pub rec Frame {
    parent: u32;
    stmt:   mach.front.ast.id.StmtId;
    index:  u32;
    exprs:  std.collections.map.Map[u32, mach.type.kind.TypeId];
    decls:  std.collections.map.Map[u32, mach.type.kind.TypeId];
    calls:  std.collections.map.Map[u32, u32];
    gates:  std.collections.map.Map[u32, bool];
    arrays: std.collections.map.Map[u32, mach.front.comptime.value.CTValue];
    # the next iteration frame whose key hashes alike
    next: u32;
}
```

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

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) res[Set, mach.base.fail.Fail]
```

### dnit

```mach
pub fun dnit(set: *Set)
```

release an instance set; a nil set, or one never made, holds nothing

### count

```mach
pub fun count(set: *Set) u32
```

### at

```mach
pub fun at(set: *Set, id: u32) *Instance
```

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

### decl_record

```mach
pub fun decl_record(set: *Set, frame: u32, did: mach.front.ast.id.DeclId, ty: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

### call_record

```mach
pub fun call_record(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId, id: u32) err[mach.base.fail.Fail]
```

### gate_record

```mach
pub fun gate_record(set: *Set, frame: u32, cond: mach.front.ast.id.ExprId, active: bool) err[mach.base.fail.Fail]
```

### each_record

```mach
pub fun each_record(set: *Set, frame: u32, sid: mach.front.ast.id.StmtId, array: mach.front.comptime.value.CTValue) err[mach.base.fail.Fail]
```

the published constant value of the array the `$each` at `sid` reads

### expr_type_at

```mach
pub fun expr_type_at(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId) opt[mach.type.kind.TypeId]
```

### decl_type_at

```mach
pub fun decl_type_at(set: *Set, frame: u32, did: mach.front.ast.id.DeclId) opt[mach.type.kind.TypeId]
```

### call_at

```mach
pub fun call_at(set: *Set, frame: u32, eid: mach.front.ast.id.ExprId) opt[u32]
```

the instance the call or generic reference `eid` names in frame `frame`

### gate_at

```mach
pub fun gate_at(set: *Set, frame: u32, cond: mach.front.ast.id.ExprId) opt[bool]
```

### each_at

```mach
pub fun each_at(set: *Set, frame: u32, sid: mach.front.ast.id.StmtId) opt[mach.front.comptime.value.CTValue]
```

## Added

```mach
pub rec Added {
    id:    u32;
    fresh: bool;
}
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` |  |
| `FRAME_TEMPLATE` | `u32` | `0` | the module's own typing, held by its typing tables |

