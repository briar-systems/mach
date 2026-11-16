[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **product**

# mach.front.sema.product

what sema publishes for a module, and how one module reads another's

<sub>source: [`src/front/sema/product.mach`](../../../../src/front/sema/product.mach) · imported by 18 modules</sub>

<sub>`fields` is [`mach.front.sema.fields`](fields.md) · `report_phase` is [`mach.report.phase`](../../report/phase.md) · `A` is `std.allocator` · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `comptime_scope` is [`mach.front.comptime.scope`](../comptime/scope.md) · `sema_instance` is [`mach.front.sema.instance`](instance.md) · `sema_growth` is [`mach.front.sema.growth`](growth.md) · `interface` is [`mach.front.sema.interface`](interface.md) · `comptime_deep` is [`mach.front.comptime.deep`](../comptime/deep.md) · `diagnostic` is [`mach.base.diagnostic`](../../base/diagnostic.md) · `session_module` is [`mach.session.module`](../../session/module.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `collections_vector` is `std.collections.vector` · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../../base/fail.md) · `AcquiredDefinition` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`SemaResult`](#semaresult) | `rec` |  |
| [`InstanceResult`](#instanceresult) | `rec` | one instance typed on its own, which every user of it reads |
| [`TypeExport`](#typeexport) | `rec` | a constant an array, record or case literal gives is a value naming a node of the store its module's interface publishes |
| [`ModuleSema`](#modulesema) | `rec` | exports are append-only, and `index` maps (origin, canon) to the first export with that key over the first `indexed` of them |
| [`DefinitionPhase`](#definitionphase) | `def` |  |
| [`Definition`](#definition) | `rec` | a module's definition as far as a phase acquired it |
| [`DefinitionReader`](#definitionreader) | `rec` |  |
| [`DefinedSymbol`](#definedsymbol) | `rec` |  |
| [`SemaDeps`](#semadeps) | `rec` | the imported surfaces are borrowed: the driver owns each one and shares it between every importer in a pass |
| [`GrowthWatch`](#growthwatch) | `rec` | who hears of a growing cycle's expansion: the module that declares it, the templates on it, and how many instances of it the module being typed has made so far; a nil `expanded` hears nothing |

| On `SemaResult` | |
| :--- | :--- |
| [`result_dnit`](#result_dnit) | release a semantic result; a nil result holds nothing |
| [`result_shed`](#result_shed) | give back what only the module's own lowering reads of a semantic result, once it is lowered: its typing, its bound constants, its instances and its growth |

| On `InstanceResult` | |
| :--- | :--- |
| [`instance_dnit`](#instance_dnit) | release a typed instance; a nil one holds nothing |

| On `ModuleSema` | |
| :--- | :--- |
| [`module_sema_init`](#module_sema_init) · [`module_sema_dnit`](#module_sema_dnit) | |
| [`module_export_for`](#module_export_for) | the export of `module` keyed (origin, canon), bringing the index up to the current exports |

| On `DefinitionPhase` | |
| :--- | :--- |
| [`DEFINITION_PARSED`](#definitionphase) · [`DEFINITION_RESOLVED`](#definitionphase) · [`DEFINITION_TYPED`](#definitionphase) | |

| On `DefinitionReader` | |
| :--- | :--- |
| [`reader_init`](#reader_init) | a reader lives for one computation, which acquires the same current definition once per origin |
| [`reader_dnit`](#reader_dnit) | release a reader; a nil reader holds nothing |
| [`acquire_definition`](#acquire_definition) | the definition of module `origin` as far as each phase got: an importer is checked against what a rejected module resolved and typed, since what it refused carries the error type |
| [`acquire_symbol`](#acquire_symbol) | the current definition of `requested`, which module `origin` defines as this operation numbers it |

| Constants | |
| :--- | :--- |
| [`INFERRED_LEN_NONE`](#constants) · [`INFERRED_LEN_FAILED`](#constants) | |
| [`INFERRED_LEN_NEEDS_LITERAL`](#constants) | a `[_]T` annotation on a binding whose initializer is absent or not an array literal |

---

## SemaResult

```mach
pub rec SemaResult {
    field_graph:      fields.Graph;
    status:           report_phase.PhaseStatus;
    alloc:            *A.Allocator;
    expr_type:        *type_kind.TypeId;
    type_resolved_ty: *type_kind.TypeId;
    decl_type:        *type_kind.TypeId;
    expr_count:       u32;
    type_count:       u32;
    decl_count:       u32;
    embed_paths:      *intern.StrId;
    embed_path_count: u32;
    embed_path_cap:   u32;
    # the constants sema bound and the gates it decided, over the load's and resolve's
    constants: comptime_scope.Bindings;
    # every instance the module's own code asks for, with where and from what each is asked,
    # and the calls and gates of its own typing (frame 0); each instance is typed by itself
    # (InstanceResult), not here
    instances: sema_instance.Set;
    # whether the module's annotations reach a secret, which an instance of its templates
    # starts its own answer from
    has_secret: bool;
    # the growing cycles among the module's templates
    growth: sema_growth.Analysis;
    # what the lowering of every importer reads of the module
    interface: interface.Interface;
}
```

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
lowering reads, the field graph and the status stay

## InstanceResult

```mach
pub rec InstanceResult {
    status: report_phase.PhaseStatus;
    set:    sema_instance.Set;
    values: comptime_deep.Store;
    diags:  diagnostic.DiagnosticStore;
}
```

one instance typed on its own, which every user of it reads

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `report_phase.PhaseStatus` | whether its body type checks |
| `set` | `sema_instance.Set` | the instance at its head (id 0), its typing in the head's frames, and after it every instance its body asks for, with where each is asked |
| `values` | `comptime_deep.Store` | the constants its body builds, which answer to comptime_deep.INSTANCE_MODULE |
| `diags` | `diagnostic.DiagnosticStore` | what typing it reported; a report the instance raises where it is asked for is placed nowhere (base_source.FILE_NIL), for the build's demand to place at the site it chose |

### instance_dnit

```mach
pub fun instance_dnit(r: *InstanceResult)
```

release a typed instance; a nil one holds nothing

## TypeExport

```mach
pub rec TypeExport {
    origin:   session_module.StableModuleId;
    canon:    intern.StrId;
    ty:       type_kind.TypeId;
    constant: resolve.ExportConstant;
}
```

a constant an array, record or case literal gives is a value naming a node of the store its
module's interface publishes

## ModuleSema

```mach
pub rec ModuleSema {
    path:    intern.StrId;
    module:  session_module.ModuleId;
    exports: collections_vector.Vector[TypeExport];
    graph:   fields.Graph;
    index:   map.Map[u64, u32];
    indexed: usize;
}
```

exports are append-only, and `index` maps (origin, canon) to the first export with
that key over the first `indexed` of them

### module_sema_init

```mach
pub fun module_sema_init(a: *A.Allocator, path: intern.StrId, module: session_module.ModuleId) ModuleSema
```

### module_sema_dnit

```mach
pub fun module_sema_dnit(module: *ModuleSema)
```

### module_export_for

```mach
pub fun module_export_for(module: *ModuleSema, origin: session_module.StableModuleId, canon: intern.StrId) res[opt[TypeExport], A.Error]
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
    parsed:     resolve.ParsedDefinition;
    origin:     session_module.ModuleId;
    rr:         *resolve.ResolveResult;
    sr:         *SemaResult;
    ctx:        *comptime_scope.ComptimeCtx;
    decorators: *comptime_scope.ComptimeCtx;
}
```

a module's definition as far as a phase acquired it

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `*comptime_scope.ComptimeCtx` | the scope its constants are read in, once typed |
| `decorators` | `*comptime_scope.ComptimeCtx` | a scope that reads the load's records of its decorators, at every phase |

## DefinitionReader

```mach
pub rec DefinitionReader {
    ctx:      ptr;
    read:     fun(ptr, session_module.ModuleId, DefinitionPhase) res[Definition, fail.Fail];
    acquired: collections_vector.Vector[AcquiredDefinition];
}
```

### reader_init

```mach
pub fun reader_init(ctx: ptr, read: fun(ptr, session_module.ModuleId, DefinitionPhase) res[Definition, fail.Fail],
    a: *A.Allocator) DefinitionReader
```

a reader lives for one computation, which acquires the same current definition once per origin

### reader_dnit

```mach
pub fun reader_dnit(reader: *DefinitionReader)
```

release a reader; a nil reader holds nothing

### acquire_definition

```mach
pub fun acquire_definition(reader: *DefinitionReader, origin: session_module.ModuleId, phase: DefinitionPhase) res[Definition, fail.Fail]
```

the definition of module `origin` as far as each phase got: an importer is checked against
what a rejected module resolved and typed, since what it refused carries the error type

### acquire_symbol

```mach
pub fun acquire_symbol(reader: *DefinitionReader, requested: *resolve.Symbol, origin: session_module.ModuleId,
    phase: DefinitionPhase) res[DefinedSymbol, fail.Fail]
```

the current definition of `requested`, which module `origin` defines as this operation numbers it

## DefinedSymbol

```mach
pub rec DefinedSymbol {
    definition: Definition;
    symbol:     resolve.Symbol;
}
```

## SemaDeps

```mach
pub rec SemaDeps {
    definitions: DefinitionReader;
    entries:     **ModuleSema;
    count:       u32;
    watch:       GrowthWatch;
}
```

the imported surfaces are borrowed: the driver owns each one and shares it
between every importer in a pass

## GrowthWatch

```mach
pub rec GrowthWatch {
    ctx:      ptr;
    expanded: fun(ptr, intern.StrId, intern.StrId, u32);
}
```

who hears of a growing cycle's expansion: the module that declares it, the templates on
it, and how many instances of it the module being typed has made so far; a nil
`expanded` hears nothing

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `INFERRED_LEN_NONE` | `u64` | `0xFFFFFFFFFFFFFFFF` |  |
| `INFERRED_LEN_FAILED` | `u64` | `0xFFFFFFFFFFFFFFFE` |  |
| `INFERRED_LEN_NEEDS_LITERAL` | `u64` | `0xFFFFFFFFFFFFFFFD` | a `[_]T` annotation on a binding whose initializer is absent or not an array literal |

