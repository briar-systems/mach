# mach.lang.fe.sema.product

what sema publishes for a module, and how one module reads another's

## rec SemaResult

```mach
pub rec SemaResult;
```

## fun result_dnit

```mach
pub fun result_dnit(r: *SemaResult);
```

## val INFERRED_LEN_NONE

```mach
pub val INFERRED_LEN_NONE: u64 = 0xFFFFFFFFFFFFFFFF
```

## val INFERRED_LEN_FAILED

```mach
pub val INFERRED_LEN_FAILED: u64 = 0xFFFFFFFFFFFFFFFE
```

## val INFERRED_LEN_NEEDS_LITERAL

```mach
pub val INFERRED_LEN_NEEDS_LITERAL: u64 = 0xFFFFFFFFFFFFFFFD
```

a `[_]T` annotation on a binding whose initializer is absent or not an array literal

## rec TypeExport

```mach
pub rec TypeExport;
```

a constant an array, record or case literal gives is a value naming a node of the store its
module's interface publishes

## rec ModuleSema

```mach
pub rec ModuleSema;
```

exports are append-only, and `index` maps (origin, canon) to the first export with
that key over the first `indexed` of them

## fun module_sema_init

```mach
pub fun module_sema_init(a: *A.Allocator, path: intern.StrId, module: session.ModuleId) ModuleSema;
```

## fun module_sema_dnit

```mach
pub fun module_sema_dnit(module: *ModuleSema);
```

## fun module_export_for

```mach
pub fun module_export_for(module: *ModuleSema, origin: session.StableModuleId, canon: intern.StrId) res[opt[TypeExport], A.Error];
```

the export of `module` keyed (origin, canon), bringing the index up to the current exports

## def DefinitionPhase

```mach
pub def DefinitionPhase: u8
```

## val DEFINITION_PARSED

```mach
pub val DEFINITION_PARSED:   DefinitionPhase = 0
```

## val DEFINITION_RESOLVED

```mach
pub val DEFINITION_RESOLVED: DefinitionPhase = 1
```

## val DEFINITION_TYPED

```mach
pub val DEFINITION_TYPED:    DefinitionPhase = 2
```

## rec Definition

```mach
pub rec Definition;
```

a module's definition as far as a phase acquired it

ctx: the scope its constants are read in, once typed
decorators: a scope that reads the load's records of its decorators, at every phase

## rec DefinitionReader

```mach
pub rec DefinitionReader;
```

## fun reader_init

```mach
pub fun reader_init(ctx: ptr, read: fun(ptr, session.ModuleId, DefinitionPhase) res[Definition, fail.Fail],
a: *A.Allocator) DefinitionReader;
```

a reader lives for one computation, which acquires the same current definition once per origin

## fun reader_dnit

```mach
pub fun reader_dnit(reader: *DefinitionReader);
```

## rec DefinedSymbol

```mach
pub rec DefinedSymbol;
```

## fun acquire_definition

```mach
pub fun acquire_definition(reader: *DefinitionReader, origin: session.ModuleId,
phase: DefinitionPhase) res[Definition, fail.Fail];
```

## fun acquire_symbol

```mach
pub fun acquire_symbol(reader: *DefinitionReader, requested: *resolve.Symbol,
phase: DefinitionPhase) res[DefinedSymbol, fail.Fail];
```

## rec SemaDeps

```mach
pub rec SemaDeps;
```

the imported surfaces are borrowed: the driver owns each one and shares it
between every importer in a pass

## rec GrowthWatch

```mach
pub rec GrowthWatch;
```

who hears of a growing cycle's expansion: the module that declares it, the templates on
it, and how many instances of it the module being typed has made so far; a nil
`expanded` hears nothing

