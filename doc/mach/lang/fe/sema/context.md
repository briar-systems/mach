# mach.lang.fe.sema.context

## rec SemaResult

```mach
pub rec SemaResult;
```

## val EMBED_LEN_NONE

```mach
pub val EMBED_LEN_NONE: u64 = 0xFFFFFFFFFFFFFFFF
```

## val EMBED_LEN_FAILED

```mach
pub val EMBED_LEN_FAILED: u64 = 0xFFFFFFFFFFFFFFFE
```

## rec TypeExport

```mach
pub rec TypeExport;
```

## rec ModuleSema

```mach
pub rec ModuleSema;
```

## fun module_sema_init

```mach
pub fun module_sema_init(a: *A.Allocator, path: intern.StrId, module: session.ModuleId) ModuleSema;
```

## fun module_sema_dnit

```mach
pub fun module_sema_dnit(module: *ModuleSema);
```

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

## rec AcquiredDefinition

```mach
pub rec AcquiredDefinition;
```

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

## fun imported_definition

```mach
pub fun imported_definition(sc: *SemaContext, sym: *resolve.Symbol,
phase: DefinitionPhase) res[DefinedSymbol, fail.Fail];
```

## fun symbol_definition

```mach
pub fun symbol_definition(sc: *SemaContext, sym: *resolve.Symbol,
phase: DefinitionPhase) res[DefinedSymbol, fail.Fail];
```

## rec SemaDeps

```mach
pub rec SemaDeps;
```

## rec InstReq

```mach
pub rec InstReq;
```

## val INST_NONE

```mach
pub val INST_NONE: u32 = 4294967295
```

## val MAX_CT_INSTANCE_DEPTH

```mach
pub val MAX_CT_INSTANCE_DEPTH: u32 = 32
```

## val MAX_CT_INSTANCES

```mach
pub val MAX_CT_INSTANCES: u32 = 8192
```

## val RECORD_OK

```mach
pub val RECORD_OK:          u8 = 0
```

## val RECORD_LIMIT_DEPTH

```mach
pub val RECORD_LIMIT_DEPTH: u8 = 1
```

## val RECORD_LIMIT_COUNT

```mach
pub val RECORD_LIMIT_COUNT: u8 = 2
```

## rec InstWorklist

```mach
pub rec InstWorklist;
```

## val SECRET_UNKNOWN

```mach
pub val SECRET_UNKNOWN: u8 = 0
```

## val SECRET_ABSENT

```mach
pub val SECRET_ABSENT:  u8 = 1
```

## val SECRET_PRESENT

```mach
pub val SECRET_PRESENT: u8 = 2
```

## val TYPE_MEMO_NONE

```mach
pub val TYPE_MEMO_NONE:     u8 = 0
```

## val TYPE_MEMO_RESOLVED

```mach
pub val TYPE_MEMO_RESOLVED: u8 = 1
```

## val LAYOUT_PENDING

```mach
pub val LAYOUT_PENDING: u8 = 0
```

## val LAYOUT_ACTIVE

```mach
pub val LAYOUT_ACTIVE:  u8 = 1
```

## val LAYOUT_READY

```mach
pub val LAYOUT_READY:   u8 = 2
```

## val LAYOUT_FAILED

```mach
pub val LAYOUT_FAILED:  u8 = 3
```

## rec UniReports

```mach
pub rec UniReports;
```

## fun uni_report_mark

```mach
pub fun uni_report_mark(ur: *UniReports, key: type.TypeId) bool;
```

## rec Guard

```mach
pub rec Guard;
```

one lexical payload guard: the tag place and the case it is known to hold; frames live on the
walker's stack and chain through `prev`

## rec SemaContext

```mach
pub rec SemaContext;
```

## fun resolved_type_of

```mach
pub fun resolved_type_of(sc: *SemaContext, ast_tid: id.TypeId) type.TypeId;
```

## fun expression

```mach
pub fun expression(sc: *SemaContext, eid: id.ExprId) res[expr.Expr, fail.Fail];
```

## fun comptime_expression

```mach
pub fun comptime_expression(sc: *SemaContext, a: *ast.Ast, eid: id.ExprId) res[expr.Expr, comptime.EvalFail];
```

## fun apply_subst

```mach
pub fun apply_subst(sc: *SemaContext, tid: type.TypeId) type.TypeId;
```

## fun arg_triggers_revalidation

```mach
pub fun arg_triggers_revalidation(sc: *SemaContext, tid: type.TypeId) bool;
```

## fun decl_body_spreads_pack_to_c_variadic

```mach
pub fun decl_body_spreads_pack_to_c_variadic(sc: *SemaContext, origin: session.ModuleId, did: id.DeclId) bool;
```

## fun type_mentions_generic_param

```mach
pub fun type_mentions_generic_param(sc: *SemaContext, tid: type.TypeId) bool;
```

## fun record_instance

```mach
pub fun record_instance(sc: *SemaContext, origin: session.ModuleId, decl: id.DeclId,
args: *type.TypeId, arg_len: u32, sig: type.TypeId,
bare: intern.StrId, site: token.Span) res[u8, fail.Fail];
```

## fun inst_worklist_new

```mach
pub fun inst_worklist_new(alloc: *A.Allocator) res[*InstWorklist, fail.Fail];
```

## fun inst_worklist_free

```mach
pub fun inst_worklist_free(wl: *InstWorklist);
```

