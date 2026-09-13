# mach.lang.me.lower.context

## val MAX_VALUE_INSTANCES

```mach
pub val MAX_VALUE_INSTANCES: u32 = 4096
```

## val MAX_PACK_INSTANCES

```mach
pub val MAX_PACK_INSTANCES: u32 = 8192
```

## val INSTANCE_NONE

```mach
pub val INSTANCE_NONE: u32 = 4294967295
```

## val MAX_INSTANCE_DEPTH

```mach
pub val MAX_INSTANCE_DEPTH: u32 = 32
```

## val MAX_INSTANCES

```mach
pub val MAX_INSTANCES: u32 = 8192
```

## rec LoopFrame

```mach
pub rec LoopFrame;
```

## rec Instance

```mach
pub rec Instance;
```

## rec ValueInstance

```mach
pub rec ValueInstance;
```

## rec PackInstance

```mach
pub rec PackInstance;
```

## rec ModuleScope

```mach
pub rec ModuleScope;
```

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

## fun scope_nil

```mach
pub fun scope_nil() ModuleScope;
```

## fun module_scope

```mach
pub fun module_scope(
s: *session.Session,
own_module: session.ModuleId,
a: *ast.Ast,
fqn: intern.StrId,
sema_result: *sema.SemaResult,
rr: *resolve.ResolveResult,
ctx: *comptime.ComptimeCtx,
definitions: *scx.DefinitionReader) res[ModuleScope, fail.Fail];
```

## fun origin_scope

```mach
pub fun origin_scope(lc: *LowerContext, origin: session.ModuleId) res[ModuleScope, fail.Fail];
```

## fun request

```mach
pub fun request(
s: *session.Session,
tgt: *target.Target,
malloc: *A.Allocator,
a: *ast.Ast,
fqn: intern.StrId,
own_module: session.ModuleId,
sema_result: *sema.SemaResult,
rr: *resolve.ResolveResult,
ctx: *comptime.ComptimeCtx,
test_mode: bool,
debug: bool,
checks: bool,
diags: *diagnostic.DiagnosticStore,
deps: *sema.SemaDeps) res[LowerRequest, fail.Fail];
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
pub fun init_context(lc: *LowerContext, req: *LowerRequest, m: *ir.Module) err[fail.Fail];
```

## fun dnit_context

```mach
pub fun dnit_context(lc: *LowerContext);
```

## fun symbol_definition

```mach
pub fun symbol_definition(lc: *LowerContext, sym: *resolve.Symbol) res[scx.DefinedSymbol, fail.Fail];
```

## fun declaring_comptime_ctx

```mach
pub fun declaring_comptime_ctx(lc: *LowerContext, sym: *resolve.Symbol) *comptime.ComptimeCtx;
```

## fun gated_lower_member_const_message

```mach
pub fun gated_lower_member_const_message(lc: *LowerContext, name: intern.StrId, dep_fqn: intern.StrId) str;
```

## fun report_gate

```mach
pub fun report_gate(lc: *LowerContext, span: token.Span, message: str) fail.Fail;
```

## fun record_eval_result

```mach
pub fun record_eval_result(lc: *LowerContext, r: res[comptime.CTValue, comptime.EvalFail]);
```

## fun expression

```mach
pub fun expression(lc: *LowerContext, eid: id.ExprId) res[aexpr.Expr, fail.Fail];
```

## fun resolve_module_member_const

```mach
pub fun resolve_module_member_const(lc: *LowerContext, eid: id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(lc: *LowerContext, eid: id.ExprId) bool;
```

