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

<sub>source: [`src/front/sema/instance.mach`](../../../../src/front/sema/instance.mach) · imported by 17 modules</sub>

<sub>`module` is [`mach.session.module`](../../session/module.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `map` is `std.collections.map` · `A` is `std.allocator` · `collections_vector` is `std.collections.vector` · `Edge` is private · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` |  |
| [`Reach`](#reach) | `def` | the objects an instance is emitted in: the module's normal object, its test object, or both |
| [`Instance`](#instance) | `rec` |  |
| [`Frame`](#frame) | `rec` |  |
| [`Set`](#set) | `rec` |  |
| [`Added`](#added) | `rec` |  |

| On `Kind` | |
| :--- | :--- |
| [`KIND_TYPE`](#kind) · [`KIND_VALUE`](#kind) · [`KIND_PACK`](#kind) | |

| On `Reach` | |
| :--- | :--- |
| [`REACH_NONE`](#reach) · [`REACH_NORMAL`](#reach) · [`REACH_TESTING`](#reach) | |

| On `Set` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release an instance set; a nil set, or one never made, holds nothing |
| [`count`](#count) · [`at`](#at) | |
| [`add`](#add) | the instance `item` names, added when no instance has its key; the arrays it points at are copied, and its frame, reach and chain are the set's to fill |
| [`find`](#find) | the instance with `item`'s key, if the set holds one |
| [`iteration_at`](#iteration_at) | the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent` |
| [`iteration_add`](#iteration_add) | the frame of an iteration, made the first time it is typed |
| [`frame_is_iteration`](#frame_is_iteration) | an iteration's frame, rather than an instance's or the module's |
| [`expr_record`](#expr_record) · [`decl_record`](#decl_record) · [`call_record`](#call_record) · [`gate_record`](#gate_record) | |
| [`each_record`](#each_record) | the published constant value of the array the `$each` at `sid` reads |
| [`expr_type_at`](#expr_type_at) · [`decl_type_at`](#decl_type_at) | |
| [`call_at`](#call_at) | the instance the call or generic reference `eid` names in frame `frame` |
| [`gate_at`](#gate_at) · [`each_at`](#each_at) | |
| [`edge_add`](#edge_add) | `from` asked for instance `to`: an instance id, or who asked from outside any instance |
| [`reach_settle`](#reach_settle) | decides which objects each instance is emitted in: those of the declarations whose bodies reach it, through any chain of instances |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | |
| [`FRAME_TEMPLATE`](#constants) | the module's own typing, held by its typing tables |
| [`FROM_TEMPLATE`](#constants) | who asked for an instance: an instance id, or one of these |
| [`FROM_NORMAL`](#constants) · [`FROM_TESTING`](#constants) | |

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

## Reach

```mach
pub def Reach: u8
```

the objects an instance is emitted in: the module's normal object, its test object, or both

| Name | Value | |
| :--- | :--- | :--- |
| `REACH_NONE` | `0` |  |
| `REACH_NORMAL` | `1` |  |
| `REACH_TESTING` | `2` |  |

## Instance

```mach
pub rec Instance {
    kind:   Kind;
    origin: module.ModuleId;
    decl:   ast_id.DeclId;
    # the type arguments, for a generic
    args:    *type_kind.TypeId;
    arg_len: u32;
    # the comptime value arguments and the parameters they bind
    values:    *comptime_value.CTValue;
    names:     *intern.StrId;
    value_len: u32;
    # the pack's element types
    pack:     *type_kind.TypeId;
    pack_len: u32;
    # the declaration's name as written and as its module defines it, and the signature
    # the instance was asked for at
    bare:  intern.StrId;
    canon: intern.StrId;
    sig:   type_kind.TypeId;
    # the instance whose typing asked for this one first, how many such steps lie below
    # it, and the place it was asked for at
    parent:    u32;
    depth:     u32;
    site:      base_source.Span;
    site_file: base_source.FileId;
    frame:     u32;
    reach:     Reach;
    # the next instance whose key hashes alike
    next: u32;
}
```

## Frame

```mach
pub rec Frame {
    parent: u32;
    stmt:   ast_id.StmtId;
    index:  u32;
    exprs:  map.Map[u32, type_kind.TypeId];
    decls:  map.Map[u32, type_kind.TypeId];
    calls:  map.Map[u32, u32];
    gates:  map.Map[u32, bool];
    arrays: map.Map[u32, comptime_value.CTValue];
    # the next iteration frame whose key hashes alike
    next: u32;
}
```

## Set

```mach
pub rec Set {
    alloc:     *A.Allocator;
    items:     collections_vector.Vector[Instance];
    frames:    collections_vector.Vector[Frame];
    edges:     collections_vector.Vector[Edge];
    by_key:    map.Map[u64, u32];
    iteration: map.Map[u64, u32];
}
```

### init

```mach
pub fun init(alloc: *A.Allocator) res[Set, fail.Fail]
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
pub fun add(set: *Set, item: Instance) res[Added, fail.Fail]
```

the instance `item` names, added when no instance has its key; the arrays it points at are
copied, and its frame, reach and chain are the set's to fill

### find

```mach
pub fun find(set: *Set, item: *Instance) opt[u32]
```

the instance with `item`'s key, if the set holds one

### iteration_at

```mach
pub fun iteration_at(set: *Set, parent: u32, stmt: ast_id.StmtId, index: u32) opt[u32]
```

the frame of iteration `index` of the `$each` at `stmt` typed in frame `parent`

### iteration_add

```mach
pub fun iteration_add(set: *Set, parent: u32, stmt: ast_id.StmtId, index: u32) res[u32, fail.Fail]
```

the frame of an iteration, made the first time it is typed

### frame_is_iteration

```mach
pub fun frame_is_iteration(set: *Set, frame: u32) bool
```

an iteration's frame, rather than an instance's or the module's

### expr_record

```mach
pub fun expr_record(set: *Set, frame: u32, eid: ast_id.ExprId, ty: type_kind.TypeId) err[fail.Fail]
```

### decl_record

```mach
pub fun decl_record(set: *Set, frame: u32, did: ast_id.DeclId, ty: type_kind.TypeId) err[fail.Fail]
```

### call_record

```mach
pub fun call_record(set: *Set, frame: u32, eid: ast_id.ExprId, id: u32) err[fail.Fail]
```

### gate_record

```mach
pub fun gate_record(set: *Set, frame: u32, cond: ast_id.ExprId, active: bool) err[fail.Fail]
```

### each_record

```mach
pub fun each_record(set: *Set, frame: u32, sid: ast_id.StmtId, array: comptime_value.CTValue) err[fail.Fail]
```

the published constant value of the array the `$each` at `sid` reads

### expr_type_at

```mach
pub fun expr_type_at(set: *Set, frame: u32, eid: ast_id.ExprId) opt[type_kind.TypeId]
```

### decl_type_at

```mach
pub fun decl_type_at(set: *Set, frame: u32, did: ast_id.DeclId) opt[type_kind.TypeId]
```

### call_at

```mach
pub fun call_at(set: *Set, frame: u32, eid: ast_id.ExprId) opt[u32]
```

the instance the call or generic reference `eid` names in frame `frame`

### gate_at

```mach
pub fun gate_at(set: *Set, frame: u32, cond: ast_id.ExprId) opt[bool]
```

### each_at

```mach
pub fun each_at(set: *Set, frame: u32, sid: ast_id.StmtId) opt[comptime_value.CTValue]
```

### edge_add

```mach
pub fun edge_add(set: *Set, from: u32, to: u32) err[fail.Fail]
```

`from` asked for instance `to`: an instance id, or who asked from outside any instance

### reach_settle

```mach
pub fun reach_settle(set: *Set) err[fail.Fail]
```

decides which objects each instance is emitted in: those of the declarations whose bodies
reach it, through any chain of instances. an instance only a template reaches is emitted
in none

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
| `FROM_TEMPLATE` | `u32` | `0xFFFFFFFF` | who asked for an instance: an instance id, or one of these |
| `FROM_NORMAL` | `u32` | `0xFFFFFFFE` |  |
| `FROM_TESTING` | `u32` | `0xFFFFFFFD` |  |

