# mach.lang.fe.sema.context

the state of one sema walk: the module typing it shares, the module it reads, the
instance it types and its own walk state, with the reads and reports every check makes

## fun imported_definition

```mach
pub fun imported_definition(sc: *SemaContext, sym: *resolve.Symbol,
phase: sema_product.DefinitionPhase) res[sema_product.DefinedSymbol, fail.Fail];
```

## fun symbol_definition

```mach
pub fun symbol_definition(sc: *SemaContext, sym: *resolve.Symbol,
phase: sema_product.DefinitionPhase) res[sema_product.DefinedSymbol, fail.Fail];
```

## fwd type.FieldEntry

```mach
fwd type.FieldEntry
```

forwards [`mach.lang.type.field.Entry`](../../type/field.md#rec-entry)

## fwd type.FieldTable

```mach
fwd type.FieldTable
```

forwards [`mach.lang.type.field.Table`](../../type/field.md#rec-table)

## val INST_NONE

```mach
pub val INST_NONE: u32 = sema_instance.NONE
```

## rec InstWorklist

```mach
pub rec InstWorklist;
```

the instances a module's typing asks for, typed in the order they were first asked for;
the set is the module's product, published with its result

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

## rec Offer

```mach
pub rec Offer;
```

a fix an error offers: `close` at `tail`, and with `open` set, `open` at
`head` too, as one fix labelled `label`

## rec Typing

```mach
pub rec Typing;
```

the typing of one module that every walk of it shares: the instances it asks for, its
growing cycles, and the reports each instance would otherwise repeat

collect_insts: whether the walk records the instances it asks for
subst_fn: substitutes an instance's type arguments into a type
annotation_check_fn: the per-instance checks over one type annotation, run as each is resolved

## rec ModuleView

```mach
pub rec ModuleView;
```

the module a walk reads and the typing tables it writes: the module being typed, or the
declaring module of an instance or of an imported `$each` sequence

## rec Instance

```mach
pub rec Instance;
```

the instance a walk types, or none

depth: how deep the chain of instances that asked for this one runs
parent: the instance being typed, INST_NONE outside one
frame: the typing frame the walk records into (see sema_instance.Frame)
requester: who asks for an instance from the frame (sema_instance.FROM_* or an instance id)
pack_types: the element types of the pack the instance binds, `pack_len` of them; nil
            outside a pack instance
quiet: a value or non-generic pack instance retypes what its template already
            checked, so it reports only what the template could not decide: its comptime
            arguments, and the bodies of an `$each` over its pack
site: where the instance was asked for, while `site_set`
subst: the type arguments that replace the parameters of `subst_owner`, position
            for position; a parameter of any other declaration is left alone

## rec Walk

```mach
pub rec Walk;
```

the state of one walk over syntax, fresh at each body and scoped to the construct that
sets it

type_prepass: resolving every type node up front, before any arm is selected
deferred_hits: how many deferred names type resolution has met, so a consumer can tell a
               type that failed on one from a type that failed on its own
offer: a fix the next error raised offers, set by a check that knows one and
               cleared by it once the check is done; nil offers none
write_place: the place under a store or an address-of, whose root is written or
               addressed rather than read
in_test: inside a `test` body, where a `ret` value is a status in 0..255

## rec SemaContext

```mach
pub rec SemaContext;
```

## fun init

```mach
pub fun init(s: *session.Session, diags: *diagnostic.DiagnosticStore, typing: Typing, view: ModuleView,
instance: Instance) SemaContext;
```

the one way a walk is made: over `view`, typing `instance`, sharing `typing`, with a fresh walk

## fun walk_init

```mach
pub fun walk_init() Walk;
```

## fun walk_open

```mach
pub fun walk_open(sc: *SemaContext) Walk;
```

a construct that sets walk state opens the walk and closes it on the way out, which
undoes everything the construct set; the count of deferred names met carries out

## fun walk_close

```mach
pub fun walk_close(sc: *SemaContext, outer: Walk);
```

## fun instance_none

```mach
pub fun instance_none() Instance;
```

outside any instance: typing the template, recording into its frame

## fun report_deferred_name

```mach
pub fun report_deferred_name(sc: *SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, prefix: str);
```

a name resolve left for the arm sema selects, reported once at the name itself
whichever instantiation walks it, and never for an arm sema discards

## fun report_deferred_type

```mach
pub fun report_deferred_type(sc: *SemaContext, ast_tid: ast_id.TypeId);
```

## fun resolved_type_of

```mach
pub fun resolved_type_of(sc: *SemaContext, ast_tid: ast_id.TypeId) type.TypeId;
```

## fun expression

```mach
pub fun expression(sc: *SemaContext, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun comptime_expression

```mach
pub fun comptime_expression(sc: *SemaContext, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime.EvalFail];
```

## fun apply_subst

```mach
pub fun apply_subst(sc: *SemaContext, tid: type.TypeId) type.TypeId;
```

## fun awaits_instance

```mach
pub fun awaits_instance(sc: *SemaContext, tid: type.TypeId) bool;
```

true while a type still names a type parameter: no operand predicate has an answer
for it, and every question about it waits for an instantiation

## fun record_instance

```mach
pub fun record_instance(sc: *SemaContext, item: sema_instance.Instance, eid: ast_id.ExprId) err[fail.Fail];
```

an instance the walk asks for at `eid`: added to the module's set when new, and noted as
what `eid` names in the walk's frame. an instance whose type arguments or pack still
mention a type parameter is the template under other names: nothing in its body is
settled until the caller is itself instantiated, and that caller's walk asks for the
closed instance. `item` carries the kind, the declaration, its arguments, name,
signature and site; the rest is the set's

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

## fun expr_type_set

```mach
pub fun expr_type_set(sc: *SemaContext, eid: ast_id.ExprId, ty: type.TypeId);
```

the type the walk gives expression `eid`, recorded in its frame when the frame is not
the module's own typing

## fun decl_type_set

```mach
pub fun decl_type_set(sc: *SemaContext, did: ast_id.DeclId, ty: type.TypeId);
```

## fun walk_binds_values

```mach
pub fun walk_binds_values(sc: *SemaContext) bool;
```

the walk is typing a value instance, so the comptime parameters of the function it
instantiates are bound

## fun walk_binds_pack

```mach
pub fun walk_binds_pack(sc: *SemaContext) bool;
```

the walk is typing a pack instance, so the pack's element types are bound

## fun gate_set

```mach
pub fun gate_set(sc: *SemaContext, cond: ast_id.ExprId, active: bool);
```

the verdict the walk gives a comptime gate in its frame

## fun decl_type_for

```mach
pub fun decl_type_for(sc: *SemaContext, sym: *resolve.Symbol) type.TypeId;
```

## fun symbol_for_expr

```mach
pub fun symbol_for_expr(sc: *SemaContext, eid: ast_id.ExprId) opt[*resolve.Symbol];
```

## fun comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(sc: *SemaContext, eid: ast_id.ExprId) bool;
```

## fun symbol_for_type

```mach
pub fun symbol_for_type(sc: *SemaContext, tid: ast_id.TypeId) opt[*resolve.Symbol];
```

## fun symbol_by_id

```mach
pub fun symbol_by_id(sc: *SemaContext, sid: resolve.SymbolId) opt[*resolve.Symbol];
```

## fun machine_of

```mach
pub fun machine_of(sc: *SemaContext) layout.Machine;
```

## fun report

```mach
pub fun report(sc: *SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, message: str);
```

## fun check_handle_in_array

```mach
pub fun check_handle_in_array(sc: *SemaContext, ty: type.TypeId, span: lang_source.Span);
```

## fun report_internal

```mach
pub fun report_internal(sc: *SemaContext, span: lang_source.Span, message: str);
```

## fun diag_mark

```mach
pub fun diag_mark(sc: *SemaContext) u64;
```

## fun reported_since

```mach
pub fun reported_since(sc: *SemaContext, mark: u64) bool;
```

## fun report_note

```mach
pub fun report_note(sc: *SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, message: str, note: str);
```

## fun text

```mach
pub fun text(sc: *SemaContext, fmt: str, args: ...) opt[str];
```

`fmt` formatted with `args`, owned by the caller and released with `text_free`; a refused
format fails the phase as an internal failure and gives none

## fun text_free

```mach
pub fun text_free(sc: *SemaContext, t: str);
```

## fun reportf

```mach
pub fun reportf(sc: *SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, fmt: str, args: ...);
```

a diagnostic whose message is `fmt` formatted with `args`; a refused format fails the
phase as an internal failure, here and nowhere else

## fun field_table_stage

```mach
pub fun field_table_stage(sc: *SemaContext, additional: u32) res[u32, fail.Fail];
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

## fun field_lookup

```mach
pub fun field_lookup(sc: *SemaContext, ty: type.TypeId, name: intern.StrId) opt[type.TypeId];
```

## fun type_store

```mach
pub fun type_store(sc: *SemaContext) *type.TypeInterner;
```

the type store a comptime evaluation inside this walk reads

## fun resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(sc: *SemaContext, eid: ast_id.ExprId) res[opt[u32], comptime.EvalFail];
```

## fun resolve_module_member_const

```mach
pub fun resolve_module_member_const(sc: *SemaContext, eid: ast_id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun own_nominal_declaration

```mach
pub fun own_nominal_declaration(sc: *SemaContext, nominal: *type.Type) res[ast_id.DeclId, fail.Fail];
```

## fun definition_ast

```mach
pub fun definition_ast(sc: *SemaContext, origin: session.ModuleId) *ast.Ast;
```

## fun generic_owner_of_decl

```mach
pub fun generic_owner_of_decl(s: *session.Session, a: *ast.Ast, origin: session.ModuleId,
decl_id: ast_id.DeclId) res[type.GenericOwner, fail.Fail];
```

the identity a declaration's type parameters carry (type.TypeGenericParam owner
fields): the one key both interning a parameter and substituting for it use

## fun generic_param_type_for

```mach
pub fun generic_param_type_for(sc: *SemaContext, sym: *resolve.Symbol) type.TypeId;
```

## fun embed_len_of

```mach
pub fun embed_len_of(sc: *SemaContext, tid: ast_id.TypeId) u64;
```

## fun set_embed_len

```mach
pub fun set_embed_len(sc: *SemaContext, tid: ast_id.TypeId, len: u64);
```

## fun record_embed_path

```mach
pub fun record_embed_path(sc: *SemaContext, path_id: intern.StrId) err[fail.Fail];
```

## fun query_of

```mach
pub fun query_of(sc: *SemaContext) type_query.Query;
```

a question to the type store whose internal failures land in this context

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

## fun decorator_string_arg

```mach
pub fun decorator_string_arg(sc: *SemaContext, dec: *ast_decl.Decorator, ord: u32, kind: diagnostic_kind.Kind, kind_msg: str) opt[str];
```

the string argument `ord` of a decorator evaluates to. one that is
not a constant expression is refused with `decorator.not_constant`, naming
the decorator, and a constant that is no string with `kind` and `kind_msg`

## fun require_constant_arg

```mach
pub fun require_constant_arg(sc: *SemaContext, dec: *ast_decl.Decorator, ord: u32);
```

an integer argument that is not a constant expression is refused as one;
a question only a later phase answers, a layout or an instance, is left to it

## fun decorator_arg_span

```mach
pub fun decorator_arg_span(sc: *SemaContext, eid: ast_id.ExprId, fallback: lang_source.Span) lang_source.Span;
```

## def StmtListVisitor

```mach
pub def StmtListVisitor: fun(ptr, *SemaContext, u32, u32) err[fail.Fail]
```

## fun source_text_of

```mach
pub fun source_text_of(s: *session.Session, a: *ast.Ast) res[str, fail.Fail];
```

## fun is_global_binding

```mach
pub fun is_global_binding(kind: ast_decl.DeclKind) bool;
```

## fun is_fun_or_global

```mach
pub fun is_fun_or_global(kind: ast_decl.DeclKind) bool;
```

