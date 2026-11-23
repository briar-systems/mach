[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **template**

# mach.front.sema.template

the template part of a module's interface: what an importer's lowering reads to emit an
instance of one of the module's generic, comptime-parameter or pack functions. it holds an
owned copy of the module's syntax, its resolution, its typing and the constants each stage
bound, every node at the id it has in the module, so an importer's instance typing, keyed by
those ids, reads against it. a module with no such function holds none

Source: [`src/front/sema/template.mach`](../../../../src/front/sema/template.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.ast`](../ast.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.resolve`](../resolve.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Typing`](#typing) | `rec` | the module's own typing of its syntax, by node id |
| [`Template`](#template) | `rec` | load, resolved, typed: the constants and decisions the load, resolve and sema stages bound env: the build and module the module's comptime reads |
| [`Captured`](#captured) | `rec` | what a template is captured from, all of which it copies |

| On `Template` | |
| :--- | :--- |
| [`none`](#none) | the template of a module with no generic, comptime-parameter or pack function: empty tables and `present` false |
| [`dnit`](#dnit) | release a template set; a nil one, or one never made, holds nothing |

| On `Captured` | |
| :--- | :--- |
| [`capture`](#capture) | an owned copy of everything `from` names, from `alloc` |

---

## Typing

```mach
pub rec Typing {
    expr_type:        *mach.type.kind.TypeId;
    guard_case:       *u32;
    type_resolved_ty: *mach.type.kind.TypeId;
    decl_type:        *mach.type.kind.TypeId;
    expr_count:       u32;
    type_count:       u32;
    decl_count:       u32;
}
```

the module's own typing of its syntax, by node id

## Template

```mach
pub rec Template {
    alloc:    *std.allocator.Allocator;
    present:  bool;
    a:        mach.front.ast.Ast;
    source:   str;
    rr:       mach.front.resolve.ResolveResult;
    typing:   Typing;
    load:     mach.front.comptime.scope.Bindings;
    resolved: mach.front.comptime.scope.Bindings;
    typed:    mach.front.comptime.scope.Bindings;
    env:      mach.front.comptime.scope.ComptimeEnv;
}
```

load, resolved, typed: the constants and decisions the load, resolve and sema stages bound
env: the build and module the module's comptime reads

### none

```mach
pub fun none(alloc: *std.allocator.Allocator) Template
```

the template of a module with no generic, comptime-parameter or pack function: empty
tables and `present` false

| Parameter | |
| :--- | :--- |
| `alloc` | the allocator its empty tables are held in |
| **returns** | the template, released with `dnit` |

### dnit

```mach
pub fun dnit(t: *Template)
```

release a template set; a nil one, or one never made, holds nothing

## Captured

```mach
pub rec Captured {
    a:        *mach.front.ast.Ast;
    source:   str;
    rr:       *mach.front.resolve.ResolveResult;
    typing:   Typing;
    load:     *mach.front.comptime.scope.Bindings;
    resolved: *mach.front.comptime.scope.Bindings;
    typed:    *mach.front.comptime.scope.Bindings;
    env:      mach.front.comptime.scope.ComptimeEnv;
}
```

what a template is captured from, all of which it copies

### capture

```mach
pub fun capture(alloc: *std.allocator.Allocator, from: Captured) res[Template, mach.base.fail.Fail]
```

an owned copy of everything `from` names, from `alloc`

