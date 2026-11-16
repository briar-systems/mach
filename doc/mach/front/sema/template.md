[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **template**

# mach.front.sema.template

the template part of a module's interface: what an importer's lowering reads to emit an
instance of one of the module's generic, comptime-parameter or pack functions. it holds an
owned copy of the module's syntax, its resolution, its typing and the constants each stage
bound, every node at the id it has in the module, so an importer's instance typing, keyed by
those ids, reads against it. a module with no such function holds none

<sub>source: [`src/front/sema/template.mach`](../../../../src/front/sema/template.mach) · imported by 3 modules</sub>

<sub>`type_kind` is [`mach.type.kind`](../../type/kind.md) · `A` is `std.allocator` · `ast` is [`mach.front.ast`](../ast.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `comptime_scope` is [`mach.front.comptime.scope`](../comptime/scope.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Typing`](#typing) | `rec` | the module's own typing of its syntax, by node id |
| [`Template`](#template) | `rec` | load, resolved, typed: the constants and decisions the load, resolve and sema stages bound env: the build and module the module's comptime reads |
| [`Captured`](#captured) | `rec` | what a template is captured from, all of which it copies |

| On `Template` | |
| :--- | :--- |
| [`none`](#none) | |
| [`dnit`](#dnit) | release a template set; a nil one, or one never made, holds nothing |

| On `Captured` | |
| :--- | :--- |
| [`capture`](#capture) | an owned copy of everything `from` names, from `alloc` |

---

## Typing

```mach
pub rec Typing {
    expr_type:        *type_kind.TypeId;
    type_resolved_ty: *type_kind.TypeId;
    decl_type:        *type_kind.TypeId;
    expr_count:       u32;
    type_count:       u32;
    decl_count:       u32;
}
```

the module's own typing of its syntax, by node id

## Template

```mach
pub rec Template {
    alloc:    *A.Allocator;
    present:  bool;
    a:        ast.Ast;
    source:   str;
    rr:       resolve.ResolveResult;
    typing:   Typing;
    load:     comptime_scope.Bindings;
    resolved: comptime_scope.Bindings;
    typed:    comptime_scope.Bindings;
    env:      comptime_scope.ComptimeEnv;
}
```

load, resolved, typed: the constants and decisions the load, resolve and sema stages bound
env: the build and module the module's comptime reads

### none

```mach
pub fun none(alloc: *A.Allocator) Template
```

### dnit

```mach
pub fun dnit(t: *Template)
```

release a template set; a nil one, or one never made, holds nothing

## Captured

```mach
pub rec Captured {
    a:        *ast.Ast;
    source:   str;
    rr:       *resolve.ResolveResult;
    typing:   Typing;
    load:     *comptime_scope.Bindings;
    resolved: *comptime_scope.Bindings;
    typed:    *comptime_scope.Bindings;
    env:      comptime_scope.ComptimeEnv;
}
```

what a template is captured from, all of which it copies

### capture

```mach
pub fun capture(alloc: *A.Allocator, from: Captured) res[Template, fail.Fail]
```

an owned copy of everything `from` names, from `alloc`

