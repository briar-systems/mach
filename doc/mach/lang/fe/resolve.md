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

## val SYMBOL_REJECTED

```mach
pub val SYMBOL_REJECTED: SymbolId = SymbolId;
```

an identifier visited and rejected by resolution has no symbol to remap.

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

## val SYM_FLAG_PUB

```mach
pub val SYM_FLAG_PUB: u8 = 1
```

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

## rec ExportConstant

```mach
pub rec ExportConstant;
```

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

## fun param_symbol

```mach
pub fun param_symbol(r: *ResolveResult, decl: ast_id.DeclId, slot: u32) SymbolId;
```

## fun type_owner

```mach
pub fun type_owner(s: *session.Session, mid: session.ModuleId, file: lang_source.FileId) type.TypeOwner;
```

## fun remap_result

```mach
pub fun remap_result(r: *ResolveResult, s: *session.Session);
```

## rec Exporters

```mach
pub rec Exporters;
```

the modules an importer's resolution reads: how many are loaded, with ids below that count,
each one's resolution, nil when it has none, whether that resolution is current, and the
path it is imported by

## fun deps_of

```mach
pub fun deps_of(alloc: *A.Allocator, l: *fe_load.Loader, m: *fe_load.Module, from: *Exporters) res[ResolveDeps, fail.Fail];
```

the exports module `m` of loader `l` reads: each loaded module it imports and every module a
reached one re-exports, each once, allocated from `alloc` and released with deps_dnit

## fun deps_dnit

```mach
pub fun deps_dnit(alloc: *A.Allocator, deps: *ResolveDeps);
```

release what deps_of allocated

## fun result_dnit

```mach
pub fun result_dnit(r: *ResolveResult);
```

## fun resolve

```mach
pub fun resolve(
s: *session.Session,
a: *ast.Ast,
deps: *ResolveDeps,
load: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[ResolveResult, fail.Fail];
```

resolve a module's names in a scope over the load's, `load`, whose constants and gate decisions
resolve reads and never writes; what resolve binds is its result's

## fun decorators_deprecation

```mach
pub fun decorators_deprecation(c: *comptime.ComptimeCtx, a: *ast.Ast, start: u32, len: u32) res[deprecation.Deprecation, fail.Fail];
```

the message is the string the argument evaluates to; one that is not a
constant string is reported by type checking and leaves the notice bare

## fun declaration_testing

```mach
pub fun declaration_testing(a: *ast.Ast, did: ast_id.DeclId) bool;
```

whether a declaration carries `#[testing]`, which confines every reference to it to test code

## fun symbol_is_runtime

```mach
pub fun symbol_is_runtime(sym: *Symbol) bool;
```

## fun expression

```mach
pub fun expression(rr: *ResolveResult, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun unknown_catalog

```mach
pub fun unknown_catalog(s: *session.Session, catalog: str, tag: u32) fail.Fail;
```

the internal failure naming a member its catalog lacks; the text belongs to
the session interner, not a resolver candidate