## fun inst_worklist_dnit

```mach
pub fun inst_worklist_dnit(sc: *SemaContext);
```

## fun module_sema_for

```mach
pub fun module_sema_for(sc: *SemaContext, module: session.ModuleId) opt[*ModuleSema];
```

## fun public_type_for

```mach
pub fun public_type_for(sc: *SemaContext, sym: *resolve.Symbol) opt[TypeExport];
```

## fun decl_type_for

```mach
pub fun decl_type_for(sc: *SemaContext, sym: *resolve.Symbol) type.TypeId;
```

## fun symbol_for_expr

```mach
pub fun symbol_for_expr(sc: *SemaContext, eid: id.ExprId) opt[*resolve.Symbol];
```

## fun comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(sc: *SemaContext, eid: id.ExprId) bool;
```

## fun symbol_for_type

```mach
pub fun symbol_for_type(sc: *SemaContext, tid: id.TypeId) opt[*resolve.Symbol];
```

## fun symbol_by_id

```mach
pub fun symbol_by_id(sc: *SemaContext, sid: resolve.SymbolId) opt[*resolve.Symbol];
```

## fun ptr_width_of

```mach
pub fun ptr_width_of(sc: *SemaContext) u32;
```

## fun machine_of

```mach
pub fun machine_of(sc: *SemaContext) layout.Machine;
```

## fun report

```mach
pub fun report(sc: *SemaContext, span: token.Span, message: str);
```

## fun report_internal

```mach
pub fun report_internal(sc: *SemaContext, span: token.Span, message: str);
```

## fun type_result

```mach
pub fun type_result(sc: *SemaContext, r: res[type.TypeId, fail.Fail]) type.TypeId;
```

## fun diag_mark

```mach
pub fun diag_mark(sc: *SemaContext) u64;
```

## fun reported_since

```mach
pub fun reported_since(sc: *SemaContext, mark: u64) bool;
```

## fun report_warning

```mach
pub fun report_warning(sc: *SemaContext, span: token.Span, message: str);
```

## fun report_note

```mach
pub fun report_note(sc: *SemaContext, span: token.Span, message: str, note: str);
```

## fun report_numbered

```mach
pub fun report_numbered(sc: *SemaContext, span: token.Span, prefix: str, n: usize, suffix: str, fallback: str);
```

## fun field_table_stage

```mach
pub fun field_table_stage(sc: *SemaContext, additional: u32) res[u32, fail.Fail];
```

## fun field_table_stage_push

```mach
pub fun field_table_stage_push(sc: *SemaContext, name: intern.StrId, ty: type.TypeId) err[fail.Fail];
```

## fun field_table_stage_push_entry

```mach
pub fun field_table_stage_push_entry(sc: *SemaContext, entry: type.FieldEntry) err[fail.Fail];
```

## fun field_table_publish

```mach
pub fun field_table_publish(sc: *SemaContext, ty: type.TypeId, fields_start: u32, fields_len: u32) err[fail.Fail];
```

## fun field_table_for

```mach
pub fun field_table_for(sc: *SemaContext, ty: type.TypeId) opt[*FieldTable];
```

## fun field_epoch

```mach
pub fun field_epoch(sc: *SemaContext) u32;
```

## fun field_lookup

```mach
pub fun field_lookup(sc: *SemaContext, ty: type.TypeId, name: intern.StrId) opt[type.TypeId];
```

## fun resolve_field_member

```mach
pub fun resolve_field_member(sc: *SemaContext, owner: u32, index: u32, pick: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun field_type_by_name

```mach
pub fun field_type_by_name(types: *type.TypeInterner, owner_ty: type.TypeId, name: intern.StrId) opt[comptime.CTValue];
```

the declared type of a field or case payload named on an owner; none for an unknown name or a payloadless case

## fun resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(sc: *SemaContext, eid: id.ExprId) res[opt[u32], comptime.EvalFail];
```

## fun resolve_module_member_const

```mach
pub fun resolve_module_member_const(sc: *SemaContext, eid: id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun own_nominal_declaration

```mach
pub fun own_nominal_declaration(sc: *SemaContext, nominal: *type.Type) res[id.DeclId, fail.Fail];
```

## fun definition_ast

```mach
pub fun definition_ast(sc: *SemaContext, origin: session.ModuleId) *ast.Ast;
```

## fun generic_param_type_for

```mach
pub fun generic_param_type_for(sc: *SemaContext, sym: *resolve.Symbol) type.TypeId;
```

## fun embed_len_of

```mach
pub fun embed_len_of(sc: *SemaContext, tid: id.TypeId) u64;
```

## fun set_embed_len

```mach
pub fun set_embed_len(sc: *SemaContext, tid: id.TypeId, len: u64);
```

## fun record_embed_path

```mach
pub fun record_embed_path(sc: *SemaContext, path_id: intern.StrId) err[fail.Fail];
```

## fun recover

```mach
pub fun recover[T](sc: *SemaContext, r: res[T, fail.Fail], recovery: T) T;
```

## fun record_result

```mach
pub fun record_result(sc: *SemaContext, r: err[fail.Fail]);
```

## fun record_eval_result

```mach
pub fun record_eval_result(sc: *SemaContext, r: res[comptime.CTValue, comptime.EvalFail]);
```

## def StmtListVisitor

```mach
pub def StmtListVisitor: fun(ptr, *SemaContext, u32, u32) err[fail.Fail]
```