## fun resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(lc: *LowerContext, eid: id.ExprId) res[opt[u32], comptime.EvalFail];
```

## fun resolve_type_query

```mach
pub fun resolve_type_query(lc: *LowerContext, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(lc: *LowerContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

## fun resolve_field_member

```mach
pub fun resolve_field_member(lc: *LowerContext, owner: u32, index: u32, pick: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

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
pub fun bind_local(lc: *LowerContext, sym: resolve.SymbolId, v: value.Value, secret: bool) err[fail.Fail];
```

## fun lookup_local_name

```mach
pub fun lookup_local_name(lc: *LowerContext, name: intern.StrId) opt[LocalBinding];
```

## fun lookup_local

```mach
pub fun lookup_local(lc: *LowerContext, sym: resolve.SymbolId) opt[value.Value];
```

## fun push_loop

```mach
pub fun push_loop(lc: *LowerContext, header: ir.BlockId, exit: ir.BlockId) err[fail.Fail];
```

## fun pop_loop

```mach
pub fun pop_loop(lc: *LowerContext);
```

## fun current_loop

```mach
pub fun current_loop(lc: *LowerContext) *LoopFrame;
```

## fun enqueue_instance

```mach
pub fun enqueue_instance(lc: *LowerContext, decl: id.DeclId, origin: session.ModuleId,
args: *type.TypeId, arg_len: u32, name: intern.StrId,
bare: intern.StrId) err[fail.Fail];
```

## fun enqueue_value_instance

```mach
pub fun enqueue_value_instance(lc: *LowerContext, decl: id.DeclId, origin: session.ModuleId,
vals: *comptime.CTValue, names: *intern.StrId, val_len: u32,
name: intern.StrId) err[fail.Fail];
```

## fun enqueue_pack_instance

```mach
pub fun enqueue_pack_instance(lc: *LowerContext, decl: id.DeclId, origin: session.ModuleId,
args: *type.TypeId, arg_len: u32,
types: *type.TypeId, type_len: u32, name: intern.StrId) err[fail.Fail];
```

## fun push_fin

```mach
pub fun push_fin(lc: *LowerContext, body: id.StmtId) err[fail.Fail];
```

## fun fin_count

```mach
pub fun fin_count(lc: *LowerContext) u32;
```

## fun fin_at

```mach
pub fun fin_at(lc: *LowerContext, i: u32) id.StmtId;
```

## fun pop_fins_to

```mach
pub fun pop_fins_to(lc: *LowerContext, mark: u32);
```

## fun lower_type

```mach
pub fun lower_type(lc: *LowerContext, tid: type.TypeId) res[ir_type.IrTypeId, fail.Fail];
```

## fun gate_is_active

```mach
pub fun gate_is_active(lc: *LowerContext, source: str, cond: id.ExprId,
scope: comptime.GateScope, cache: bool) res[bool, fail.Fail];
```

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
pub fun expr_type_of(lc: *LowerContext, eid: id.ExprId) type.TypeId;
```

## fun expr_float_width

```mach
pub fun expr_float_width(lc: *LowerContext, eid: id.ExprId) float.FloatWidth;
```

## fun float_width_of_type

```mach
pub fun float_width_of_type(lc: *LowerContext, tid: type.TypeId) float.FloatWidth;
```

## fun expr_is_secret

```mach
pub fun expr_is_secret(lc: *LowerContext, eid: id.ExprId) bool;
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

## fun substitute_type

```mach
pub fun substitute_type(lc: *LowerContext, tid: type.TypeId) type.TypeId;
```

## fun type_resolved_of

```mach
pub fun type_resolved_of(lc: *LowerContext, tid: id.TypeId) type.TypeId;
```

## fun decl_type_of

```mach
pub fun decl_type_of(lc: *LowerContext, did: id.DeclId) type.TypeId;
```

## fun decl_ret_sem

```mach
pub fun decl_ret_sem(lc: *LowerContext, did: id.DeclId) type.TypeId;
```

## fun function_return_ir

```mach
pub fun function_return_ir(lc: *LowerContext, did: id.DeclId) res[ir_type.IrTypeId, fail.Fail];
```

## fun pack_instance_sig

```mach
pub fun pack_instance_sig(lc: *LowerContext, d: *adecl.Decl, fixed_count: u32,
types: *type.TypeId, type_len: u32, ret_ir: ir_type.IrTypeId) res[ir_type.IrTypeId, fail.Fail];
```

## fun instance_ref_sig

```mach
pub fun instance_ref_sig(lc: *LowerContext, decl_id: id.DeclId, origin: session.ModuleId,
args: *type.TypeId, arg_len: u32) res[ir_type.IrTypeId, fail.Fail];
```

## fun pack_instance_ref_sig

```mach
pub fun pack_instance_ref_sig(lc: *LowerContext, decl_id: id.DeclId, origin: session.ModuleId,
args: *type.TypeId, arg_len: u32,
types: *type.TypeId, type_len: u32) res[ir_type.IrTypeId, fail.Fail];
```

## fun expr_symbol_of

```mach
pub fun expr_symbol_of(lc: *LowerContext, eid: id.ExprId) resolve.SymbolId;
```

## fun module_source

```mach
pub fun module_source(lc: *LowerContext) str;
```

## fun ast_source

```mach
pub fun ast_source(lc: *LowerContext, a: *ast.Ast) str;
```

## fun decl_target_op

```mach
pub fun decl_target_op(a: *ast.Ast, src: str, d: *adecl.Decl, defs: *isa.TargetDefs, out_op: *u32) u32;
```

## fun emit_dbg_birth

```mach
pub fun emit_dbg_birth(lc: *LowerContext, name_span: token.Span, ty: ir_type.IrTypeId,
ty_sem: type.TypeId, is_param: bool, val_v: value.Value,
slot: opt[value.Value]) err[fail.Fail];
```

## fun symbol_of

```mach
pub fun symbol_of(lc: *LowerContext, sid: resolve.SymbolId) opt[*resolve.Symbol];
```

## fun ensure_extern_function

```mach
pub fun ensure_extern_function(lc: *LowerContext, name: intern.StrId, sig: ir_type.IrTypeId, import_flag: bool) res[u32, fail.Fail];
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

