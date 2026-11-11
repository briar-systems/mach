# mach.lang.fe.resolve

## rec SymbolId

```mach
pub rec SymbolId;
```

a symbol's slot in the resolve context's symbol table

## val SYMBOL_NIL

```mach
pub val SYMBOL_NIL: SymbolId = SymbolId;
```

## fun symbol_id

```mach
pub fun symbol_id(index: u32) SymbolId;
```

## fun symbol_index

```mach
pub fun symbol_index(id: SymbolId) u32;
```

## fun symbol_same

```mach
pub fun symbol_same(left: SymbolId, right: SymbolId) bool;
```

## fun symbol_is_sentinel

```mach
pub fun symbol_is_sentinel(id: SymbolId) bool;
```

whether an id is one of the sentinels, which name no symbol

## fun symbol_is_nil

```mach
pub fun symbol_is_nil(id: SymbolId) bool;
```

## fun symbol_is_rejected

```mach
pub fun symbol_is_rejected(id: SymbolId) bool;
```

## val SYMBOL_DEFERRED_TYPE

```mach
pub val SYMBOL_DEFERRED_TYPE: SymbolId = SymbolId;
```

## fun symbol_deferred

```mach
pub fun symbol_deferred(sid: SymbolId) bool;
```

## def SymKind

```mach
pub def SymKind: u8
```

## val SYM_FUN

```mach
pub val SYM_FUN:      SymKind = 0
```

## val SYM_REC

```mach
pub val SYM_REC:      SymKind = 1
```

## val SYM_DEF

```mach
pub val SYM_DEF:      SymKind = 2
```

## val SYM_VAL

```mach
pub val SYM_VAL:      SymKind = 3
```

## val SYM_VAR

```mach
pub val SYM_VAR:      SymKind = 4
```

## val SYM_PARAM

```mach
pub val SYM_PARAM:    SymKind = 5
```

## val SYM_GENERIC

```mach
pub val SYM_GENERIC:  SymKind = 6
```

## val SYM_USE

```mach
pub val SYM_USE:      SymKind = 7
```

## val SYM_IMPORTED

```mach
pub val SYM_IMPORTED: SymKind = 8
```

## val SYM_TEST

```mach
pub val SYM_TEST:   SymKind = 9
```

reached only by mach-lsp

## val SYM_UNI

```mach
pub val SYM_UNI:    SymKind = 10
```

## val SYM_PRIM

```mach
pub val SYM_PRIM:   SymKind = 11
```

## val SYM_VECTOR

```mach
pub val SYM_VECTOR: SymKind = 12
```

## val SYM_TAG

```mach
pub val SYM_TAG:    SymKind = 13
```

## fun sym_kind_valid

```mach
pub fun sym_kind_valid(kind: SymKind) bool;
```

## val SYM_FLAG_PUB

```mach
pub val SYM_FLAG_PUB: u8 = 1
```

reached only by mach-lsp

## val SYM_FLAG_COMPTIME

```mach
pub val SYM_FLAG_COMPTIME: u8 = 2
```

## val SYM_FLAG_MODULE

```mach
pub val SYM_FLAG_MODULE: u8 = 4
```

## rec Symbol

```mach
pub rec Symbol;
```

## rec PublicSymbol

```mach
pub rec PublicSymbol;
```

## rec ModuleExports

```mach
pub rec ModuleExports;
```

what a module exports to an importer, every module it names by stable id

## rec ExportConstant

```mach
pub rec ExportConstant;
```

a constant an importer reads, whose per-tuple gate is the published declaration's, not this record's

## rec ParsedDefinition

```mach
pub rec ParsedDefinition;
```

## rec ResolveDeps

```mach
pub rec ResolveDeps;
```

what a module's resolve reads of the modules it imports

entries: the public surface of each module it may name
bindings: the load's record of each of its import declarations, which resolve binds
               from rather than reading their paths; a declaration with none is bound by
               its path
binding_count: how many records `bindings` holds

## rec ResolveResult

```mach
pub rec ResolveResult;
```

## rec AsmBind

```mach
pub rec AsmBind;
```

one `{name}` of an asm block and the local it binds

## fun param_symbol

```mach
pub fun param_symbol(r: *ResolveResult, decl: ast_id.DeclId, slot: u32) SymbolId;
```

## val SYMBOL_AMBIGUOUS

```mach
pub val SYMBOL_AMBIGUOUS: SymbolId = SymbolId;
```

a nominal the module defines named by more than one declaration

## fun own_nominal

```mach
pub fun own_nominal(r: *ResolveResult, kind: SymKind, canon: intern.StrId) SymbolId;
```

the symbol of the record, union or tag of `kind` the module defines under `canon`: SYMBOL_NIL
when it defines none, SYMBOL_AMBIGUOUS when more than one declaration does

## fun stable_of_module

```mach
pub fun stable_of_module(s: *session.Session, mid: module.ModuleId) module.StableModuleId;
```

## fun type_owner

```mach
pub fun type_owner(s: *session.Session, mid: module.ModuleId, file: lang_source.FileId) type.TypeOwner;
```

