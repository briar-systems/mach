# mach.lang.me.lower.context

## rec LoopFrame

```mach
pub rec LoopFrame;
```

## rec Machine

```mach
pub rec Machine;
```

the target lowering builds for: the tuple its modules are named for, the target mirl
assembled from it, and the context the build's selection opened, none for a target with
no baseline
slots: the address space the target's frame slots live in

## fun machine_of

```mach
pub fun machine_of(x: *target_owner.Assembled) res[Machine, fail.Fail];
```

the machine of a target the owners assembled, borrowing it

## rec FnInfo

```mach
pub rec FnInfo;
```

what the driver reads of a lowered module beside its mirl module: its name and, for
each function of the module by index, what mach declared it as

test: a test, run by the dispatcher under `name`
noreturn: it never returns, so falling off its end is unreachable
defined: the module gives it a body

## rec Unit

```mach
pub rec Unit;
```

what mach keeps of a lowered module once its mirl module is compiled and gone: its name
and the facts of its functions

exports: the linkage names a root-project `fwd` puts on a shared library's surface,
         which another module of the image defines

## fun unit_init

```mach
pub fun unit_init(a: *A.Allocator, name: intern.StrId, root_project: bool) Unit;
```

an empty unit named `name`, its facts held in `a`

## fun unit_dnit

```mach
pub fun unit_dnit(u: *Unit);
```

release a unit's facts

## fun module_init

```mach
pub fun module_init(a: *A.Allocator, machine: *Machine) res[mirl_module.Module, fail.Fail];
```

an empty mirl module in `a`, built for the machine's target and stated as mach's

## fun fn_info

```mach
pub fun fn_info(u: *Unit, f: u32) opt[*FnInfo];
```

the facts of function `f` of a unit, none past its functions

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

## rec ClassCopy

```mach
pub rec ClassCopy;
```

a copy of a function whose pointer parameters take the classes its call sites pass,
lowered into this module once the module's own declarations are

