[mach](../README.md) / [front](README.md) / **resolve**

# mach.front.resolve

name resolution: every name a module spells bound to the symbol it means

a module resolves in a scope over its load, reading the load's constants, gate decisions and
import bindings and the public surface of each module it imports. it gives each expression,
type node and declaration its symbol, records how each ambiguous bracket and literal head reads
(`mach.front.ast.interpretation`), binds the `{name}` substitutions of asm blocks, and builds
what the module exports to its importers. it reports a name that resolves to nothing and a
symbol import nothing uses.

<sub>source: [`src/front/resolve.mach`](../../../src/front/resolve.mach) · imported by 30 modules</sub>

<sub>`base_source` is [`mach.base.source`](../base/source.md) · `deprecation` is [`mach.type.deprecation`](../type/deprecation.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `ast_id` is [`mach.front.ast.id`](ast/id.md) · `module` is [`mach.session.module`](../session/module.md) · `session` is [`mach.session`](../session.md) · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `comptime_value` is [`mach.front.comptime.value`](comptime/value.md) · `ast` is [`mach.front.ast`](ast.md) · `report_phase` is [`mach.report.phase`](../report/phase.md) · `type` is [`mach.type`](../type.md) · `front_load` is [`mach.front.load`](load.md) · `comptime_scope` is [`mach.front.comptime.scope`](comptime/scope.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `map` is `std.collections.map` · `interpretation` is [`mach.front.ast.interpretation`](ast/interpretation.md) · `collections_vector` is `std.collections.vector` · `ast_expr` is [`mach.front.ast.expr`](ast/expr.md) · `ast_type` is [`mach.front.ast.type`](ast/type.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`SymbolId`](#symbolid) | `def` | a symbol's slot in the resolve context's symbol table |
| [`SymKind`](#symkind) | `def` |  |
| [`Symbol`](#symbol) | `rec` |  |
| [`PublicSymbol`](#publicsymbol) | `rec` |  |
| [`ModuleExports`](#moduleexports) | `rec` | what a module exports to an importer, every module it names by stable id |
| [`ExportConstant`](#exportconstant) | `rec` | a constant an importer reads, whose per-tuple gate is the published declaration's, not this record's |
| [`ParsedDefinition`](#parseddefinition) | `rec` |  |
| [`ResolveDeps`](#resolvedeps) | `rec` | what a module's resolve reads of the modules it imports |
| [`ResolveResult`](#resolveresult) | `rec` |  |
| [`AsmBind`](#asmbind) | `rec` | one `{name}` of an asm block and the local it binds |
| [`Exporters`](#exporters) | `rec` | the modules an importer's resolution reads: how many are loaded, with ids below that count, what each one exports, and the path it is imported by |

| On `SymbolId` | |
| :--- | :--- |
| [`SYMBOL_NIL`](#symbolid) · [`symbol_id`](#symbol_id) · [`symbol_index`](#symbol_index) · [`symbol_same`](#symbol_same) | |
| [`symbol_is_sentinel`](#symbol_is_sentinel) | whether an id is one of the sentinels, which name no symbol |
| [`symbol_is_nil`](#symbol_is_nil) · [`symbol_is_rejected`](#symbol_is_rejected) · [`SYMBOL_DEFERRED_TYPE`](#symbolid) · [`symbol_deferred`](#symbol_deferred) | |
| [`SYMBOL_AMBIGUOUS`](#symbolid) | a nominal the module defines named by more than one declaration |

| On `SymKind` | |
| :--- | :--- |
| [`SYM_FUN`](#symkind) · [`SYM_REC`](#symkind) · [`SYM_DEF`](#symkind) · [`SYM_VAL`](#symkind) · [`SYM_VAR`](#symkind) · [`SYM_PARAM`](#symkind) · [`SYM_GENERIC`](#symkind) · [`SYM_USE`](#symkind) · [`SYM_IMPORTED`](#symkind) | |
| [`SYM_TEST`](#symkind) | reached only by mach-lsp |
| [`SYM_UNI`](#symkind) · [`SYM_PRIM`](#symkind) · [`SYM_VECTOR`](#symkind) · [`SYM_TAG`](#symkind) · [`sym_kind_valid`](#sym_kind_valid) | |

| On `Symbol` | |
| :--- | :--- |
| [`origin_of`](#origin_of) | the module a symbol is defined in, as the current operation numbers it |
| [`foreign_to`](#foreign_to) | whether `sym` names a binding a module other than `own` declares: an import, or a `fwd` of one, which carries the kind the binding is declared with there as its own |
| [`use_target`](#use_target) | the module a `use` symbol names, as the current operation numbers it |
| [`use_stable`](#use_stable) | the stable id of the module a `use` symbol names, nil for any other symbol |
| [`symbol_is_runtime`](#symbol_is_runtime) | |

| On `ModuleExports` | |
| :--- | :--- |
| [`exports_copy`](#exports_copy) | an importer's own copy of `from`, under the importer's path for it |
| [`exports_dnit`](#exports_dnit) | release what an exports record owns |

| On `ResolveDeps` | |
| :--- | :--- |
| [`deps_dnit`](#deps_dnit) | release what deps_of allocated; a list it left empty owns nothing |
| [`resolve`](#resolve) | resolve a module's names in a scope over the load's, `load`, whose constants and gate decisions resolve reads and never writes; what resolve binds is its result's |

| On `ResolveResult` | |
| :--- | :--- |
| [`param_symbol`](#param_symbol) | |
| [`own_nominal`](#own_nominal) | the symbol of the record, union or tag of `kind` the module defines under `canon`: SYMBOL_NIL when it defines none, SYMBOL_AMBIGUOUS when more than one declaration does |
| [`own_declared`](#own_declared) | the symbol of `kind` the module declares under `canon`, the last of them: SYMBOL_NIL when it declares none, SYMBOL_AMBIGUOUS when more than one declaration does |
| [`result_exports`](#result_exports) | what a finished resolution of module `stable` exports to an importer |
| [`result_dnit`](#result_dnit) | release a resolution; a nil result holds nothing |
| [`result_copy`](#result_copy) | an owned copy of `r` from `alloc`, every entry at the id it has in `r` |
| [`asm_binds_at`](#asm_binds_at) | the locals an asm block binds, in the order its body names them |
| [`expr_is_runtime`](#expr_is_runtime) | whether a resolved expression names a runtime binding, a local or a parameter whose value no comptime evaluation sees; false for a name that resolved to none |
| [`expression`](#expression) | |
| [`type_node`](#type_node) | the type node `tid` of `a`, whose text is `source`, as name resolution read it: a literal head it split names the type by its path's prefix and the case by the last segment |

| On `Exporters` | |
| :--- | :--- |
| [`deps_of`](#deps_of) | the exports module `m` of loader `l` reads: each loaded module it imports and every module a reached one re-exports, each once, allocated from `alloc` and released with deps_dnit |

| Functions | |
| :--- | :--- |
| [`stable_of_module`](#stable_of_module) · [`type_owner`](#type_owner) | |
| [`decorators_deprecation`](#decorators_deprecation) | the message is the string the argument evaluates to; one that is not a constant string is reported by type checking and leaves the notice bare |
| [`declaration_testing`](#declaration_testing) | whether a declaration carries `#[testing]`, which confines every reference to it to test code |
| [`unknown_catalog`](#unknown_catalog) | the internal failure naming a member its catalog lacks; the text belongs to the session interner, not a resolver candidate |

| Constants | |
| :--- | :--- |
| [`SYM_FLAG_PUB`](#constants) | reached only by mach-lsp |
| [`SYM_FLAG_COMPTIME`](#constants) · [`SYM_FLAG_MODULE`](#constants) | |
| [`SYM_FLAG_STORED`](#constants) | something stores to the local or parameter, takes its address or names it in an asm block, so it has storage of its own |
| [`SYM_FLAG_REACHED`](#constants) | the address of something reached from the local or parameter by name, a field or an element, is taken |
| [`SYM_FLAG_WRITTEN`](#constants) | an assignment writes a field or element reached from the local or parameter by name, so what it holds changes in part where nothing stores to it whole |

---

## SymbolId

```mach
pub def SymbolId: u32
```

a symbol's slot in the resolve context's symbol table

| Name | Value | |
| :--- | :--- | :--- |
| `SYMBOL_NIL` | `0xFFFFFFFF` |  |
| `SYMBOL_DEFERRED_TYPE` | `0xFFFFFFFC` |  |
| `SYMBOL_AMBIGUOUS` | `0xFFFFFFFD` | a nominal the module defines named by more than one declaration |

### symbol_id

```mach
pub fun symbol_id(index: u32) SymbolId
```

### symbol_index

```mach
pub fun symbol_index(id: SymbolId) u32
```

### symbol_same

```mach
pub fun symbol_same(left: SymbolId, right: SymbolId) bool
```

### symbol_is_sentinel

```mach
pub fun symbol_is_sentinel(id: SymbolId) bool
```

whether an id is one of the sentinels, which name no symbol

### symbol_is_nil

```mach
pub fun symbol_is_nil(id: SymbolId) bool
```

### symbol_is_rejected

```mach
pub fun symbol_is_rejected(id: SymbolId) bool
```

### symbol_deferred

```mach
pub fun symbol_deferred(sid: SymbolId) bool
```

## SymKind

```mach
pub def SymKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `SYM_FUN` | `0` |  |
| `SYM_REC` | `1` |  |
| `SYM_DEF` | `2` |  |
| `SYM_VAL` | `3` |  |
| `SYM_VAR` | `4` |  |
| `SYM_PARAM` | `5` |  |
| `SYM_GENERIC` | `6` |  |
| `SYM_USE` | `7` |  |
| `SYM_IMPORTED` | `8` |  |
| `SYM_TEST` | `9` | reached only by mach-lsp |
| `SYM_UNI` | `10` |  |
| `SYM_PRIM` | `11` |  |
| `SYM_VECTOR` | `12` |  |
| `SYM_TAG` | `13` |  |

### sym_kind_valid

```mach
pub fun sym_kind_valid(kind: SymKind) bool
```

## Symbol

```mach
pub rec Symbol {
    kind:                SymKind;
    definition_kind:     SymKind;
    definition_file:     base_source.FileId;
    generic_arity:       u32;
    has_comptime_params: bool;
    has_pack_param:      bool;
    deprecation:         deprecation.Deprecation;
    testing:             bool;
    flags:               u8;
    name:                intern.StrId;
    canon:               intern.StrId;
    decl:                ast_id.DeclId;
    # a `use` symbol's slot is the stable id of the module it names
    slot: u32;
    # the module the symbol is defined in. a result outlives the operation that numbered the
    # modules, so it names modules by stable id and a reader numbers them through origin_of
    origin: module.StableModuleId;
}
```

### origin_of

```mach
pub fun origin_of(s: *session.Session, sym: *Symbol) module.ModuleId
```

the module a symbol is defined in, as the current operation numbers it

### foreign_to

```mach
pub fun foreign_to(s: *session.Session, sym: *Symbol, own: module.ModuleId) bool
```

whether `sym` names a binding a module other than `own` declares: an import, or a `fwd` of one,
which carries the kind the binding is declared with there as its own

### use_target

```mach
pub fun use_target(s: *session.Session, sym: *Symbol) module.ModuleId
```

the module a `use` symbol names, as the current operation numbers it

### use_stable

```mach
pub fun use_stable(sym: *Symbol) module.StableModuleId
```

the stable id of the module a `use` symbol names, nil for any other symbol

### symbol_is_runtime

```mach
pub fun symbol_is_runtime(sym: *Symbol) bool
```

## PublicSymbol

```mach
pub rec PublicSymbol {
    kind:                SymKind;
    name:                intern.StrId;
    canon:               intern.StrId;
    file:                base_source.FileId;
    generic_arity:       u32;
    has_comptime_params: bool;
    has_pack_param:      bool;
    deprecation:         deprecation.Deprecation;
    testing:             bool;
    # a `use` symbol's slot is the stable id of the module it names
    slot:   u32;
    origin: module.StableModuleId;
}
```

## ModuleExports

```mach
pub rec ModuleExports {
    path:    intern.StrId;
    module:  module.StableModuleId;
    symbols: *PublicSymbol;
    count:   u32;
    # `(name << 32) | index` of each symbol, ascending, which find_export searches
    by_name: *u64;
    # the module refused itself, and that refusal is its diagnostic, so a missing name is not one
    refused: bool;
}
```

what a module exports to an importer, every module it names by stable id

### exports_copy

```mach
pub fun exports_copy(alloc: *A.Allocator, from: *ModuleExports, path: intern.StrId, stable: module.StableModuleId) res[ModuleExports, fail.Fail]
```

an importer's own copy of `from`, under the importer's path for it

| Parameter | |
| :--- | :--- |
| `alloc` | owns the copy, released with exports_dnit |

### exports_dnit

```mach
pub fun exports_dnit(alloc: *A.Allocator, mx: *ModuleExports)
```

release what an exports record owns

## ExportConstant

```mach
pub rec ExportConstant {
    name:    intern.StrId;
    present: bool;
    value:   comptime_value.CTValue;
}
```

a constant an importer reads, whose per-tuple gate is the published declaration's, not this record's

## ParsedDefinition

```mach
pub rec ParsedDefinition {
    a:           *ast.Ast;
    status:      report_phase.PhaseStatus;
    owner:       type.TypeOwner;
    incarnation: u64;
}
```

## ResolveDeps

```mach
pub rec ResolveDeps {
    entries:       *ModuleExports;
    count:         u32;
    bindings:      *front_load.Binding;
    binding_count: u32;
}
```

what a module's resolve reads of the modules it imports

| Field | Type | |
| :--- | :--- | :--- |
| `entries` | `*ModuleExports` | the public surface of each module it may name |
| `bindings` | `*front_load.Binding` | the load's record of each of its import declarations, which resolve binds from rather than reading their paths; a declaration with none is bound by its path |
| `binding_count` | `u32` | how many records `bindings` holds |

### deps_dnit

```mach
pub fun deps_dnit(alloc: *A.Allocator, deps: *ResolveDeps)
```

release what deps_of allocated; a list it left empty owns nothing

### resolve

```mach
pub fun resolve(
    s: *session.Session,
    a: *ast.Ast,
    deps: *ResolveDeps,
    load: *comptime_scope.ComptimeCtx,
    own_module: module.ModuleId,
    own_stable: module.StableModuleId,
    diags: *diagnostic.DiagnosticStore) res[ResolveResult, fail.Fail]
```

resolve a module's names in a scope over the load's, `load`, whose constants and gate decisions
resolve reads and never writes; what resolve binds is its result's

## ResolveResult

```mach
pub rec ResolveResult {
    status:        report_phase.PhaseStatus;
    alloc:         *A.Allocator;
    symbols:       *Symbol;
    symbol_count:  u32;
    pub_count:     u32;
    expr_resolved: *SymbolId;
    type_resolved: *SymbolId;
    decl_symbol:   *SymbolId;
    expr_count:    u32;
    type_count:    u32;
    decl_count:    u32;

    param_index: map.Map[u64, SymbolId];
    # the record, union and tag symbols the module defines, by nominal_key (see own_nominal)
    nominals: map.Map[u64, SymbolId];
    # every symbol the module declares, by nominal_key (see own_declared)
    declared: map.Map[u64, SymbolId];

    public_constants:      *ExportConstant;
    public_constant_count: u32;

    # the constants resolve bound and the gates it decided, over the load's
    constants: comptime_scope.Bindings;

    expr_choices:       *interpretation.Choice;
    expr_choices_count: usize;
    # how each literal head's path reads, `type_count` of them: as written, or as a type and the
    # tag case its last segment selects
    type_heads: *interpretation.Head;

    # the local each `{name}` of an asm block binds
    asm_binds: collections_vector.Vector[AsmBind];

    # a module-scope `$error` in a decided arm refused the module, so nothing else was resolved
    refused: bool;
}
```

### param_symbol

```mach
pub fun param_symbol(r: *ResolveResult, decl: ast_id.DeclId, slot: u32) SymbolId
```

### own_nominal

```mach
pub fun own_nominal(r: *ResolveResult, kind: SymKind, canon: intern.StrId) SymbolId
```

the symbol of the record, union or tag of `kind` the module defines under `canon`: SYMBOL_NIL
when it defines none, SYMBOL_AMBIGUOUS when more than one declaration does

### own_declared

```mach
pub fun own_declared(r: *ResolveResult, kind: SymKind, canon: intern.StrId) SymbolId
```

the symbol of `kind` the module declares under `canon`, the last of them: SYMBOL_NIL when it
declares none, SYMBOL_AMBIGUOUS when more than one declaration does

### result_exports

```mach
pub fun result_exports(alloc: *A.Allocator, rr: *ResolveResult, path: intern.StrId,
    stable: module.StableModuleId) res[ModuleExports, fail.Fail]
```

what a finished resolution of module `stable` exports to an importer

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned symbols, released with the ModuleExports they are in |

### result_dnit

```mach
pub fun result_dnit(r: *ResolveResult)
```

release a resolution; a nil result holds nothing

### result_copy

```mach
pub fun result_copy(r: *ResolveResult, alloc: *A.Allocator) res[ResolveResult, fail.Fail]
```

an owned copy of `r` from `alloc`, every entry at the id it has in `r`

### asm_binds_at

```mach
pub fun asm_binds_at(r: *ResolveResult, sid: ast_id.StmtId, out: *collections_vector.Vector[AsmBind]) err[fail.Fail]
```

the locals an asm block binds, in the order its body names them

### expr_is_runtime

```mach
pub fun expr_is_runtime(rr: *ResolveResult, eid: ast_id.ExprId) bool
```

whether a resolved expression names a runtime binding, a local or a parameter
whose value no comptime evaluation sees; false for a name that resolved to none

### expression

```mach
pub fun expression(rr: *ResolveResult, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail]
```

### type_node

```mach
pub fun type_node(rr: *ResolveResult, a: *ast.Ast, source: str, tid: ast_id.TypeId) res[ast_type.Type, fail.Fail]
```

the type node `tid` of `a`, whose text is `source`, as name resolution read it: a literal head
it split names the type by its path's prefix and the case by the last segment

## AsmBind

```mach
pub rec AsmBind {
    stmt:   ast_id.StmtId;
    name:   intern.StrId;
    symbol: SymbolId;
}
```

one `{name}` of an asm block and the local it binds

## Exporters

```mach
pub rec Exporters {
    ctx:     ptr;
    count:   u32;
    exports: fun(ptr, module.ModuleId, *A.Allocator) res[ModuleExports, fail.Fail];
    path:    fun(ptr, module.ModuleId) intern.StrId;
}
```

the modules an importer's resolution reads: how many are loaded, with ids below that count,
what each one exports, and the path it is imported by

| Field | Type | |
| :--- | :--- | :--- |
| `exports` | `fun(ptr, module.ModuleId, *A.Allocator) res[ModuleExports, fail.Fail]` | what a module exports to an importer, its symbols allocated from the allocator given; reading it is what makes the importer's resolution depend on the exporter's surface |

### deps_of

```mach
pub fun deps_of(alloc: *A.Allocator, s: *session.Session, l: *front_load.Loader, m: *front_load.Module, from: *Exporters) res[ResolveDeps, fail.Fail]
```

the exports module `m` of loader `l` reads: each loaded module it imports and every module a
reached one re-exports, each once, allocated from `alloc` and released with deps_dnit

## Functions

### stable_of_module

```mach
pub fun stable_of_module(s: *session.Session, mid: module.ModuleId) module.StableModuleId
```

### type_owner

```mach
pub fun type_owner(s: *session.Session, mid: module.ModuleId, file: base_source.FileId) type.TypeOwner
```

### decorators_deprecation

```mach
pub fun decorators_deprecation(c: *comptime_scope.ComptimeCtx, a: *ast.Ast, start: u32, len: u32) res[deprecation.Deprecation, fail.Fail]
```

the message is the string the argument evaluates to; one that is not a
constant string is reported by type checking and leaves the notice bare

### declaration_testing

```mach
pub fun declaration_testing(a: *ast.Ast, did: ast_id.DeclId) bool
```

whether a declaration carries `#[testing]`, which confines every reference to it to test code

### unknown_catalog

```mach
pub fun unknown_catalog(s: *session.Session, catalog: str, tag: u32) fail.Fail
```

the internal failure naming a member its catalog lacks; the text belongs to
the session interner, not a resolver candidate

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SYM_FLAG_PUB` | `u8` | `1` | reached only by mach-lsp |
| `SYM_FLAG_COMPTIME` | `u8` | `2` |  |
| `SYM_FLAG_MODULE` | `u8` | `4` |  |
| `SYM_FLAG_STORED` | `u8` | `8` | something stores to the local or parameter, takes its address or names it in an asm block, so it has storage of its own |
| `SYM_FLAG_REACHED` | `u8` | `16` | the address of something reached from the local or parameter by name, a field or an element, is taken |
| `SYM_FLAG_WRITTEN` | `u8` | `32` | an assignment writes a field or element reached from the local or parameter by name, so what it holds changes in part where nothing stores to it whole |