## fun origin_of

```mach
pub fun origin_of(s: *session.Session, sym: *Symbol) module.ModuleId;
```

the module a symbol is defined in, as the current operation numbers it

## fun foreign_to

```mach
pub fun foreign_to(s: *session.Session, sym: *Symbol, own: module.ModuleId) bool;
```

whether `sym` names a binding a module other than `own` declares: an import, or a `fwd` of one,
which carries the kind the binding is declared with there as its own

## fun use_target

```mach
pub fun use_target(s: *session.Session, sym: *Symbol) module.ModuleId;
```

the module a `use` symbol names, as the current operation numbers it

## fun use_stable

```mach
pub fun use_stable(sym: *Symbol) module.StableModuleId;
```

the stable id of the module a `use` symbol names, nil for any other symbol

## rec Exporters

```mach
pub rec Exporters;
```

the modules an importer's resolution reads: how many are loaded, with ids below that count,
what each one exports, and the path it is imported by

exports: what a module exports to an importer, its symbols allocated from the allocator given;
         reading it is what makes the importer's resolution depend on the exporter's surface

## fun result_exports

```mach
pub fun result_exports(alloc: *A.Allocator, rr: *ResolveResult, path: intern.StrId,
stable: module.StableModuleId) res[ModuleExports, fail.Fail];
```

what a finished resolution of module `stable` exports to an importer

alloc: owns the returned symbols, released with the ModuleExports they are in

## fun exports_copy

```mach
pub fun exports_copy(alloc: *A.Allocator, from: *ModuleExports, path: intern.StrId, stable: module.StableModuleId) res[ModuleExports, fail.Fail];
```

an importer's own copy of `from`, under the importer's path for it

alloc: owns the copy, released with exports_dnit

## fun exports_dnit

```mach
pub fun exports_dnit(alloc: *A.Allocator, mx: *ModuleExports);
```

release what an exports record owns

## fun deps_of

```mach
pub fun deps_of(alloc: *A.Allocator, s: *session.Session, l: *fe_load.Loader, m: *fe_load.Module, from: *Exporters) res[ResolveDeps, fail.Fail];
```

the exports module `m` of loader `l` reads: each loaded module it imports and every module a
reached one re-exports, each once, allocated from `alloc` and released with deps_dnit

## fun deps_dnit

```mach
pub fun deps_dnit(alloc: *A.Allocator, deps: *ResolveDeps);
```

release what deps_of allocated; a list it left empty owns nothing

## fun result_dnit

```mach
pub fun result_dnit(r: *ResolveResult);
```

release a resolution; a nil result holds nothing

## fun result_copy

```mach
pub fun result_copy(r: *ResolveResult, alloc: *A.Allocator) res[ResolveResult, fail.Fail];
```

an owned copy of `r` from `alloc`, every entry at the id it has in `r`

## fun resolve

```mach
pub fun resolve(
s: *session.Session,
a: *ast.Ast,
deps: *ResolveDeps,
load: *comptime_scope.ComptimeCtx,
own_module: module.ModuleId,
own_stable: module.StableModuleId,
diags: *diagnostic.DiagnosticStore) res[ResolveResult, fail.Fail];
```

resolve a module's names in a scope over the load's, `load`, whose constants and gate decisions
resolve reads and never writes; what resolve binds is its result's

## fun decorators_deprecation

```mach
pub fun decorators_deprecation(c: *comptime_scope.ComptimeCtx, a: *ast.Ast, start: u32, len: u32) res[deprecation.Deprecation, fail.Fail];
```

the message is the string the argument evaluates to; one that is not a
constant string is reported by type checking and leaves the notice bare

## fun declaration_testing

```mach
pub fun declaration_testing(a: *ast.Ast, did: ast_id.DeclId) bool;
```

whether a declaration carries `#[testing]`, which confines every reference to it to test code

## fun asm_binds_at

```mach
pub fun asm_binds_at(r: *ResolveResult, sid: ast_id.StmtId, out: *collections_vector.Vector[AsmBind]) err[fail.Fail];
```

the locals an asm block binds, in the order its body names them

## fun symbol_is_runtime

```mach
pub fun symbol_is_runtime(sym: *Symbol) bool;
```

## fun expr_is_runtime

```mach
pub fun expr_is_runtime(rr: *ResolveResult, eid: ast_id.ExprId) bool;
```

whether a resolved expression names a runtime binding, a local or a parameter
whose value no comptime evaluation sees; false for a name that resolved to none

## fun expression

```mach
pub fun expression(rr: *ResolveResult, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun type_node

```mach
pub fun type_node(rr: *ResolveResult, a: *ast.Ast, source: str, tid: ast_id.TypeId) res[ast_type.Type, fail.Fail];
```

the type node `tid` of `a`, whose text is `source`, as name resolution read it: a literal head
it split names the type by its path's prefix and the case by the last segment

## fun unknown_catalog

```mach
pub fun unknown_catalog(s: *session.Session, catalog: str, tag: u32) fail.Fail;
```

the internal failure naming a member its catalog lacks; the text belongs to
the session interner, not a resolver candidate

