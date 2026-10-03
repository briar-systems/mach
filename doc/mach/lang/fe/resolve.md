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

## val TESTING_DIRECTIVE

```mach
pub val TESTING_DIRECTIVE: str = "testing"
```

## val EXTENSIONS_DIRECTIVE

```mach
pub val EXTENSIONS_DIRECTIVE: str = "extensions"
```

`#[extensions(name, ...)]` takes bare extension names, which bind to nothing

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

## fun dnit_result

```mach
pub fun dnit_result(r: *ResolveResult);
```

## fun resolve

```mach
pub fun resolve(
s: *session.Session,
a: *ast.Ast,
deps: *ResolveDeps,
ctx: *comptime.ComptimeCtx,
own_module: session.ModuleId,
diags: *diagnostic.DiagnosticStore) res[ResolveResult, fail.Fail];
```

## fun decorators_deprecation

```mach
pub fun decorators_deprecation(s: *session.Session, mid: session.ModuleId, a: *ast.Ast, source: str, start: u32, len: u32) res[deprecation.Deprecation, fail.Fail];
```

the message is the string the argument evaluates to; one that is not a
constant string is reported by type checking and leaves the notice bare

## fun declaration_testing

```mach
pub fun declaration_testing(a: *ast.Ast, source: str, did: ast_id.DeclId) bool;
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

