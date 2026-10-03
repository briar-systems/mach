# mach.lang.fe.sema

## fwd sema_product.SemaResult

```mach
fwd sema_product.SemaResult
```

forwards [`mach.lang.fe.sema.product.SemaResult`](sema/product.md#rec-semaresult)

## fwd sema_product.result_dnit

```mach
fwd sema_product.result_dnit
```

forwards [`mach.lang.fe.sema.product.result_dnit`](sema/product.md#fun-result_dnit)

## fwd sema_product.TypeExport

```mach
fwd sema_product.TypeExport
```

forwards [`mach.lang.fe.sema.product.TypeExport`](sema/product.md#rec-typeexport)

## fwd sema_product.ModuleSema

```mach
fwd sema_product.ModuleSema
```

forwards [`mach.lang.fe.sema.product.ModuleSema`](sema/product.md#rec-modulesema)

## fwd sema_product.module_sema_init

```mach
fwd sema_product.module_sema_init
```

forwards [`mach.lang.fe.sema.product.module_sema_init`](sema/product.md#fun-module_sema_init)

## fwd sema_product.module_sema_dnit

```mach
fwd sema_product.module_sema_dnit
```

forwards [`mach.lang.fe.sema.product.module_sema_dnit`](sema/product.md#fun-module_sema_dnit)

## fwd sema_product.DefinitionPhase

```mach
fwd sema_product.DefinitionPhase
```

forwards [`mach.lang.fe.sema.product.DefinitionPhase`](sema/product.md#def-definitionphase)

## fwd sema_product.DEFINITION_RESOLVED

```mach
fwd sema_product.DEFINITION_RESOLVED
```

forwards [`mach.lang.fe.sema.product.DEFINITION_RESOLVED`](sema/product.md#val-definition_resolved)

## fwd sema_product.DEFINITION_TYPED

```mach
fwd sema_product.DEFINITION_TYPED
```

forwards [`mach.lang.fe.sema.product.DEFINITION_TYPED`](sema/product.md#val-definition_typed)

## fwd sema_product.Definition

```mach
fwd sema_product.Definition
```

forwards [`mach.lang.fe.sema.product.Definition`](sema/product.md#rec-definition)

## fwd sema_product.DefinedSymbol

```mach
fwd sema_product.DefinedSymbol
```

forwards [`mach.lang.fe.sema.product.DefinedSymbol`](sema/product.md#rec-definedsymbol)

## fwd sema_product.DefinitionReader

```mach
fwd sema_product.DefinitionReader
```

forwards [`mach.lang.fe.sema.product.DefinitionReader`](sema/product.md#rec-definitionreader)

## fwd sema_product.reader_init

```mach
fwd sema_product.reader_init
```

forwards [`mach.lang.fe.sema.product.reader_init`](sema/product.md#fun-reader_init)

## fwd sema_product.reader_dnit

```mach
fwd sema_product.reader_dnit
```

forwards [`mach.lang.fe.sema.product.reader_dnit`](sema/product.md#fun-reader_dnit)

## fwd sema_product.acquire_definition

```mach
fwd sema_product.acquire_definition
```

forwards [`mach.lang.fe.sema.product.acquire_definition`](sema/product.md#fun-acquire_definition)

## fwd sema_product.acquire_symbol

```mach
fwd sema_product.acquire_symbol
```

forwards [`mach.lang.fe.sema.product.acquire_symbol`](sema/product.md#fun-acquire_symbol)

## fwd sema_product.SemaDeps

```mach
fwd sema_product.SemaDeps
```

forwards [`mach.lang.fe.sema.product.SemaDeps`](sema/product.md#rec-semadeps)

## fwd sema_product.GrowthWatch

```mach
fwd sema_product.GrowthWatch
```

forwards [`mach.lang.fe.sema.product.GrowthWatch`](sema/product.md#rec-growthwatch)

## fwd sema_context.generic_owner_of_decl

```mach
fwd sema_context.generic_owner_of_decl
```

forwards [`mach.lang.fe.sema.context.generic_owner_of_decl`](sema/context.md#fun-generic_owner_of_decl)

## fwd sema_context.field_type_by_name

```mach
fwd sema_context.field_type_by_name
```

forwards [`mach.lang.fe.sema.context.field_type_by_name`](sema/context.md#fun-field_type_by_name)

## fwd mach.lang.fe.sema.fields

```mach
fwd mach.lang.fe.sema.fields
```

forwards [`mach.lang.fe.sema.fields`](sema/fields.md)

## fwd mach.lang.fe.sema.instance

```mach
fwd mach.lang.fe.sema.instance
```

forwards [`mach.lang.fe.sema.instance`](sema/instance.md)

## fun sema

```mach
pub fun sema(
s: *session.Session,
a: *ast.Ast,
rr: *resolve.ResolveResult,
deps: *sema_product.SemaDeps,
load: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[sema_product.SemaResult, fail.Fail];
```

type a module in a scope over the load's, `load`, and resolve's bindings in `rr`, which sema
reads and never writes; what sema binds is its result's

