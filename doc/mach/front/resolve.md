[mach](../README.md) / [front](README.md) / **resolve**

# mach.front.resolve

name resolution: every name a module spells bound to the symbol it means

a module resolves in a scope over its load, reading the load's constants, gate decisions and
import bindings and the public surface of each module it imports. it gives each expression,
type node and declaration its symbol, records how each ambiguous bracket and literal head reads
(`mach.front.ast.interpretation`), binds the `{name}` substitutions of asm blocks, and builds
what the module exports to its importers. it reports a name that resolves to nothing and a
symbol import nothing uses.

Source: [`src/front/resolve.mach`](../../../src/front/resolve.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast`](ast.md), [`mach.front.ast.expr`](ast/expr.md), [`mach.front.ast.id`](ast/id.md), [`mach.front.ast.interpretation`](ast/interpretation.md), [`mach.front.ast.type`](ast/type.md), [`mach.front.comptime.capability`](comptime/capability.md), [`mach.front.comptime.scope`](comptime/scope.md), [`mach.front.comptime.value`](comptime/value.md), [`mach.front.load`](load.md), [`mach.front.omitted`](omitted.md), [`mach.front.refusal`](refusal.md), [`mach.report.phase`](../report/phase.md), [`mach.session`](../session.md), [`mach.session.module`](../session/module.md), [`mach.type`](../type.md), [`mach.type.deprecation`](../type/deprecation.md).

| Type | | |
| :--- | :--- | :--- |
| [`SymbolId`](#symbolid) | `def` | a symbol's slot in the resolve context's symbol table |
| [`SymKind`](#symkind) | `def` |  |
| [`TypeFrame`](#typeframe) | `def` | how a type node reads a comptime parameter or `$each` loop variable: not at all, in a body, where each frame that binds them resolves it afresh, or in a signature, where it is refused |
| [`Symbol`](#symbol) | `rec` | one binding a module's resolution knows: declared here, imported, or built in |
| [`PublicSymbol`](#publicsymbol) | `rec` | one symbol a module exports, as an importer reads it |
| [`ModuleExports`](#moduleexports) | `rec` | what a module exports to an importer, every module it names by stable id |
| [`ExportConstant`](#exportconstant) | `rec` | a constant an importer reads, whose per-tuple gate is the published declaration's, not this record's |
| [`ParsedDefinition`](#parseddefinition) | `rec` | a parsed module as a definition reads it |
| [`ResolveDeps`](#resolvedeps) | `rec` | what a module's resolve reads of the modules it imports |
| [`ResolveResult`](#resolveresult) | `rec` | what resolving one module produced, owned by the result and released with result_dnit |
| [`AsmBind`](#asmbind) | `rec` | one `{name}` of an asm block and the local it binds |
| [`Exporters`](#exporters) | `rec` | the modules an importer's resolution reads: how many are loaded, with ids below that count, what each one exports, and the path it is imported by |

| On `SymbolId` | |
| :--- | :--- |
| [`SYMBOL_NIL`](#symbolid) | |
| [`symbol_id`](#symbol_id) | the symbol id of slot `index` |
| [`symbol_index`](#symbol_index) | the slot symbol id `id` names |
| [`symbol_same`](#symbol_same) | whether two symbol ids are the same id |
| [`symbol_is_sentinel`](#symbol_is_sentinel) | whether an id is one of the sentinels, which name no symbol |
| [`symbol_is_nil`](#symbol_is_nil) | whether `id` is SYMBOL_NIL, a name bound to nothing |
| [`symbol_is_rejected`](#symbol_is_rejected) | whether `id` marks a name resolution visited and rejected |
| [`SYMBOL_DEFERRED_TYPE`](#symbolid) | |
| [`symbol_deferred`](#symbol_deferred) | whether `sid` marks a name left for sema to report, in value or type position |
| [`SYMBOL_AMBIGUOUS`](#symbolid) | a nominal the module defines named by more than one declaration |

| On `SymKind` | |
| :--- | :--- |
| [`SYM_FUN`](#symkind), [`SYM_REC`](#symkind), [`SYM_DEF`](#symkind), [`SYM_VAL`](#symkind), [`SYM_VAR`](#symkind), [`SYM_PARAM`](#symkind), [`SYM_GENERIC`](#symkind), [`SYM_USE`](#symkind), [`SYM_IMPORTED`](#symkind) | |
| [`SYM_TEST`](#symkind) | reached only by mach-lsp |
| [`SYM_UNI`](#symkind), [`SYM_PRIM`](#symkind), [`SYM_VECTOR`](#symkind), [`SYM_TAG`](#symkind) | |

| On `TypeFrame` | |
| :--- | :--- |
| [`TYPE_FRAME_NONE`](#typeframe), [`TYPE_FRAME_BODY`](#typeframe), [`TYPE_FRAME_REFUSED`](#typeframe) | |

| On `Symbol` | |
| :--- | :--- |
| [`origin_of`](#origin_of) | the module a symbol is defined in, as the current operation numbers it |
| [`foreign_to`](#foreign_to) | whether `sym` names a binding a module other than the one `own` names declares: an import, or a `fwd` of one, which carries the kind the binding is declared with there as its own |
| [`use_target`](#use_target) | the module a `use` symbol names, as the current operation numbers it |
| [`use_stable`](#use_stable) | the stable id of the module a `use` symbol names, nil for any other symbol |
| [`asm_constant`](#asm_constant) | whether `sym` may name a compile-time constant an asm block binds: a module `val` or a comptime parameter, whose value lowering reads |
| [`symbol_is_runtime`](#symbol_is_runtime) | whether `sym` is a runtime binding: a local `val` or `var` or a parameter, none of them comptime |
| [`symbol_is_comptime_local`](#symbol_is_comptime_local) | a comptime parameter or `$each` loop variable, whose value a frame binds by name |
| [`symbol_name_kind`](#symbol_name_kind) | what a comptime evaluation reads a name resolved to `sym` as |

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
| [`param_symbol`](#param_symbol) | the symbol of parameter `slot` of function `decl`, SYMBOL_NIL when it has none |
| [`own_nominal`](#own_nominal) | the symbol of the record, union or tag of `kind` the module defines under `canon`: SYMBOL_NIL when it defines none, SYMBOL_AMBIGUOUS when more than one declaration does |
| [`own_declared`](#own_declared) | the symbol of `kind` the module declares under `canon`, the last of them: SYMBOL_NIL when it declares none, SYMBOL_AMBIGUOUS when more than one declaration does |
| [`result_exports`](#result_exports) | what a finished resolution of module `stable` exports to an importer |
| [`result_dnit`](#result_dnit) | release a resolution; a nil result holds nothing |
| [`result_copy`](#result_copy) | an owned copy of `r` from `alloc`, every entry at the id it has in `r` |
| [`asm_binds_at`](#asm_binds_at) | the locals an asm block binds, in the order its body names them |
| [`expr_name_kind`](#expr_name_kind) | what a resolved expression names, as a comptime evaluation reads it |
| [`expression`](#expression) | expression `eid` of `a` as name resolution read it |
| [`type_node`](#type_node) | the type node `tid` of `a`, whose text is `source`, as name resolution read it: a literal head it split names the type by its path's prefix and the case by the last segment |

| On `Exporters` | |
| :--- | :--- |
| [`deps_of`](#deps_of) | the exports module `m` of loader `l` reads: each loaded module it imports and every module a reached one re-exports, each once, allocated from `alloc` and released with deps_dnit |

| Functions | |
| :--- | :--- |
| [`stable_of_module`](#stable_of_module) | the stable id of module `mid`, nil for a nil module or one with no name |
| [`type_owner`](#type_owner) | the owner the types module `mid` declares in `file` carry |
| [`decorators_deprecation`](#decorators_deprecation) | the message is the string the argument evaluates to; one that is not a constant string is reported by type checking and leaves the notice bare |
| [`declaration_testing`](#declaration_testing) | whether a declaration carries `#[testing]`, which confines every reference to it to test code |
| [`unknown_catalog`](#unknown_catalog) | the internal failure naming a member its catalog lacks; the text belongs to the session interner, not a resolver candidate |

| Constants | |
| :--- | :--- |
| [`SYM_FLAG_PUB`](#constants) | the symbol is exported, so it joins the module's public surface |
| [`SYM_FLAG_COMPTIME`](#constants), [`SYM_FLAG_MODULE`](#constants) | |
| [`SYM_FLAG_STORED`](#constants) | something stores to the local or parameter, takes its address or names it in an asm block, so it has storage of its own |
| [`SYM_FLAG_REACHED`](#constants) | the address of something reached from the local or parameter by name, a field or an element, is taken |
| [`SYM_FLAG_WRITTEN`](#constants) | an assignment writes a field or element reached from the local or parameter by name, so what it holds changes in part where nothing stores to it whole |
| [`SYM_FLAG_VALUE_SIZED`](#constants) | a function whose body spells a type one of its comptime parameters sizes, so each value instance types the body afresh |

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

the symbol id of slot `index`

### symbol_index

```mach
pub fun symbol_index(id: SymbolId) u32
```

the slot symbol id `id` names

### symbol_same

```mach
pub fun symbol_same(left: SymbolId, right: SymbolId) bool
```

whether two symbol ids are the same id

### symbol_is_sentinel

```mach
pub fun symbol_is_sentinel(id: SymbolId) bool
```

whether an id is one of the sentinels, which name no symbol

### symbol_is_nil

```mach
pub fun symbol_is_nil(id: SymbolId) bool
```

whether `id` is SYMBOL_NIL, a name bound to nothing

### symbol_is_rejected

```mach
pub fun symbol_is_rejected(id: SymbolId) bool
```

whether `id` marks a name resolution visited and rejected

### symbol_deferred

```mach
pub fun symbol_deferred(sid: SymbolId) bool
```

whether `sid` marks a name left for sema to report, in value or type position

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

## TypeFrame

```mach
pub def TypeFrame: u8
```

how a type node reads a comptime parameter or `$each` loop variable: not at all, in a body,
where each frame that binds them resolves it afresh, or in a signature, where it is refused

| Name | Value | |
| :--- | :--- | :--- |
| `TYPE_FRAME_NONE` | `0` |  |
| `TYPE_FRAME_BODY` | `1` |  |
| `TYPE_FRAME_REFUSED` | `2` |  |

## Symbol

```mach
pub rec Symbol {
    kind:                SymKind;
    definition_kind:     SymKind;
    definition_file:     mach.base.source.FileId;
    generic_arity:       u32;
    has_comptime_params: bool;
    has_pack_param:      bool;
    deprecation:         mach.type.deprecation.Deprecation;
    testing:             bool;
    flags:               u8;
    name:                mach.base.intern.StrId;
    canon:               mach.base.intern.StrId;
    decl:                mach.front.ast.id.DeclId;
    # a `use` symbol's slot is the stable id of the module it names
    slot: u32;
    # the module the symbol is defined in. a result outlives the operation that numbered the
    # modules, so it names modules by stable id and a reader numbers them through origin_of
    origin: mach.session.module.StableModuleId;
}
```

one binding a module's resolution knows: declared here, imported, or built in

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `SymKind` | what the binding is here, SYM_IMPORTED for an imported symbol |
| `definition_kind` | `SymKind` | the kind it is declared with where it is defined |
| `definition_file` | `mach.base.source.FileId` | the file it is defined in, nil when none is known |
| `generic_arity` | `u32` | how many type parameters its declaration takes |
| `has_comptime_params` | `bool` | whether its function declares a comptime parameter |
| `has_pack_param` | `bool` | whether its function declares a variadic pack parameter |
| `deprecation` | `mach.type.deprecation.Deprecation` | the `#[deprecated]` notice on its declaration or on the import binding it |
| `testing` | `bool` | whether its declaration or the import binding it carries `#[testing]` |
| `flags` | `u8` | SYM_FLAG_* bits |
| `name` | `mach.base.intern.StrId` | the name it is bound under here |
| `canon` | `mach.base.intern.StrId` | the name it is declared under where it is defined |
| `decl` | `mach.front.ast.id.DeclId` | its declaration in this module's syntax, nil for one declared elsewhere |

### origin_of

```mach
pub fun origin_of(s: *mach.session.Session, sym: *Symbol) mach.session.module.ModuleId
```

the module a symbol is defined in, as the current operation numbers it

### foreign_to

```mach
pub fun foreign_to(sym: *Symbol, own: mach.session.module.StableModuleId) bool
```

whether `sym` names a binding a module other than the one `own` names declares: an import, or a
`fwd` of one, which carries the kind the binding is declared with there as its own

### use_target

```mach
pub fun use_target(s: *mach.session.Session, sym: *Symbol) mach.session.module.ModuleId
```

the module a `use` symbol names, as the current operation numbers it

### use_stable

```mach
pub fun use_stable(sym: *Symbol) mach.session.module.StableModuleId
```

the stable id of the module a `use` symbol names, nil for any other symbol

### asm_constant

```mach
pub fun asm_constant(sym: *Symbol) bool
```

whether `sym` may name a compile-time constant an asm block binds: a module `val` or a comptime parameter, whose
value lowering reads

### symbol_is_runtime

```mach
pub fun symbol_is_runtime(sym: *Symbol) bool
```

whether `sym` is a runtime binding: a local `val` or `var` or a parameter, none of them comptime

### symbol_is_comptime_local

```mach
pub fun symbol_is_comptime_local(sym: *Symbol) bool
```

a comptime parameter or `$each` loop variable, whose value a frame binds by name

### symbol_name_kind

```mach
pub fun symbol_name_kind(sym: *Symbol) mach.front.comptime.capability.NameKind
```

what a comptime evaluation reads a name resolved to `sym` as

## PublicSymbol

```mach
pub rec PublicSymbol {
    kind:                SymKind;
    name:                mach.base.intern.StrId;
    canon:               mach.base.intern.StrId;
    file:                mach.base.source.FileId;
    generic_arity:       u32;
    has_comptime_params: bool;
    has_pack_param:      bool;
    deprecation:         mach.type.deprecation.Deprecation;
    testing:             bool;
    # a `use` symbol's slot is the stable id of the module it names
    slot:   u32;
    origin: mach.session.module.StableModuleId;
}
```

one symbol a module exports, as an importer reads it

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `SymKind` | the kind it is declared with where it is defined |
| `name` | `mach.base.intern.StrId` | the name it is exported under |
| `canon` | `mach.base.intern.StrId` | the name it is declared under where it is defined |
| `file` | `mach.base.source.FileId` | the file it is defined in |
| `generic_arity` | `u32` | how many type parameters its declaration takes |
| `has_comptime_params` | `bool` | whether its function declares a comptime parameter |
| `has_pack_param` | `bool` | whether its function declares a variadic pack parameter |
| `deprecation` | `mach.type.deprecation.Deprecation` | the `#[deprecated]` notice it carries |
| `testing` | `bool` | whether it carries `#[testing]` |
| `origin` | `mach.session.module.StableModuleId` | the module it is defined in |

## ModuleExports

```mach
pub rec ModuleExports {
    path:    mach.base.intern.StrId;
    module:  mach.session.module.StableModuleId;
    symbols: *PublicSymbol;
    count:   u32;
    # `(name << 32) | index` of each symbol, ascending, which find_export searches
    by_name: *u64;
    # the module is refused, by its own `$error` or an import's: an importer is not analysed
    refused: bool;
    # the names its module-scope `$if` branches declare that the build does not select
    omitted:       *mach.front.omitted.Omitted;
    omitted_count: u32;
}
```

what a module exports to an importer, every module it names by stable id

### exports_copy

```mach
pub fun exports_copy(alloc: *std.allocator.Allocator, from: *ModuleExports, path: mach.base.intern.StrId, stable: mach.session.module.StableModuleId) res[ModuleExports, mach.base.fail.Fail]
```

an importer's own copy of `from`, under the importer's path for it

| Parameter | |
| :--- | :--- |
| `alloc` | owns the copy, released with exports_dnit |

### exports_dnit

```mach
pub fun exports_dnit(alloc: *std.allocator.Allocator, mx: *ModuleExports)
```

release what an exports record owns

## ExportConstant

```mach
pub rec ExportConstant {
    name:    mach.base.intern.StrId;
    present: bool;
    value:   mach.front.comptime.value.CTValue;
}
```

a constant an importer reads, whose per-tuple gate is the published declaration's, not this record's

## ParsedDefinition

```mach
pub rec ParsedDefinition {
    a:           *mach.front.ast.Ast;
    status:      mach.report.phase.PhaseStatus;
    owner:       mach.type.TypeOwner;
    incarnation: u64;
}
```

a parsed module as a definition reads it

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*mach.front.ast.Ast` | its syntax |
| `status` | `mach.report.phase.PhaseStatus` | how its parse ended |
| `owner` | `mach.type.TypeOwner` | the module and file that own the types it declares |
| `incarnation` | `u64` | the incarnation of the syntax, as `a` records it |

## ResolveDeps

```mach
pub rec ResolveDeps {
    entries:       *ModuleExports;
    count:         u32;
    bindings:      *mach.front.load.Binding;
    binding_count: u32;
}
```

what a module's resolve reads of the modules it imports

| Field | Type | |
| :--- | :--- | :--- |
| `entries` | `*ModuleExports` | the public surface of each module it may name |
| `bindings` | `*mach.front.load.Binding` | the load's record of each of its import declarations, which resolve binds from rather than reading their paths; a declaration with none is bound by its path |
| `binding_count` | `u32` | how many records `bindings` holds |

### deps_dnit

```mach
pub fun deps_dnit(alloc: *std.allocator.Allocator, deps: *ResolveDeps)
```

release what deps_of allocated; a list it left empty owns nothing

### resolve

```mach
pub fun resolve(
    s: *mach.session.Session,
    a: *mach.front.ast.Ast,
    deps: *ResolveDeps,
    load: *mach.front.comptime.scope.ComptimeCtx,
    own_module: mach.session.module.ModuleId,
    own_stable: mach.session.module.StableModuleId,
    syntax_clean: bool,
    diags: *mach.base.diagnostic.DiagnosticStore) res[ResolveResult, mach.base.fail.Fail]
```

resolve a module's names in a scope over the load's, `load`, whose constants and gate decisions
resolve reads and never writes; what resolve binds is its result's

## ResolveResult

```mach
pub rec ResolveResult {
    status:        mach.report.phase.PhaseStatus;
    alloc:         *std.allocator.Allocator;
    symbols:       *Symbol;
    symbol_count:  u32;
    pub_count:     u32;
    expr_resolved: *SymbolId;
    type_resolved: *SymbolId;
    decl_symbol:   *SymbolId;
    expr_count:    u32;
    type_count:    u32;
    decl_count:    u32;

    param_index: std.collections.map.Map[u64, SymbolId];
    # the record, union and tag symbols the module defines, by nominal_key (see own_nominal)
    nominals: std.collections.map.Map[u64, SymbolId];
    # every symbol the module declares, by nominal_key (see own_declared)
    declared: std.collections.map.Map[u64, SymbolId];

    public_constants:      *ExportConstant;
    public_constant_count: u32;

    # the constants resolve bound and the gates it decided, over the load's
    constants: mach.front.comptime.scope.Bindings;

    expr_choices:       *mach.front.ast.interpretation.Choice;
    expr_choices_count: usize;
    # how each literal head's path reads, `type_count` of them: as written, or as a type and the
    # tag case its last segment selects
    type_heads: *mach.front.ast.interpretation.Head;
    # how each type node reads a comptime local, `type_count` of them
    type_frames: *TypeFrame;

    # the local each `{name}` of an asm block binds
    asm_binds: std.collections.vector.Vector[AsmBind];
    # the names the module's module-scope `$if` branches declare that the build does not select,
    # in declaration order, which a failed lookup of one reports
    omitted: std.collections.vector.Vector[mach.front.omitted.Omitted];

    # why the module is refused, if it is: then nothing else of it was resolved
    refusal: mach.front.refusal.Refusal;
    # the module's syntax parsed without an error, so its module-scope `$error`s are evaluated
    syntax_clean: bool;
}
```

what resolving one module produced, owned by the result and released with result_dnit

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `mach.report.phase.PhaseStatus` | whether resolution accepted the module, rejected it or failed internally |
| `alloc` | `*std.allocator.Allocator` | owns every table here |
| `symbols` | `*Symbol` | every symbol, the `pub_count` exported ones first, `symbol_count` in all |
| `expr_resolved` | `*SymbolId` | the symbol each expression names, `expr_count` of them, nil or a sentinel for none |
| `type_resolved` | `*SymbolId` | the symbol each type node names, `type_count` of them |
| `decl_symbol` | `*SymbolId` | the symbol each declaration binds, `decl_count` of them |
| `param_index` | `std.collections.map.Map[u64, SymbolId]` | each parameter's symbol, by declaration index in the high 32 bits and slot in the low |
| `public_constants` | `*ExportConstant` | each exported module `val` and its value when it has one, `public_constant_count` of them |
| `expr_choices` | `*mach.front.ast.interpretation.Choice` | how each expression reads, `expr_choices_count` of them |

### param_symbol

```mach
pub fun param_symbol(r: *ResolveResult, decl: mach.front.ast.id.DeclId, slot: u32) SymbolId
```

the symbol of parameter `slot` of function `decl`, SYMBOL_NIL when it has none

### own_nominal

```mach
pub fun own_nominal(r: *ResolveResult, kind: SymKind, canon: mach.base.intern.StrId) SymbolId
```

the symbol of the record, union or tag of `kind` the module defines under `canon`: SYMBOL_NIL
when it defines none, SYMBOL_AMBIGUOUS when more than one declaration does

### own_declared

```mach
pub fun own_declared(r: *ResolveResult, kind: SymKind, canon: mach.base.intern.StrId) SymbolId
```

the symbol of `kind` the module declares under `canon`, the last of them: SYMBOL_NIL when it
declares none, SYMBOL_AMBIGUOUS when more than one declaration does

### result_exports

```mach
pub fun result_exports(alloc: *std.allocator.Allocator, rr: *ResolveResult, path: mach.base.intern.StrId,
    stable: mach.session.module.StableModuleId) res[ModuleExports, mach.base.fail.Fail]
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
pub fun result_copy(r: *ResolveResult, alloc: *std.allocator.Allocator) res[ResolveResult, mach.base.fail.Fail]
```

an owned copy of `r` from `alloc`, every entry at the id it has in `r`

### asm_binds_at

```mach
pub fun asm_binds_at(r: *ResolveResult, sid: mach.front.ast.id.StmtId, out: *std.collections.vector.Vector[AsmBind]) err[mach.base.fail.Fail]
```

the locals an asm block binds, in the order its body names them

### expr_name_kind

```mach
pub fun expr_name_kind(rr: *ResolveResult, eid: mach.front.ast.id.ExprId) mach.front.comptime.capability.NameKind
```

what a resolved expression names, as a comptime evaluation reads it

### expression

```mach
pub fun expression(rr: *ResolveResult, a: *mach.front.ast.Ast, eid: mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.base.fail.Fail]
```

expression `eid` of `a` as name resolution read it

| Parameter | |
| :--- | :--- |
| `rr` | the module's resolution |
| `a` | the module's syntax |
| `eid` | the expression |
| **returns** | the expression, or a failure for a nil resolution, choices that do not match the syntax, or an id outside it |

### type_node

```mach
pub fun type_node(rr: *ResolveResult, a: *mach.front.ast.Ast, source: str, tid: mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.base.fail.Fail]
```

the type node `tid` of `a`, whose text is `source`, as name resolution read it: a literal head
it split names the type by its path's prefix and the case by the last segment

## AsmBind

```mach
pub rec AsmBind {
    stmt:   mach.front.ast.id.StmtId;
    name:   mach.base.intern.StrId;
    symbol: SymbolId;
}
```

one `{name}` of an asm block and the local it binds

## Exporters

```mach
pub rec Exporters {
    ctx:     ptr;
    count:   u32;
    exports: fun(ptr, mach.session.module.ModuleId, *std.allocator.Allocator) res[ModuleExports, mach.base.fail.Fail];
    path:    fun(ptr, mach.session.module.ModuleId) mach.base.intern.StrId;
}
```

the modules an importer's resolution reads: how many are loaded, with ids below that count,
what each one exports, and the path it is imported by

| Field | Type | |
| :--- | :--- | :--- |
| `exports` | `fun(ptr, mach.session.module.ModuleId, *std.allocator.Allocator) res[ModuleExports, mach.base.fail.Fail]` | what a module exports to an importer, its symbols allocated from the allocator given; reading it is what makes the importer's resolution depend on the exporter's surface |

### deps_of

```mach
pub fun deps_of(alloc: *std.allocator.Allocator, s: *mach.session.Session, l: *mach.front.load.Loader, m: *mach.front.load.Module, from: *Exporters) res[ResolveDeps, mach.base.fail.Fail]
```

the exports module `m` of loader `l` reads: each loaded module it imports and every module a
reached one re-exports, each once, allocated from `alloc` and released with deps_dnit

## Functions

### stable_of_module

```mach
pub fun stable_of_module(s: *mach.session.Session, mid: mach.session.module.ModuleId) mach.session.module.StableModuleId
```

the stable id of module `mid`, nil for a nil module or one with no name

### type_owner

```mach
pub fun type_owner(s: *mach.session.Session, mid: mach.session.module.ModuleId, file: mach.base.source.FileId) mach.type.TypeOwner
```

the owner the types module `mid` declares in `file` carry

### decorators_deprecation

```mach
pub fun decorators_deprecation(c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, start: u32, len: u32) res[mach.type.deprecation.Deprecation, mach.base.fail.Fail]
```

the message is the string the argument evaluates to; one that is not a
constant string is reported by type checking and leaves the notice bare

### declaration_testing

```mach
pub fun declaration_testing(a: *mach.front.ast.Ast, did: mach.front.ast.id.DeclId) bool
```

whether a declaration carries `#[testing]`, which confines every reference to it to test code

### unknown_catalog

```mach
pub fun unknown_catalog(s: *mach.session.Session, catalog: str, tag: u32) mach.base.fail.Fail
```

the internal failure naming a member its catalog lacks; the text belongs to
the session interner, not a resolver candidate

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SYM_FLAG_PUB` | `u8` | `1` | the symbol is exported, so it joins the module's public surface |
| `SYM_FLAG_COMPTIME` | `u8` | `2` |  |
| `SYM_FLAG_MODULE` | `u8` | `4` |  |
| `SYM_FLAG_STORED` | `u8` | `8` | something stores to the local or parameter, takes its address or names it in an asm block, so it has storage of its own |
| `SYM_FLAG_REACHED` | `u8` | `16` | the address of something reached from the local or parameter by name, a field or an element, is taken |
| `SYM_FLAG_WRITTEN` | `u8` | `32` | an assignment writes a field or element reached from the local or parameter by name, so what it holds changes in part where nothing stores to it whole |
| `SYM_FLAG_VALUE_SIZED` | `u8` | `64` | a function whose body spells a type one of its comptime parameters sizes, so each value instance types the body afresh |

