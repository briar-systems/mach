[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **product**

# mach.front.sema.product

what sema publishes for a module, and how one module reads another's

Source: [`src/front/sema/product.mach`](../../../../src/front/sema/product.mach)

Uses types from [`mach.base.diagnostic`](../../base/diagnostic.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.comptime.deep`](../comptime/deep.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.refusal`](../refusal.md), [`mach.front.resolve`](../resolve.md), [`mach.front.sema.fields`](fields.md), [`mach.front.sema.growth`](growth.md), [`mach.front.sema.instance`](instance.md), [`mach.front.sema.interface`](interface.md), [`mach.report.phase`](../../report/phase.md), [`mach.session.module`](../../session/module.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`SemaResult`](#semaresult) | `rec` | what sema produced for one module, owned by the result and released with `result_dnit` |
| [`InstanceResult`](#instanceresult) | `rec` | one instance typed on its own, which every user of it reads |
| [`TypeExport`](#typeexport) | `rec` | a constant an array, record or case literal gives is a value naming a node of the store its module's interface publishes |
| [`ModuleSema`](#modulesema) | `rec` | exports are append-only, and `index` maps (origin, canon) to the first export with that key over the first `indexed` of them |
| [`DefinitionPhase`](#definitionphase) | `def` |  |
| [`Definition`](#definition) | `rec` | a module's definition as far as a phase acquired it |
| [`DefinitionReader`](#definitionreader) | `rec` | reads the definitions of other modules, keeping the one acquired for each origin |
| [`DefinedSymbol`](#definedsymbol) | `rec` | a symbol and the definition of the module that declares it |
| [`SemaDeps`](#semadeps) | `rec` | the imported surfaces are borrowed: the driver owns each one and shares it between every importer in a pass |

| On `SemaResult` | |
| :--- | :--- |
| [`result_dnit`](#result_dnit) | release a semantic result; a nil result holds nothing |
| [`result_shed`](#result_shed) | give back what only the module's own lowering reads of a semantic result, once it is lowered: its typing, its bound constants, its instances and its growth |

| On `InstanceResult` | |
| :--- | :--- |
| [`instance_dnit`](#instance_dnit) | release a typed instance; a nil one holds nothing |

| On `ModuleSema` | |
| :--- | :--- |
| [`module_sema_init`](#module_sema_init) | an empty surface for one module |
| [`module_sema_dnit`](#module_sema_dnit) | releases the exports, graph and index of `module` |
| [`module_export_for`](#module_export_for) | the export of `module` keyed (origin, canon), bringing the index up to the current exports |

| On `DefinitionPhase` | |
| :--- | :--- |
| [`DEFINITION_PARSED`](#definitionphase), [`DEFINITION_RESOLVED`](#definitionphase), [`DEFINITION_TYPED`](#definitionphase) | |

| On `DefinitionReader` | |
| :--- | :--- |
| [`reader_init`](#reader_init) | a reader lives for one computation, which acquires the same current definition once per origin |
| [`reader_dnit`](#reader_dnit) | release a reader; a nil reader holds nothing |
| [`acquire_definition`](#acquire_definition) | the definition of module `origin` as far as each phase got: an importer is checked against what a rejected module resolved and typed, since what it refused carries the error type |
| [`acquire_symbol`](#acquire_symbol) | the current definition of `requested`, which module `origin` defines as this operation numbers it |

| On `SemaDeps` | |
| :--- | :--- |
| [`surfaces_set`](#surfaces_set) | hand `deps` the surfaces `entries` holds, an array of `count` allocated from `a` |
| [`surfaces_dnit`](#surfaces_dnit) | release what surfaces_set handed `deps`: the array, from `a`, and the index; the surfaces are borrowed |
| [`surface_of_owner`](#surface_of_owner) | the surface of the module `owner` among those `deps` holds, none when it holds none |

| Constants | |
| :--- | :--- |
| [`INFERRED_LEN_NONE`](#constants), [`INFERRED_LEN_FAILED`](#constants) | |
| [`INFERRED_LEN_NEEDS_LITERAL`](#constants) | a `[_]T` annotation on a binding whose initializer is absent or not an array literal |

---

## SemaResult

```mach
pub rec SemaResult {
    field_graph:      mach.front.sema.fields.Graph;
    status:           mach.report.phase.PhaseStatus;
    alloc:            *std.allocator.Allocator;
    expr_type:        *mach.type.kind.TypeId;
    type_resolved_ty: *mach.type.kind.TypeId;
    decl_type:        *mach.type.kind.TypeId;
    expr_count:       u32;
    type_count:       u32;
    decl_count:       u32;
    embed_paths:      *mach.base.intern.StrId;
    embed_path_count: u32;
    embed_path_cap:   u32;
    # the constants sema bound and the gates it decided, over the load's and resolve's
    constants: mach.front.comptime.scope.Bindings;
    # every instance the module's own code asks for, with where and from what each is asked,
    # and the calls and gates of its own typing (frame 0); each instance is typed by itself
    # (InstanceResult), not here
    instances: mach.front.sema.instance.Set;
    # whether the module's annotations reach a secret, which an instance of its templates
    # starts its own answer from
    has_secret: bool;
    # the growing cycles among the module's templates
    growth: mach.front.sema.growth.Analysis;
    # what the lowering of every importer reads of the module
    interface: mach.front.sema.interface.Interface;
    # why typing refused the module, if it did: a module-scope `$error` that only typing
    # decides fired in it, or a module it imports was refused
    refusal: mach.front.refusal.Refusal;
}
```

what sema produced for one module, owned by the result and released with `result_dnit`

| Field | Type | |
| :--- | :--- | :--- |
| `field_graph` | `mach.front.sema.fields.Graph` | the field tables of the types the module was typed with |
| `status` | `mach.report.phase.PhaseStatus` | whether typing passed, rejected the module or failed internally |
| `alloc` | `*std.allocator.Allocator` | the allocator the typing arrays and embed paths are held in |
| `expr_type` | `*mach.type.kind.TypeId` | the type of each expression, `expr_count` of them |
| `type_resolved_ty` | `*mach.type.kind.TypeId` | the type each type node resolved to, `type_count` of them |
| `decl_type` | `*mach.type.kind.TypeId` | the type of each declaration, `decl_count` of them |
| `expr_count` | `u32` | how many expressions `expr_type` holds |
| `type_count` | `u32` | how many type nodes `type_resolved_ty` holds |
| `decl_count` | `u32` | how many declarations `decl_type` holds |
| `embed_paths` | `*mach.base.intern.StrId` | the path of each file an `embed` read, `embed_path_count` of them |
| `embed_path_count` | `u32` | how many embed paths are recorded |
| `embed_path_cap` | `u32` | how many embed paths `embed_paths` has room for |

### result_dnit

```mach
pub fun result_dnit(r: *SemaResult)
```

release a semantic result; a nil result holds nothing

### result_shed

```mach
pub fun result_shed(r: *SemaResult)
```

give back what only the module's own lowering reads of a semantic result, once it is lowered:
its typing, its bound constants, its instances and its growth. the interface every importer's
lowering reads, the field graph, the status and the refusal stay

## InstanceResult

```mach
pub rec InstanceResult {
    status: mach.report.phase.PhaseStatus;
    set:    mach.front.sema.instance.Set;
    values: mach.front.comptime.deep.Store;
    diags:  mach.base.diagnostic.DiagnosticStore;
}
```

one instance typed on its own, which every user of it reads

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `mach.report.phase.PhaseStatus` | whether its body type checks |
| `set` | `mach.front.sema.instance.Set` | the instance at its head (id 0), its typing in the head's frames, and after it every instance its body asks for, with where each is asked |
| `values` | `mach.front.comptime.deep.Store` | the constants its body builds, which answer to comptime_deep.INSTANCE_MODULE |
| `diags` | `mach.base.diagnostic.DiagnosticStore` | what typing it reported; a report the instance raises where it is asked for is placed nowhere (base_source.FILE_NIL), for the demand of each module reaching it to place |

### instance_dnit

```mach
pub fun instance_dnit(r: *InstanceResult)
```

release a typed instance; a nil one holds nothing

## TypeExport

```mach
pub rec TypeExport {
    origin:   mach.session.module.StableModuleId;
    canon:    mach.base.intern.StrId;
    ty:       mach.type.kind.TypeId;
    constant: mach.front.resolve.ExportConstant;
}
```

a constant an array, record or case literal gives is a value naming a node of the store its
module's interface publishes

## ModuleSema

```mach
pub rec ModuleSema {
    path:    mach.base.intern.StrId;
    module:  mach.session.module.ModuleId;
    stable:  mach.session.module.StableModuleId;
    exports: std.collections.vector.Vector[TypeExport];
    graph:   mach.front.sema.fields.Graph;
    index:   std.collections.map.Map[u64, u32];
    indexed: usize;
    # the module is refused, by its own `$error` or an import's: an importer is not typed
    refused: bool;
}
```

exports are append-only, and `index` maps (origin, canon) to the first export with
that key over the first `indexed` of them. `stable` names the module whose surface it is

### module_sema_init

```mach
pub fun module_sema_init(a: *std.allocator.Allocator, path: mach.base.intern.StrId, module: mach.session.module.ModuleId, stable: mach.session.module.StableModuleId) ModuleSema
```

an empty surface for one module

| Parameter | |
| :--- | :--- |
| `a` | the allocator its exports, graph and index are held in |
| `path` | the module's path |
| `module` | the module's id in this session |
| `stable` | the module's stable id |
| **returns** | the surface, released with `module_sema_dnit` |

### module_sema_dnit

```mach
pub fun module_sema_dnit(module: *ModuleSema)
```

releases the exports, graph and index of `module`

| Parameter | |
| :--- | :--- |
| `module` | the surface to release |

### module_export_for

```mach
pub fun module_export_for(module: *ModuleSema, origin: mach.session.module.StableModuleId, canon: mach.base.intern.StrId) res[opt[TypeExport], std.allocator.Error]
```

the export of `module` keyed (origin, canon), bringing the index up to the current exports

## DefinitionPhase

```mach
pub def DefinitionPhase: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `DEFINITION_PARSED` | `0` |  |
| `DEFINITION_RESOLVED` | `1` |  |
| `DEFINITION_TYPED` | `2` |  |

## Definition

```mach
pub rec Definition {
    parsed:     mach.front.resolve.ParsedDefinition;
    origin:     mach.session.module.ModuleId;
    rr:         *mach.front.resolve.ResolveResult;
    sr:         *SemaResult;
    ctx:        *mach.front.comptime.scope.ComptimeCtx;
    decorators: *mach.front.comptime.scope.ComptimeCtx;
}
```

a module's definition as far as a phase acquired it

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `*mach.front.comptime.scope.ComptimeCtx` | the scope its constants are read in, once typed |
| `decorators` | `*mach.front.comptime.scope.ComptimeCtx` | a scope that reads the load's records of its decorators, at every phase |

## DefinitionReader

```mach
pub rec DefinitionReader {
    ctx:      ptr;
    read:     fun(ptr, mach.session.module.ModuleId, DefinitionPhase) res[Definition, mach.base.fail.Fail];
    acquired: std.collections.vector.Vector[AcquiredDefinition];
}
```

reads the definitions of other modules, keeping the one acquired for each origin

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the context passed to `read` |
| `read` | `fun(ptr, mach.session.module.ModuleId, DefinitionPhase) res[Definition, mach.base.fail.Fail]` | acquires a module's definition as far as a phase |
| `acquired` | `std.collections.vector.Vector[AcquiredDefinition]` | each definition read so far, with the phase it reached |

### reader_init

```mach
pub fun reader_init(ctx: ptr, read: fun(ptr, mach.session.module.ModuleId, DefinitionPhase) res[Definition, mach.base.fail.Fail],
    a: *std.allocator.Allocator) DefinitionReader
```

a reader lives for one computation, which acquires the same current definition once per origin

### reader_dnit

```mach
pub fun reader_dnit(reader: *DefinitionReader)
```

release a reader; a nil reader holds nothing

### acquire_definition

```mach
pub fun acquire_definition(reader: *DefinitionReader, origin: mach.session.module.ModuleId, phase: DefinitionPhase) res[Definition, mach.base.fail.Fail]
```

the definition of module `origin` as far as each phase got: an importer is checked against
what a rejected module resolved and typed, since what it refused carries the error type

### acquire_symbol

```mach
pub fun acquire_symbol(reader: *DefinitionReader, requested: *mach.front.resolve.Symbol, origin: mach.session.module.ModuleId,
    phase: DefinitionPhase) res[DefinedSymbol, mach.base.fail.Fail]
```

the current definition of `requested`, which module `origin` defines as this operation numbers it

## DefinedSymbol

```mach
pub rec DefinedSymbol {
    definition: Definition;
    symbol:     mach.front.resolve.Symbol;
}
```

a symbol and the definition of the module that declares it

| Field | Type | |
| :--- | :--- | :--- |
| `definition` | `Definition` | the declaring module's definition |
| `symbol` | `mach.front.resolve.Symbol` | the symbol as that module resolved it |

## SemaDeps

```mach
pub rec SemaDeps {
    definitions: DefinitionReader;
    entries:     **ModuleSema;
    count:       u32;
    owners:      std.collections.map.Map[u32, u32];
}
```

the imported surfaces are borrowed: the driver owns each one and shares it
between every importer in a pass. `owners` holds each surface's position by the stable
index of its module, the first where two are one module's; surfaces_set fills it with
`entries`, which nothing changes after, and surfaces_dnit releases both

### surfaces_set

```mach
pub fun surfaces_set(deps: *SemaDeps, entries: **ModuleSema, count: u32, a: *std.allocator.Allocator) err[std.allocator.Error]
```

hand `deps` the surfaces `entries` holds, an array of `count` allocated from `a`

### surfaces_dnit

```mach
pub fun surfaces_dnit(deps: *SemaDeps, a: *std.allocator.Allocator)
```

release what surfaces_set handed `deps`: the array, from `a`, and the index; the surfaces are borrowed

### surface_of_owner

```mach
pub fun surface_of_owner(deps: *SemaDeps, owner: mach.session.module.StableModuleId) opt[*ModuleSema]
```

the surface of the module `owner` among those `deps` holds, none when it holds none

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `INFERRED_LEN_NONE` | `u64` | `0xFFFFFFFFFFFFFFFF` |  |
| `INFERRED_LEN_FAILED` | `u64` | `0xFFFFFFFFFFFFFFFE` |  |
| `INFERRED_LEN_NEEDS_LITERAL` | `u64` | `0xFFFFFFFFFFFFFFFD` | a `[_]T` annotation on a binding whose initializer is absent or not an array literal |

