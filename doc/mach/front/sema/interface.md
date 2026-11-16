[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **interface**

# mach.front.sema.interface

what a module publishes for the lowering of every module that imports it, in place of
its syntax tree: the facts of each module-level function and binding, the deep value of
each module constant, and the public surface a `fwd` re-exports. it owns everything it
holds and points into no syntax tree, scope or other product

Source: [`src/front/sema/interface.mach`](../../../../src/front/sema/interface.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.deep`](../comptime/deep.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.template`](template.md), [`mach.session.module`](../../session/module.md).

| Type | | |
| :--- | :--- | :--- |
| [`Declaration`](#declaration) | `rec` | one module-level function, `val` or `var` |
| [`Param`](#param) | `rec` |  |
| [`Exported`](#exported) | `rec` | one symbol on the module's public surface, as an importer of it names it |
| [`Interface`](#interface) | `rec` |  |
| [`Reader`](#reader) | `rec` | how a lowering reads the interface another module published |

| On `Interface` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release an interface; a nil one, or one never made, holds nothing |
| [`declaration_add`](#declaration_add) | adds `d` with the parameters `params[0 .. params_len]`; a second declaration of one kind and canonical name leaves that name ambiguous, which a lookup by name refuses |
| [`declaration_at`](#declaration_at) | the declaration `decl` of the module, none for one the interface does not describe |
| [`declaration_for`](#declaration_for) | the one declaration of `kind` the module defines under `canon` |
| [`param_at`](#param_at), [`declaration_has_comptime_param`](#declaration_has_comptime_param) | |
| [`declaration_has_instances`](#declaration_has_instances) | a generic, comptime-parameter or pack function has instances in place of one definition |
| [`template_of`](#template_of) | the template part, none for a module with no generic, comptime-parameter or pack function |
| [`exported_add`](#exported_add), [`exported_count`](#exported_count), [`exported_at`](#exported_at) | |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants), [`DECLARATION_EXT`](#constants), [`DECLARATION_BODY`](#constants), [`DECLARATION_PACK`](#constants) | |
| [`DECLARATION_THREAD`](#constants) | a `#[thread]` var: each thread holds its own copy, which every reference reaches through the thread |
| [`DECLARATION_SOFT`](#constants) | a function declared `#[constant_time("soft")]` or `#[constant_time("hard")]`: what a constant-time caller may trust of it, from its declaration alone |
| [`DECLARATION_HARD`](#constants) | |
| [`DECLARATION_WEAK`](#constants) | a `#[weak]` function, a link-time default another definition may replace |

---

## Declaration

```mach
pub rec Declaration {
    kind:         mach.front.resolve.SymKind;
    canon:        mach.base.intern.StrId;
    decl:         mach.front.ast.id.DeclId;
    flags:        DeclarationFlags;
    generics:     u32;
    params_start: u32;
    params_len:   u32;
    op_set:       mach.base.intern.StrId;
    op_name:      mach.base.intern.StrId;
    value:        u32;
}
```

one module-level function, `val` or `var`

| Field | Type | |
| :--- | :--- | :--- |
| `decl` | `mach.front.ast.id.DeclId` | the declaration in its module, which keys the session's tables of it |
| `generics` | `u32` | how many type parameters a function declares |
| `params_start` | `u32` | where a function's parameters start in `params` |
| `op_set` | `mach.base.intern.StrId` | the instruction set and operation an `#[intrinsic]` names, STR_NIL without one |
| `value` | `u32` | the node in `values` of a `val`'s constant value, or of the value a `var`'s initializer gives it, NONE when it has none |

## Param

```mach
pub rec Param {
    comptime: bool;
}
```

## Exported

```mach
pub rec Exported {
    kind:                mach.front.resolve.SymKind;
    name:                mach.base.intern.StrId;
    canon:               mach.base.intern.StrId;
    origin:              mach.session.module.StableModuleId;
    module:              mach.session.module.StableModuleId;
    generic_arity:       u32;
    has_comptime_params: bool;
    has_pack_param:      bool;
}
```

one symbol on the module's public surface, as an importer of it names it

| Field | Type | |
| :--- | :--- | :--- |
| `origin` | `mach.session.module.StableModuleId` | the module that declares it, which may be one this module re-exports |
| `module` | `mach.session.module.StableModuleId` | the module a `pub use` names, for a module symbol |

## Interface

```mach
pub rec Interface {
    alloc:        *std.allocator.Allocator;
    declarations: std.collections.vector.Vector[Declaration];
    params:       std.collections.vector.Vector[Param];
    surface:      std.collections.vector.Vector[Exported];
    by_decl:      std.collections.map.Map[u32, u32];
    by_canon:     std.collections.map.Map[u64, u32];
    # the deep value of each of the module's constants, and of each array a `$each` of its
    # typing reads
    values: mach.front.comptime.deep.Store;
    # what an importer emits the module's instances from
    template: mach.front.sema.template.Template;
}
```

### init

```mach
pub fun init(alloc: *std.allocator.Allocator) Interface
```

### dnit

```mach
pub fun dnit(iface: *Interface)
```

release an interface; a nil one, or one never made, holds nothing

### declaration_add

```mach
pub fun declaration_add(iface: *Interface, d: Declaration, params: *Param, params_len: u32) err[mach.base.fail.Fail]
```

adds `d` with the parameters `params[0 .. params_len]`; a second declaration of one kind and
canonical name leaves that name ambiguous, which a lookup by name refuses

### declaration_at

```mach
pub fun declaration_at(iface: *Interface, decl: mach.front.ast.id.DeclId) opt[*Declaration]
```

the declaration `decl` of the module, none for one the interface does not describe

### declaration_for

```mach
pub fun declaration_for(iface: *Interface, kind: mach.front.resolve.SymKind, canon: mach.base.intern.StrId) res[*Declaration, mach.base.fail.Fail]
```

the one declaration of `kind` the module defines under `canon`

### param_at

```mach
pub fun param_at(iface: *Interface, d: *Declaration, ord: u32) opt[Param]
```

### declaration_has_comptime_param

```mach
pub fun declaration_has_comptime_param(iface: *Interface, d: *Declaration) bool
```

### declaration_has_instances

```mach
pub fun declaration_has_instances(iface: *Interface, d: *Declaration) bool
```

a generic, comptime-parameter or pack function has instances in place of one definition

### template_of

```mach
pub fun template_of(iface: *Interface) opt[*mach.front.sema.template.Template]
```

the template part, none for a module with no generic, comptime-parameter or pack function

### exported_add

```mach
pub fun exported_add(iface: *Interface, e: Exported) err[mach.base.fail.Fail]
```

### exported_count

```mach
pub fun exported_count(iface: *Interface) u32
```

### exported_at

```mach
pub fun exported_at(iface: *Interface, ord: u32) *Exported
```

## Reader

```mach
pub rec Reader {
    ctx:  ptr;
    read: fun(ptr, mach.session.module.ModuleId) res[*Interface, mach.base.fail.Fail];
}
```

how a lowering reads the interface another module published

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` |  |
| `DECLARATION_EXT` | `DeclarationFlags` | `1` |  |
| `DECLARATION_BODY` | `DeclarationFlags` | `2` |  |
| `DECLARATION_PACK` | `DeclarationFlags` | `4` |  |
| `DECLARATION_THREAD` | `DeclarationFlags` | `8` | a `#[thread]` var: each thread holds its own copy, which every reference reaches through the thread |
| `DECLARATION_SOFT` | `DeclarationFlags` | `16` | a function declared `#[constant_time("soft")]` or `#[constant_time("hard")]`: what a constant-time caller may trust of it, from its declaration alone |
| `DECLARATION_HARD` | `DeclarationFlags` | `32` |  |
| `DECLARATION_WEAK` | `DeclarationFlags` | `64` | a `#[weak]` function, a link-time default another definition may replace |

