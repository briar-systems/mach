# mach.lang.me.lower.context

## rec LoopFrame

```mach
pub rec LoopFrame;
```

## rec NormalObject

```mach
pub rec NormalObject;
```

a module's test object holds its tests and `#[testing]` declarations beside its
normal object, which it never changes. every definition the normal object already
holds is declared in the test object instead, so each symbol has one definition

defined: the global symbols the normal object defines

## fun normal_defines

```mach
pub fun normal_defines(lc: *LowerContext, name: intern.StrId) bool;
```

lowering the test object, which declares `name` because the normal object defines it

## rec ModuleScope

```mach
pub rec ModuleScope;
```

## rec EachElement

```mach
pub rec EachElement;
```

the element of a constant array a `$each` loop variable names: the array's expression and the
element's index in it, where the element is stored

## rec ScopeMark

```mach
pub rec ScopeMark;
```

## rec LowerRequest

```mach
pub rec LowerRequest;
```

## rec LowerContext

```mach
pub rec LowerContext;
```

## fun module_scope

```mach
pub fun module_scope(
s: *session.Session,
own_module: session.ModuleId,
a: *ast.Ast,
fqn: intern.StrId,
sema_result: *fe_sema.SemaResult,
rr: *resolve.ResolveResult,
ctx: *comptime.ComptimeCtx) res[ModuleScope, fail.Fail];
```

## fun origin_scope

```mach
pub fun origin_scope(lc: *LowerContext, origin: session.ModuleId) res[ModuleScope, fail.Fail];
```

the scope another module's template is read in: the copy its interface publishes, with a
comptime scope over its constants that this lowering owns, so what an instance binds stays here

## fun request

```mach
pub fun request(
s: *session.Session,
tgt: *resolved.Target,
malloc: *A.Allocator,
a: *ast.Ast,
fqn: intern.StrId,
own_module: session.ModuleId,
sema_result: *fe_sema.SemaResult,
rr: *resolve.ResolveResult,
ctx: *comptime.ComptimeCtx,
normal_object: *NormalObject,
shared_artifact: bool,
debug: bool,
checks: bool,
diags: *diagnostic.DiagnosticStore,
interfaces: fe_sema.interface.Reader) res[LowerRequest, fail.Fail];
```

## fun scope_enter

```mach
pub fun scope_enter(lc: *LowerContext, next: ModuleScope) res[ScopeMark, fail.Fail];
```

## fun scope_leave

```mach
pub fun scope_leave(lc: *LowerContext, mark: ScopeMark) err[fail.Fail];
```

## fun init_context

```mach
pub fun init_context(lc: *LowerContext, req: *LowerRequest, m: *me_ir.Module) err[fail.Fail];
```

## fun dnit_context

```mach
pub fun dnit_context(lc: *LowerContext);
```

## fun each_element_of

```mach
pub fun each_element_of(lc: *LowerContext, name: intern.StrId) opt[EachElement];
```

the element the innermost `$each` loop variable `name` names

## fun interface_of

```mach
pub fun interface_of(lc: *LowerContext, origin: session.ModuleId) res[*fe_sema.interface.Interface, fail.Fail];
```

the interface of the module `origin`: the one the lowering module published, or an import's

## rec Declared

```mach
pub rec Declared;
```

a module-level declaration as the interface of its module describes it

## fun declared

```mach
pub fun declared(lc: *LowerContext, sym: *resolve.Symbol) res[Declared, fail.Fail];
```

the declaration `sym` names: one of the module in scope by its id, or another module's by
its kind and canonical name

## fun constant_store

```mach
pub fun constant_store(lc: *LowerContext, module_index: u32) res[*comptime_deep.Store, fail.Fail];
```

the store of constant values the module with stable id `module_index` publishes in its interface

## fun declared_named

```mach
pub fun declared_named(lc: *LowerContext, origin: session.ModuleId, kind: resolve.SymKind, canon: intern.StrId) res[Declared, fail.Fail];
```

the one declaration of `kind` the module `origin` defines under `canon`

## fun declared_scalar

```mach
pub fun declared_scalar(lc: *LowerContext, d: Declared) opt[comptime.CTValue];
```

the scalar a module `val` holds, none when it holds no constant or an aggregate

## fun gated_lower_member_const_message

```mach
pub fun gated_lower_member_const_message(lc: *LowerContext, name: intern.StrId, dep_fqn: intern.StrId) res[str, fail.Fail];
```

## fun report_gate

```mach
pub fun report_gate(lc: *LowerContext, k: diagnostic_kind.Kind, span: lang_source.Span, message: str) fail.Fail;
```

## fun record_eval_result

```mach
pub fun record_eval_result(lc: *LowerContext, r: res[comptime.CTValue, comptime.EvalFail]);
```

## fun expression