origin: the module that declares the function
decl: its declaration there
name: the copy's linkage name, the function's own with the classes it takes
sig: the copy's signature

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
l: *linkage.Linkage,
tgt: *resolved.Target,
machine: *Machine,
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
pub fun init_context(lc: *LowerContext, req: *LowerRequest, m: *mirl_module.Module, unit: *Unit) err[fail.Fail];
```

## fun dnit_context

```mach
pub fun dnit_context(lc: *LowerContext);
```

release a lowering context; a nil context holds nothing

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

## fun foreign

```mach
pub fun foreign(lc: *LowerContext, sym: *resolve.Symbol) bool;
```

whether `sym` names a binding a module other than the one in scope declares

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

## fun refuse_nesting

```mach
pub fun refuse_nesting(lc: *LowerContext, span: lang_source.Span) fail.Fail;
```

the lowering recurses as deep as the input nests, on frames far larger than the checks
before it, so each walk asks for stack before it descends and refuses the node it stands on

## fun refuse_type_nesting

```mach
pub fun refuse_type_nesting(lc: *LowerContext) fail.Fail;
```

the same refusal of a walk over a type, which has no place of its own, at the node the
lowering stands on

## fun refuse

```mach
pub fun refuse(lc: *LowerContext, f: fail.Fail) fail.Fail;
```

a failure a walk with no place of its own returned: a refusal of the input is reported under
its own kind at the node the lowering stands on; any other failure is the lowering's own

## fun record_eval_result

```mach
pub fun record_eval_result(lc: *LowerContext, r: res[comptime.CTValue, comptime.EvalFail]);
```

## fun type_node

```mach
pub fun type_node(lc: *LowerContext, tid: ast_id.TypeId) res[ast_type.Type, fail.Fail];
```

the type node `tid` as name resolution read it, a literal head it split naming the type by its
path's prefix and the case by the last segment

## fun expression

```mach
pub fun expression(lc: *LowerContext, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail];
```

## fun resolve_module_member_const

```mach
pub fun resolve_module_member_const(lc: *LowerContext, eid: ast_id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

## fun comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(lc: *LowerContext, eid: ast_id.ExprId) bool;
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

## fun resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(lc: *LowerContext, eid: ast_id.ExprId) res[opt[u32], comptime.EvalFail];
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

## fun resolve_type_query

```mach
pub fun resolve_type_query(lc: *LowerContext, tid: u32, which: u8) res[opt[comptime.CTValue], comptime.EvalFail];
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

## fun resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(lc: *LowerContext, eid: u32) res[opt[comptime.CTValue], comptime.EvalFail];
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

## fun member_offset

```mach
pub fun member_offset(lc: *LowerContext, tid: type.TypeId, ir: Type, index: u32) res[u64, fail.Fail];
```

the byte offset of field or case `index` of the source type `tid`, lowered as `ir`: a
record's field, a union's member at zero, or a tag case's payload

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
pub fun begin_function(lc: *LowerContext, fn_index: u32, ret_type: Type);
```

## fun bind_local

```mach
pub fun bind_local(lc: *LowerContext, sym: resolve.SymbolId, v: value.Value) err[fail.Fail];
```

## fun lookup_local

```mach
pub fun lookup_local(lc: *LowerContext, sym: resolve.SymbolId) opt[value.Value];
```

## fun push_loop

```mach
pub fun push_loop(lc: *LowerContext, header: id.Block, exit: id.Block) err[fail.Fail];
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

## val TAG_DISCRIMINANT

```mach
pub val TAG_DISCRIMINANT: u32 = 0
```

a tag lowers to a structure of its discriminant and, when a case holds a payload, the
union of the payloads, which every payload starts at

## val TAG_PAYLOAD

```mach
pub val TAG_PAYLOAD:      u32 = 1
```

## fun ptr_or_none

```mach
pub fun ptr_or_none(lc: *LowerContext) Type;
```

the pointer type, which every address is held in

## fun byte_type

```mach
pub fun byte_type(lc: *LowerContext) Type;
```

the byte type, which a global another module defines is declared as when only its
address is taken

## fun builder_state

```mach
pub fun builder_state(lc: *LowerContext) err[fail.Fail];
```

the first refusal the builder met, which every builder call after it skips

## fun int_type

```mach
pub fun int_type(lc: *LowerContext, bits: u32) res[Type, fail.Fail];
```

## fun ptr_type

```mach
pub fun ptr_type(lc: *LowerContext) res[Type, fail.Fail];
```

## fun lower_type

```mach
pub fun lower_type(lc: *LowerContext, tid: type.TypeId) res[Type, fail.Fail];
```

the mirl type a value of the source type `tid` is held in

## fun tag_case_has_payload

```mach
pub fun tag_case_has_payload(lc: *LowerContext, tid: type.TypeId, i: u32) bool;
```

whether case `i` of the tag `tid` holds a payload

## fun tag_payload_member

```mach
pub fun tag_payload_member(lc: *LowerContext, tid: type.TypeId, i: u32) u32;
```

the member of the payload union case `i` of the tag `tid` is laid out as

## fun tag_disc_type

```mach
pub fun tag_disc_type(lc: *LowerContext, tid: type.TypeId) res[Type, fail.Fail];
```

the integer a tag's discriminant is held in

## fun storage_align

```mach
pub fun storage_align(lc: *LowerContext, tid: type.TypeId, ir: Type) u64;
```

the alignment a value of the source type `tid`, held in `ir`, is stored at: a decorated
record's, else the target's

## fun slot_for

```mach
pub fun slot_for(lc: *LowerContext, tid: type.TypeId, ir: Type) res[value.Value, fail.Fail];
```

a stack slot for a value of the source type `tid`, held in `ir`

## fun debug_type_of

```mach
pub fun debug_type_of(lc: *LowerContext, tid: type.TypeId) res[opt[mirl_debug.Type], fail.Fail];
```

the debug type a debug record names for source type `tid`, as substituted for the code
being lowered; none when this lowering keeps no debug information

## fun debug_types_finish

```mach
pub fun debug_types_finish(lc: *LowerContext) err[fail.Fail];
```

describe every pointee left waiting, then link the table. an instance of an expansive
generic the module describes no value of stays undescribed past its pointer, since such
a generic has unboundedly many instances behind its pointers, and is described opaque

## fun block_terminated

```mach
pub fun block_terminated(lc: *LowerContext) bool;
```

whether the block being built already ends

## fun is_void_ir

```mach
pub fun is_void_ir(lc: *LowerContext, tid: Type) bool;
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
pub fun function_return_ir(lc: *LowerContext, did: ast_id.DeclId) res[Type, fail.Fail];
```

## rec FnSig

```mach
pub rec FnSig;
```

a function's signature: its parameters and result, each narrow integer stating the
extension its source type's signedness gives it

## fun lower_fn_sig

```mach
pub fun lower_fn_sig(lc: *LowerContext, did: ast_id.DeclId) res[FnSig, fail.Fail];
```

the signature of the function `did` declares

## fun fn_sig_of_type

```mach
pub fun fn_sig_of_type(lc: *LowerContext, tid: type.TypeId) res[FnSig, fail.Fail];
```

the signature a value of the source function type `tid` is called through

## fun ext_of

```mach
pub fun ext_of(lc: *LowerContext, sem: type.TypeId) types.Extension;
```

the extension a convention gives an integer narrower than a register: its declared
signedness. a bool is a byte read unsigned

## fun pack_instance_sig

```mach
pub fun pack_instance_sig(lc: *LowerContext, fn_ty: type.TypeId, elems: *type.TypeId, elem_len: u32) res[FnSig, fail.Fail];
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

## fun declared_op

```mach
pub fun declared_op(lc: *LowerContext, d: Declared) opt[*target_definition.Op];
```

the operation of the selected target an `#[op]` function stands for, none for any other

## fun emit_dbg_birth

```mach
pub fun emit_dbg_birth(lc: *LowerContext, name_span: lang_source.Span, ty_sem: type.TypeId, param: opt[u32], val_v: value.Value,
slot: opt[value.Value]) err[fail.Fail];
```

a source variable named at `name_span` begins: one held in a stack slot is bound to the
slot, which describes it wherever it lives, and any other, a parameter, to its value
from here on

## fun symbol_of

```mach
pub fun symbol_of(lc: *LowerContext, sid: resolve.SymbolId) opt[*resolve.Symbol];
```

## fun external_linkage

```mach
pub fun external_linkage() Linkage;
```

the linkage of a symbol another module defines, or a definition every module sees

## fun add_function

```mach
pub fun add_function(lc: *LowerContext, data: FunctionData, info: FnInfo) res[u32, fail.Fail];
```

add a function to the module with what mach declares of it

## fun function_named

```mach
pub fun function_named(lc: *LowerContext, name: intern.StrId) opt[u32];
```

the function of the module named `name`, none when no function is

## fun global_named

```mach
pub fun global_named(lc: *LowerContext, name: intern.StrId) opt[u32];
```

the global of the module named `name`, none when no global is

## fun export_add

```mach
pub fun export_add(lc: *LowerContext, name: intern.StrId) err[fail.Fail];
```

put `name` on the shared library's export surface

## fun define_function

```mach
pub fun define_function(lc: *LowerContext, data: FunctionData, info: FnInfo) res[u32, fail.Fail];
```

the module's definition of a function: a reference earlier in the module declared it,
and the definition takes the declaration's place

## fun define_global

```mach
pub fun define_global(lc: *LowerContext, g: GlobalData) res[u32, fail.Fail];
```

the module's definition of a global, in place of a declaration a reference made

## fun ensure_extern_function

```mach
pub fun ensure_extern_function(lc: *LowerContext, name: intern.StrId, sig: FnSig) res[u32, fail.Fail];
```

the module's function `name`, declared here when another module defines it

## fun class_copy

```mach
pub fun class_copy(lc: *LowerContext, origin: session.ModuleId, decl: ast_id.DeclId, base: intern.StrId, sig: Type) res[u32, fail.Fail];
```

the copy of the function `decl` of `origin`, linked as `base`, whose pointer parameters
take the classes `sig` states, declared here and lowered once the module's own
declarations are. one name is one copy, so a copy asked for twice is asked for once

## fun ensure_extern_global

```mach
pub fun ensure_extern_global(lc: *LowerContext, name: intern.StrId, ty: Type) res[u32, fail.Fail];
```

the module's global `name` of `ty`, declared here when another module defines it

## fun add_global

```mach
pub fun add_global(lc: *LowerContext, g: GlobalData) res[u32, fail.Fail];
```

## fun constant_global

```mach
pub fun constant_global(lc: *LowerContext, v: value.Value) res[value.Value, fail.Fail];
```

a read-only global of the module's own holding the constant `v`, whose address is the value