```mach
pub fun expression(lc: *LowerContext, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun resolve_module_member_const

```mach
pub fun resolve_module_member_const(lc: *LowerContext, eid: ast_id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(lc: *LowerContext, eid: ast_id.ExprId) bool;
```

## fun resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(lc: *LowerContext, eid: ast_id.ExprId) res[opt[u32], comptime.EvalFail];
```

## fun resolve_type_query

```mach
pub fun resolve_type_query(lc: *LowerContext, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(lc: *LowerContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun field_offset

```mach
pub fun field_offset(lc: *LowerContext, owner: u32, index: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

a field descriptor's offset, from the layout lowering gives the owning type

## fun type_store

```mach
pub fun type_store(lc: *LowerContext) *type.TypeInterner;
```

the type store a comptime evaluation during lowering reads

## fun begin_function

```mach
pub fun begin_function(lc: *LowerContext, fn_index: u32, ret_type: ir_type.IrTypeId);
```

## rec LocalBinding

```mach
pub rec LocalBinding;
```

## fun bind_local

```mach
pub fun bind_local(lc: *LowerContext, sym: resolve.SymbolId, v: value.Value, secrecy: ct.BindSecrecy) err[fail.Fail];
```

## fun local_binding

```mach
pub fun local_binding(lc: *LowerContext, sym: resolve.SymbolId) opt[LocalBinding];
```

## fun lookup_local

```mach
pub fun lookup_local(lc: *LowerContext, sym: resolve.SymbolId) opt[value.Value];
```

## fun push_loop

```mach
pub fun push_loop(lc: *LowerContext, header: me_ir.BlockId, exit: me_ir.BlockId) err[fail.Fail];
```

## fun pop_loop

```mach
pub fun pop_loop(lc: *LowerContext);
```

## fun current_loop

```mach
pub fun current_loop(lc: *LowerContext) *LoopFrame;
```

## fun push_fin

```mach
pub fun push_fin(lc: *LowerContext, body: ast_id.StmtId) err[fail.Fail];
```

## fun fin_count

```mach
pub fun fin_count(lc: *LowerContext) u32;
```

## fun fin_at

```mach
pub fun fin_at(lc: *LowerContext, i: u32) ast_id.StmtId;
```

## fun pop_fins_to

```mach
pub fun pop_fins_to(lc: *LowerContext, mark: u32);
```

## fun lower_type

```mach
pub fun lower_type(lc: *LowerContext, tid: type.TypeId) res[ir_type.IrTypeId, fail.Fail];
```

## fun debug_type_of

```mach
pub fun debug_type_of(lc: *LowerContext, tid: type.TypeId) res[ir_debug.TypeId, fail.Fail];
```

the debug type a debug record names for source type `tid`, as substituted for
the code being lowered; TYPE_NIL when this lowering keeps no debug information
or nothing describes the type

## fun debug_types_finish

```mach
pub fun debug_types_finish(lc: *LowerContext) err[fail.Fail];
```

describes every pointee left waiting: one that holds by value an instance of
an expansive generic this module describes no value of stays untyped, since
such a generic has unboundedly many instances behind its pointers

## fun block_terminated

```mach
pub fun block_terminated(lc: *LowerContext) bool;
```

## fun is_void_ir

```mach
pub fun is_void_ir(lc: *LowerContext, tid: ir_type.IrTypeId) bool;
```

## fun expr_type_of

```mach
pub fun expr_type_of(lc: *LowerContext, eid: ast_id.ExprId) type.TypeId;
```

an expression's type inside an instance's scope is the instance's, never the template's:
sema records the template's and every type decision here is made against this instance
an expression's type as the frame the code is read through decided it, else the template's
under the instance's substitution

## fun expr_float_width

```mach
pub fun expr_float_width(lc: *LowerContext, eid: ast_id.ExprId) float.FloatWidth;
```

## fun expr_is_secret

```mach
pub fun expr_is_secret(lc: *LowerContext, eid: ast_id.ExprId) bool;
```

## fun type_is_secret

```mach
pub fun type_is_secret(lc: *LowerContext, tid: type.TypeId) bool;
```

## fun type_contains_secret

```mach
pub fun type_contains_secret(lc: *LowerContext, tid: type.TypeId) bool;
```

## fun type_carries_secret

```mach
pub fun type_carries_secret(lc: *LowerContext, tid: type.TypeId) bool;
```

## fun bind_secrecy_of

```mach
pub fun bind_secrecy_of(lc: *LowerContext, tid: type.TypeId) ct.BindSecrecy;
```

the secrecy an inline-asm binding of this type carries: its own storage (`^usize`,
`^*T`) and, for a pointer, whether the pointee reaches a secret (`*^T`, `**^T`,
`*rec{ k: ^u32 }`); a pointer to a secret is a public address and a secret load

## fun substitute_type

```mach
pub fun substitute_type(lc: *LowerContext, tid: type.TypeId) type.TypeId;
```

## fun type_resolved_of

```mach
pub fun type_resolved_of(lc: *LowerContext, tid: ast_id.TypeId) type.TypeId;
```

## fun decl_type_of

```mach
pub fun decl_type_of(lc: *LowerContext, did: ast_id.DeclId) type.TypeId;
```

## fun instance_of_call

```mach
pub fun instance_of_call(lc: *LowerContext, eid: ast_id.ExprId) res[*fe_sema.instance.Instance, fail.Fail];
```

the instance sema decided the call or generic reference `eid` names, in the frame the
code is read through

## fun instance_link_name

```mach
pub fun instance_link_name(lc: *LowerContext, item: *fe_sema.instance.Instance) res[intern.StrId, fail.Fail];
```

the symbol an instance is emitted and referenced under

## fun gate_of

```mach
pub fun gate_of(lc: *LowerContext, cond: ast_id.ExprId) opt[bool];
```

the verdict sema gave a statement gate in the frame the code is read through

## fun each_array

```mach
pub fun each_array(lc: *LowerContext, stmt: ast_id.StmtId) res[comptime.CTValue, fail.Fail];
```

the published value of the array sema typed the `$each` at `stmt` over, in the frame the code
is read through

## fun iteration_frame

```mach
pub fun iteration_frame(lc: *LowerContext, stmt: ast_id.StmtId, index: u32) res[u32, fail.Fail];
```

the frame of iteration `index` of the `$each` at `stmt` under the current frame

## fun decl_ret_sem

```mach
pub fun decl_ret_sem(lc: *LowerContext, did: ast_id.DeclId) type.TypeId;
```

## fun function_return_ir

```mach
pub fun function_return_ir(lc: *LowerContext, did: ast_id.DeclId) res[ir_type.IrTypeId, fail.Fail];
```

## rec FnSig

```mach
pub rec FnSig;
```

a function's signature and the extension each parameter declares, which the
signless signature does not carry

## fun lower_fn_sig

```mach
pub fun lower_fn_sig(lc: *LowerContext, did: ast_id.DeclId) res[FnSig, fail.Fail];
```

the signature of the semantic type `tid` and, when it is a function type,
its parameters' extensions

## fun fun_type_ext

```mach
pub fun fun_type_ext(lc: *LowerContext, tid: type.TypeId) res[ir_type.ExtListId, fail.Fail];
```

the extensions the parameters of the semantic function type `tid` declare,
EXT_LIST_EMPTY for any other type

## fun pack_instance_sig

```mach
pub fun pack_instance_sig(lc: *LowerContext, fn_ty: type.TypeId, types: *type.TypeId, type_len: u32) res[FnSig, fail.Fail];
```

the signature of a pack instance of the function typed `fn_ty`: its fixed parameters, then
one per element of the pack

## fun instance_ref_sig

```mach
pub fun instance_ref_sig(lc: *LowerContext, item: *fe_sema.instance.Instance) res[FnSig, fail.Fail];
```

the signature an instance is referenced under: the one sema settled when it asked for it

## fun expr_symbol_of

```mach
pub fun expr_symbol_of(lc: *LowerContext, eid: ast_id.ExprId) resolve.SymbolId;
```

## fun module_source

```mach
pub fun module_source(lc: *LowerContext) str;
```

the source of the module in scope, read when the scope was made

## fun ast_source

```mach
pub fun ast_source(s: *session.Session, a: *ast.Ast) res[str, fail.Fail];
```

the source text `a` was parsed from

## fun declared_op

```mach
pub fun declared_op(lc: *LowerContext, d: Declared, out_op: *u32) u32;
```

the operation of the selected target an `#[op]` function stands for, NO_OP_SET for any other

## fun emit_dbg_birth

```mach
pub fun emit_dbg_birth(lc: *LowerContext, name_span: lang_source.Span, ty: ir_type.IrTypeId,
ty_sem: type.TypeId, is_param: bool, val_v: value.Value,
slot: opt[value.Value]) err[fail.Fail];
```

## fun symbol_of

```mach
pub fun symbol_of(lc: *LowerContext, sid: resolve.SymbolId) opt[*resolve.Symbol];
```

## fun ensure_extern_function

```mach
pub fun ensure_extern_function(lc: *LowerContext, name: intern.StrId, sig: FnSig, import_flag: bool) res[u32, fail.Fail];
```

## fun ensure_extern_global

```mach
pub fun ensure_extern_global(lc: *LowerContext, name: intern.StrId, ty: ir_type.IrTypeId, import_flag: bool) res[u32, fail.Fail];
```

## fun add_rodata_bytes

```mach
pub fun add_rodata_bytes(lc: *LowerContext, init: value.Value, pty: ir_type.IrTypeId) res[value.Value, fail.Fail];
```

## fun recover_type

```mach
pub fun recover_type(lc: *LowerContext, r: res[type.TypeId, fail.Fail]) type.TypeId;
```

