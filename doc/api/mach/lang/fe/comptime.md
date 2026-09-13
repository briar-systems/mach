# mach.lang.fe.comptime

## def CTKind

```mach
pub def CTKind: u8
```

## val CT_KIND_INT

```mach
pub val CT_KIND_INT:   CTKind = 0
```

## val CT_KIND_FLOAT

```mach
pub val CT_KIND_FLOAT: CTKind = 1
```

## val CT_KIND_STR

```mach
pub val CT_KIND_STR:   CTKind = 2
```

## val CT_KIND_TYPE

```mach
pub val CT_KIND_TYPE: CTKind = 3
```

## val CT_KIND_FIELD

```mach
pub val CT_KIND_FIELD: CTKind = 4
```

## rec FieldRef

```mach
pub rec FieldRef;
```

## val CT_KIND_PACK_ELEM

```mach
pub val CT_KIND_PACK_ELEM: CTKind = 5
```

## rec PackElemRef

```mach
pub rec PackElemRef;
```

## val CT_KIND_CONST_ELEM

```mach
pub val CT_KIND_CONST_ELEM: CTKind = 6
```

## val CT_KIND_CASE

```mach
pub val CT_KIND_CASE: CTKind = 7
```

a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor

## def GateState

```mach
pub def GateState: u8
```

## val GATE_STATE_UNKNOWN

```mach
pub val GATE_STATE_UNKNOWN:  GateState = 0
```

## val GATE_STATE_TRUE

```mach
pub val GATE_STATE_TRUE:     GateState = 1
```

## val GATE_STATE_FALSE

```mach
pub val GATE_STATE_FALSE:    GateState = 2
```

## val GATE_STATE_REJECTED

```mach
pub val GATE_STATE_REJECTED: GateState = 3
```

## def GateOutcome

```mach
pub def GateOutcome: u8
```

## val GATE_INACTIVE

```mach
pub val GATE_INACTIVE:        GateOutcome = 0
```

## val GATE_ACTIVE

```mach
pub val GATE_ACTIVE:          GateOutcome = 1
```

## val GATE_DEFERRED

```mach
pub val GATE_DEFERRED:        GateOutcome = 2
```

## val GATE_REJECTED

```mach
pub val GATE_REJECTED:        GateOutcome = 3
```

## val GATE_FAILED

```mach
pub val GATE_FAILED:          GateOutcome = 4
```

## val GATE_AWAITING_LAYOUT

```mach
pub val GATE_AWAITING_LAYOUT: GateOutcome = 5
```

## val GATE_AWAITING_PHASE

```mach
pub val GATE_AWAITING_PHASE:  GateOutcome = 6
```

## val GATE_AWAITING_TYPES

```mach
pub val GATE_AWAITING_TYPES:  GateOutcome = 7
```

## rec GateSelection

```mach
pub rec GateSelection;
```

## fun gate_selection

```mach
pub fun gate_selection(outcome: GateOutcome, branch: u32) GateSelection;
```

## rec ConstElemRef

```mach
pub rec ConstElemRef;
```

## val COMPTIME_BARE_IDENT_MSG

```mach
pub val COMPTIME_BARE_IDENT_MSG: str =
"comptime parameters are referenced without `$`
```

## val COMPTIME_IDENT_UNBOUND_MSG

```mach
pub val COMPTIME_IDENT_UNBOUND_MSG: str =
"identifier is not a comptime constant in scope"
```

## val COMPTIME_LAYOUT_NO_RESOLVER_MSG

```mach
pub val COMPTIME_LAYOUT_NO_RESOLVER_MSG: str =
"a layout intrinsic is only comptime-evaluable after type checking: an arm of this `$if` declares something, so the gate selects DECLARATIONS and is decided during name resolution, before any type is laid out. a `$if` whose every arm declares nothing is decided during type checking and can measure a type"
```

## rec CTValue

```mach
pub rec CTValue;
```

## rec NamedConst

```mach
pub rec NamedConst;
```

## val COMPTIME_MEMBER_NO_RESOLVER_MSG

```mach
pub val COMPTIME_MEMBER_NO_RESOLVER_MSG: str =
"a module-qualified member path is only comptime-evaluable after name resolution"
```

## val COMPTIME_TYPE_NO_RESOLVER_MSG

```mach
pub val COMPTIME_TYPE_NO_RESOLVER_MSG: str =
"a type comparison is only comptime-evaluable after type checking"
```

## val COMPTIME_TYPE_UNRESOLVED_MSG

```mach
pub val COMPTIME_TYPE_UNRESOLVED_MSG: str =
"type comparison operand does not name a type"
```

## val FIELD_SEL_NAME

```mach
pub val FIELD_SEL_NAME:         u8 = 0
```

## val FIELD_SEL_TYPE

```mach
pub val FIELD_SEL_TYPE:         u8 = 1
```

## val FIELD_SEL_OFFSET

```mach
pub val FIELD_SEL_OFFSET:       u8 = 2
```

## val FIELD_SEL_ZERO_BY_NAME

```mach
pub val FIELD_SEL_ZERO_BY_NAME: u8 = 3
```

## val FIELD_SEL_HAS_PAYLOAD

```mach
pub val FIELD_SEL_HAS_PAYLOAD:  u8 = 4
```

## val FIELD_SEL_CODE

```mach
pub val FIELD_SEL_CODE:         u8 = 5
```

## val FIELD_SEL_TYPE_BY_NAME

```mach
pub val FIELD_SEL_TYPE_BY_NAME: u8 = 6
```

## val COMPTIME_SEL_NOT_CONSTANT_MSG

```mach
pub val COMPTIME_SEL_NOT_CONSTANT_MSG: str =
"`sel` at compile time tests a constant tag: a module `val` constructed as `Type.case;
```

## val COMPTIME_SEL_DESCRIPTOR_NEEDS_TYPES_MSG

```mach
pub val COMPTIME_SEL_DESCRIPTOR_NEEDS_TYPES_MSG: str =
"a case descriptor names its case only once the tag's fields are known, so this `sel` is decided during type checking"
```

## val COMPTIME_CASE_DESCRIPTOR_OPERAND_MSG

```mach
pub val COMPTIME_CASE_DESCRIPTOR_OPERAND_MSG: str =
"a bracketed case operand is not a comptime case descriptor"
```

## val COMPTIME_CASE_PAYLOAD_MISSING_MSG

```mach
pub val COMPTIME_CASE_PAYLOAD_MISSING_MSG: str =
"this tag case takes no payload"
```

## val COMPTIME_IDENT_RUNTIME_MSG

```mach
pub val COMPTIME_IDENT_RUNTIME_MSG: str =
"identifier names a runtime binding, so it has no comptime value"
```

## val COMPTIME_ALIAS_FLOAT_MSG

```mach
pub val COMPTIME_ALIAS_FLOAT_MSG: str =
"the width of a float constant whose type comes through an alias is not readable before type checking, so it has no comptime value here. spell the type as `f32` or `f64` on the declaration to fold it at this position"
```

## val FLOAT_WIDTH_MISMATCH_MSG

```mach
pub val FLOAT_WIDTH_MISMATCH_MSG: str =
"float operands have different declared widths
```

## val TYPE_QUERY_IS_RECORD

```mach
pub val TYPE_QUERY_IS_RECORD:  u8 = 0
```

## val TYPE_QUERY_IS_UNION

```mach
pub val TYPE_QUERY_IS_UNION:   u8 = 1
```

## val TYPE_QUERY_IS_POINTER

```mach
pub val TYPE_QUERY_IS_POINTER: u8 = 2
```

## val TYPE_QUERY_NAME

```mach
pub val TYPE_QUERY_NAME:       u8 = 3
```

## val TYPE_QUERY_IS_SECRET

```mach
pub val TYPE_QUERY_IS_SECRET:  u8 = 4
```

## val TYPE_QUERY_IS_TAG

```mach
pub val TYPE_QUERY_IS_TAG:     u8 = 5
```

## def PhaseCapabilityKind

```mach
pub def PhaseCapabilityKind: u8
```

## val PHASE_CAP_NONE

```mach
pub val PHASE_CAP_NONE:           PhaseCapabilityKind = 0
```

## val PHASE_CAP_LOADING

```mach
pub val PHASE_CAP_LOADING:        PhaseCapabilityKind = 1
```

## val PHASE_CAP_RESOLUTION

```mach
pub val PHASE_CAP_RESOLUTION:     PhaseCapabilityKind = 2
```

## val PHASE_CAP_SEMANTIC_NAMES

```mach
pub val PHASE_CAP_SEMANTIC_NAMES: PhaseCapabilityKind = 3
```

## val PHASE_CAP_SEMANTIC_TYPES

```mach
pub val PHASE_CAP_SEMANTIC_TYPES: PhaseCapabilityKind = 4
```

## val PHASE_CAP_LOWERING

```mach
pub val PHASE_CAP_LOWERING:       PhaseCapabilityKind = 5
```

## rec NoCapabilityContext

```mach
pub rec NoCapabilityContext;
```

## rec PhaseCapabilities

```mach
pub rec PhaseCapabilities[T];
```

## fun no_capabilities

```mach
pub fun no_capabilities() PhaseCapabilities[NoCapabilityContext];
```

## fun loading_capabilities

```mach
pub fun loading_capabilities[T](member: fun(*T, id.ExprId) res[opt[CTValue], EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun resolution_capabilities

```mach
pub fun resolution_capabilities[T](ident: fun(*T, id.ExprId) bool,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_name_capabilities

```mach
pub fun semantic_name_capabilities[T](
member: fun(*T, id.ExprId) res[opt[CTValue], EvalFail],
ident: fun(*T, id.ExprId) bool,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_type_capabilities

```mach
pub fun semantic_type_capabilities[T](
member: fun(*T, id.ExprId) res[opt[CTValue], EvalFail],
type_: fun(*T, id.ExprId) res[opt[u32], EvalFail],
field: fun(*T, u32, u32, u8) res[opt[CTValue], EvalFail],
query: fun(*T, u32, u8) res[opt[CTValue], EvalFail],
layout: fun(*T, u32) res[opt[CTValue], EvalFail],
cast: fun(*T, id.ExprId, CTValue) res[CTValue, EvalFail],
scalar: fun(*T, id.ExprId, CTValue) res[CTValue, EvalFail],
ident: fun(*T, id.ExprId) bool,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun lowering_capabilities

```mach
pub fun lowering_capabilities[T](
member: fun(*T, id.ExprId) res[opt[CTValue], EvalFail],
type_: fun(*T, id.ExprId) res[opt[u32], EvalFail],
field: fun(*T, u32, u32, u8) res[opt[CTValue], EvalFail],
query: fun(*T, u32, u8) res[opt[CTValue], EvalFail],
layout: fun(*T, u32) res[opt[CTValue], EvalFail],
cast: fun(*T, id.ExprId, CTValue) res[CTValue, EvalFail],
scalar: fun(*T, id.ExprId, CTValue) res[CTValue, EvalFail],
ident: fun(*T, id.ExprId) bool,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## val COMPTIME_TYPE_QUERY_NO_RESOLVER_MSG

```mach
pub val COMPTIME_TYPE_QUERY_NO_RESOLVER_MSG: str =
"a comptime type predicate is only evaluable after type checking"
```

## val COMPTIME_FIELD_NO_RESOLVER_MSG

```mach
pub val COMPTIME_FIELD_NO_RESOLVER_MSG: str =
"a field descriptor member is only comptime-evaluable after type checking"
```

## val COMPTIME_FIELD_UNKNOWN_MEMBER_MSG

```mach
pub val COMPTIME_FIELD_UNKNOWN_MEMBER_MSG: str =
"a field descriptor has only `.name`, `.type`, and `.offset`"
```

## val COMPTIME_CASE_UNKNOWN_MEMBER_MSG

```mach
pub val COMPTIME_CASE_UNKNOWN_MEMBER_MSG: str =
"a case descriptor has only `.name`, `.has_payload`, `.type`, `.offset`, and `.code`"
```

## val COMPTIME_DESCRIPTOR_LAYOUT_MSG

```mach
pub val COMPTIME_DESCRIPTOR_LAYOUT_MSG: str =
"this descriptor's offset has no answer: the layout of its owning type could not be determined"
```

## val COMPTIME_CASE_NO_PAYLOAD_MSG

```mach
pub val COMPTIME_CASE_NO_PAYLOAD_MSG: str =
"this case has no payload, so it has no `.type` or `.offset`
```

## val COMPTIME_FIELD_UNAVAILABLE_MSG

```mach
pub val COMPTIME_FIELD_UNAVAILABLE_MSG: str =
"this field descriptor member is not available until lowering"
```

## val COMPTIME_MEMBER_NOT_CONST_MSG

```mach
pub val COMPTIME_MEMBER_NOT_CONST_MSG: str =
"module-qualified member does not name a module-level constant"
```

## def EvalFailKind

```mach
pub def EvalFailKind: u8
```

## val EVAL_FAIL_NONE

```mach
pub val EVAL_FAIL_NONE:         EvalFailKind = 0
```

## val EVAL_FAIL_REJECTED

```mach
pub val EVAL_FAIL_REJECTED:     EvalFailKind = 1
```

## val EVAL_FAIL_UNBOUND

```mach
pub val EVAL_FAIL_UNBOUND:      EvalFailKind = 2
```

## val EVAL_FAIL_NEEDS_MEMBER

```mach
pub val EVAL_FAIL_NEEDS_MEMBER: EvalFailKind = 3
```

## val EVAL_FAIL_NEEDS_TYPES

```mach
pub val EVAL_FAIL_NEEDS_TYPES:  EvalFailKind = 4
```

## val EVAL_FAIL_NEEDS_LAYOUT

```mach
pub val EVAL_FAIL_NEEDS_LAYOUT: EvalFailKind = 5
```

## val EVAL_FAIL_INTERNAL

```mach
pub val EVAL_FAIL_INTERNAL:     EvalFailKind = 6
```

## rec EvalFail

```mach
pub rec EvalFail;
```

## fun eval_error

```mach
pub fun eval_error(kind: EvalFailKind, message: str) EvalFail;
```

## fun eval_from_fail

```mach
pub fun eval_from_fail(f: fail.Fail, rejected_message: str) EvalFail;
```

## fun gate_eval_failure_is_transient

```mach
pub fun gate_eval_failure_is_transient(kind: EvalFailKind) bool;
```

## def ConstKind

```mach
pub def ConstKind: u8
```

## val CONST_INT

```mach
pub val CONST_INT:     ConstKind = 0
```

## val CONST_FLOAT

```mach
pub val CONST_FLOAT:   ConstKind = 1
```

## val CONST_POINTER

```mach
pub val CONST_POINTER: ConstKind = 2
```

## val CONST_SYMBOL

```mach
pub val CONST_SYMBOL:  ConstKind = 3
```

## val CONST_ZERO

```mach
pub val CONST_ZERO:    ConstKind = 4
```

## rec ConstAddress

```mach
pub rec ConstAddress;
```

## rec TypedConstant

```mach
pub rec TypedConstant;
```

## fun const_int

```mach
pub fun const_int(ty: type.TypeId, bits: u64) TypedConstant;
```

## fun const_float

```mach
pub fun const_float(ti: *type.TypeInterner, ty: type.TypeId, f: f64) res[TypedConstant, fail.Fail];
```

## fun const_zero

```mach
pub fun const_zero(ty: type.TypeId) TypedConstant;
```

## fun const_pointer

```mach
pub fun const_pointer(ty: type.TypeId, symbol: intern.StrId, addend: i64) TypedConstant;
```

## fun const_symbol

```mach
pub fun const_symbol(ty: type.TypeId, symbol: intern.StrId) TypedConstant;
```

## val REINTERPRET_MSG_UNSUPPORTED_SOURCE

```mach
pub val REINTERPRET_MSG_UNSUPPORTED_SOURCE: str =
"typed constant reinterpretation: unsupported source kind"
```

## val REINTERPRET_MSG_UNREPRESENTABLE

```mach
pub val REINTERPRET_MSG_UNREPRESENTABLE: str =
"typed constant reinterpretation: unrepresentable operand type"
```

## val REINTERPRET_MSG_WIDTH_MISMATCH

```mach
pub val REINTERPRET_MSG_WIDTH_MISMATCH: str =
"typed constant reinterpretation: bit extent mismatch"
```

## fun const_reinterpret

```mach
pub fun const_reinterpret(ti: *type.TypeInterner, c: TypedConstant, dest_ty: type.TypeId, ptr_width: u32) res[TypedConstant, fail.Fail];
```

## fun const_address_add

```mach
pub fun const_address_add(c: TypedConstant, delta: i64) res[TypedConstant, fail.Fail];
```

## rec DataModel

```mach
pub rec DataModel;
```

## fun data_model

```mach
pub fun data_model(ptr_width: u32, big_endian: bool) DataModel;
```

## rec RelocationRequest

```mach
pub rec RelocationRequest;
```

## fun encode_scalar

```mach
pub fun encode_scalar(ti: *type.TypeInterner, c: TypedConstant, dm: DataModel, base_offset: u32,
out: *u8, out_len: usize, reloc_out: *opt[RelocationRequest]) res[u32, fail.Fail];
```

## rec FrameMark

```mach
pub rec FrameMark;
```

## rec ComptimeEnv

```mach
pub rec ComptimeEnv;
```

## rec ComptimeCtx

```mach
pub rec ComptimeCtx;
```

## fun environment

```mach
pub fun environment(c: *ComptimeCtx) ComptimeEnv;
```

## fun init

```mach
pub fun init(
alloc: *A.Allocator,
target_os_id: u32,
target_arch_id: u32,
target_abi_id: u32,
build_mode_id: u32,
build_pie: u32,
pointer_width: u32,
vector_bits: u32,
has_float: bool,
compiler_name: intern.StrId,
compiler_ver: intern.StrId) ComptimeCtx;
```

## fun set_union_build

```mach
pub fun set_union_build(c: *ComptimeCtx, v: bool);
```

## fun set_target_defs

```mach
pub fun set_target_defs(c: *ComptimeCtx, d: *isa.TargetDefs);
```

## fun set_va_list

```mach
pub fun set_va_list(c: *ComptimeCtx, size: u32, align: u32);
```

## fun set_build_context

```mach
pub fun set_build_context(
c: *ComptimeCtx,
project_id: intern.StrId,
project_ver: intern.StrId,
target_os: intern.StrId,
target_isa: intern.StrId,
target_abi: intern.StrId,
target_platform: intern.StrId,
bin_name: intern.StrId);
```

## fun dnit

```mach
pub fun dnit(c: *ComptimeCtx);
```

## fun prepare_gates

```mach
pub fun prepare_gates(c: *ComptimeCtx, expr_count: u32, decl_count: u32) err[fail.Fail];
```

## fun gate_state

```mach
pub fun gate_state(c: *ComptimeCtx, eid: id.ExprId) GateState;
```

## fun set_gate_state

```mach
pub fun set_gate_state(c: *ComptimeCtx, eid: id.ExprId, state: GateState);
```

## fun load_walked

```mach
pub fun load_walked(c: *ComptimeCtx, did: id.DeclId) bool;
```

## fun mark_load_walked

```mach
pub fun mark_load_walked(c: *ComptimeCtx, did: id.DeclId);
```

## fun defer_float_width

```mach
pub fun defer_float_width(c: *ComptimeCtx, name: intern.StrId) err[fail.Fail];
```

## fun float_width_deferred

```mach
pub fun float_width_deferred(c: *ComptimeCtx, name: intern.StrId) bool;
```

## fun bind

```mach
pub fun bind(c: *ComptimeCtx, name: intern.StrId, value: CTValue) err[fail.Fail];
```

## fun bind_gated

```mach
pub fun bind_gated(c: *ComptimeCtx, name: intern.StrId, value: CTValue, gated: bool) err[fail.Fail];
```

## fun frame_open

```mach
pub fun frame_open(c: *ComptimeCtx) res[FrameMark, fail.Fail];
```

## fun frame_close

```mach
pub fun frame_close(c: *ComptimeCtx, mark: FrameMark) err[fail.Fail];
```

## fun lookup

```mach
pub fun lookup(c: *ComptimeCtx, name: intern.StrId) opt[NamedConst];
```

## fun field_type_of_binding

```mach
pub fun field_type_of_binding[T](c: *ComptimeCtx, cap_ctx: *T, caps: PhaseCapabilities[T], name: intern.StrId) res[opt[u32], EvalFail];
```

## fun evaluate

```mach
pub fun evaluate[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: id.ExprId,
interner: *intern.Interner) res[CTValue, EvalFail];
```

## fun evaluate_at_float_width

```mach
pub fun evaluate_at_float_width[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
e: id.ExprId,
interner: *intern.Interner,
fw: float.FloatWidth) res[CTValue, EvalFail];
```

## rec LitInt

```mach
pub rec LitInt;
```

## rec LitFloat

```mach
pub rec LitFloat;
```

## fun scan_lit_int

```mach
pub fun scan_lit_int(source: str, span: token.Span) res[LitInt, fail.Fail];
```

## fun eval_lit_int

```mach
pub fun eval_lit_int(source: str, span: token.Span) res[CTValue, EvalFail];
```

## fun scan_lit_float

```mach
pub fun scan_lit_float(source: str, span: token.Span) res[LitFloat, fail.Fail];
```

## fun eval_lit_float

```mach
pub fun eval_lit_float(source: str, span: token.Span) res[CTValue, EvalFail];
```

## fun eval_lit_char

```mach
pub fun eval_lit_char(source: str, span: token.Span) res[CTValue, EvalFail];
```

## fun eval_lit_str

```mach
pub fun eval_lit_str(source: str, span: token.Span, interner: *intern.Interner) res[CTValue, EvalFail];
```

## fun cast_scalar

```mach
pub fun cast_scalar(types: *type.TypeInterner, pointer_width: u32, value: CTValue,
from_ty: type.TypeId, to_ty: type.TypeId, reinterpret: bool) res[CTValue, EvalFail];
```

## fun intrinsic_takes_type_operand

```mach
pub fun intrinsic_takes_type_operand(source: str, full: token.Span) bool;
```

## fun is_type_of_call

```mach
pub fun is_type_of_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun type_of_arg

```mach
pub fun type_of_arg(a: *ast.Ast, eid: id.ExprId) id.ExprId;
```

## fun is_fields_call

```mach
pub fun is_fields_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_cases_call

```mach
pub fun is_cases_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_descriptor_seq_call

```mach
pub fun is_descriptor_seq_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

`$fields(T)` and `$cases(T)` are the two descriptor sequences a `$each` walks

## fun is_descriptor

```mach
pub fun is_descriptor(v: CTValue) bool;
```

## fun comptime_if_declares_nothing

```mach
pub fun comptime_if_declares_nothing(a: *ast.Ast, branches_start: u32, branches_len: u32) bool;
```

## def GateProbe

```mach
pub def GateProbe: u8
```

## val GATE_PROBE_COMPTIME_PARAM

```mach
pub val GATE_PROBE_COMPTIME_PARAM:   GateProbe = 0
```

## val GATE_PROBE_LAYOUT_INTRINSIC

```mach
pub val GATE_PROBE_LAYOUT_INTRINSIC: GateProbe = 1
```

## val GATE_PROBE_TYPE_COMPARISON

```mach
pub val GATE_PROBE_TYPE_COMPARISON:  GateProbe = 2
```

## val GATE_PROBE_FIELD_DESCRIPTOR

```mach
pub val GATE_PROBE_FIELD_DESCRIPTOR: GateProbe = 3
```

## val GATE_PROBE_EACH_LOOPVAR

```mach
pub val GATE_PROBE_EACH_LOOPVAR:     GateProbe = 4
```

## val GATE_PROBE_TARGET_DEPENDENT

```mach
pub val GATE_PROBE_TARGET_DEPENDENT: GateProbe = 5
```

## val GATE_PROBE_NODE_ACTION

```mach
pub val GATE_PROBE_NODE_ACTION:      GateProbe = 6
```

## val GATE_PROBE_COUNT

```mach
pub val GATE_PROBE_COUNT: u32 = 5
```

## def GateNodeVerdict

```mach
pub def GateNodeVerdict: u8
```

## val GATE_NODE_NO

```mach
pub val GATE_NODE_NO:    GateNodeVerdict = 0
```

## val GATE_NODE_YES

```mach
pub val GATE_NODE_YES:   GateNodeVerdict = 1
```

## val GATE_NODE_PRUNE

```mach
pub val GATE_NODE_PRUNE: GateNodeVerdict = 2
```

## val GATE_PROBE_UNLISTED_KIND_MSG

```mach
pub val GATE_PROBE_UNLISTED_KIND_MSG: str =
"internal: this expression kind has no entry in the compile-time dependency visitor
```

## val GATE_PROBE_NO_OBSERVER_MSG

```mach
pub val GATE_PROBE_NO_OBSERVER_MSG: str =
"internal: this phase did not supply the observer a compile-time dependency probe needs"
```

## rec GateProbes

```mach
pub rec GateProbes[T];
```

## fun gate_probes

```mach
pub fun gate_probes[T](
comptime_param: fun(*T, id.ExprId) bool,
each_loopvar: fun(*T, id.ExprId) bool,
field_loopvar: fun(*T, id.ExprId) bool,
field_type_operand: fun(*T, id.ExprId) bool,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) GateProbes[T];
```

## fun gate_node_action_probes

```mach
pub fun gate_node_action_probes[T](action: fun(*T, id.ExprId, *expr.Expr) GateNodeVerdict,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail]) GateProbes[T];
```

## fun gate_walk

```mach
pub fun gate_walk[T](a: *ast.Ast, source: str, obs: *T,
action: fun(*T, id.ExprId, *expr.Expr) GateNodeVerdict,
expression: fun(*T, *ast.Ast, id.ExprId) res[expr.Expr, EvalFail],
eid: id.ExprId) res[bool, fail.Fail];
```

## fun gate_chain_depends_on

```mach
pub fun gate_chain_depends_on[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe,
branches_start: u32, branches_len: u32) res[bool, fail.Fail];
```

## fun gate_needs_typed_selection

```mach
pub fun gate_needs_typed_selection(a: *ast.Ast, source: str, eid: id.ExprId) res[bool, fail.Fail];
```

## def GateScope

```mach
pub def GateScope: u8
```

## val GATE_SCOPE_DECL

```mach
pub val GATE_SCOPE_DECL:     GateScope = 0
```

## val GATE_SCOPE_INSTANCE

```mach
pub val GATE_SCOPE_INSTANCE: GateScope = 1
```

## val GATE_U8_MSG

```mach
pub val GATE_U8_MSG: str =
"comptime branch condition must have type u8"
```

## val GATE_U8_FLOAT_MSG

```mach
pub val GATE_U8_FLOAT_MSG: str =
"comptime branch condition must have type u8, found float"
```

## val GATE_U8_STR_MSG

```mach
pub val GATE_U8_STR_MSG: str =
"comptime branch condition must have type u8, found str"
```

## rec GateVerdict

```mach
pub rec GateVerdict;
```

## fun gate_unresolved_dependency

```mach
pub fun gate_unresolved_dependency[T](c: *ComptimeCtx, cap_ctx: *T, caps: PhaseCapabilities[T],
a: *ast.Ast, source: str, interner: *intern.Interner, cond: id.ExprId) res[intern.StrId, fail.Fail];
```

## fun evaluate_gate

```mach
pub fun evaluate_gate[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
cond: id.ExprId,
interner: *intern.Interner,
scope: GateScope,
cache: bool,
terminal: bool) res[GateVerdict, fail.Fail];
```

## fun gate_chain_defers

```mach
pub fun gate_chain_defers[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T],
branches_start: u32, branches_len: u32) res[bool, fail.Fail];
```

## fun gate_depends_on

```mach
pub fun gate_depends_on[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe, eid: id.ExprId) res[bool, fail.Fail];
```

## fun is_layout_intrinsic_call

```mach
pub fun is_layout_intrinsic_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_length_of_call

```mach
pub fun is_length_of_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_offset_of_call

```mach
pub fun is_offset_of_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_size_of_call

```mach
pub fun is_size_of_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_align_of_call

```mach
pub fun is_align_of_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun layout_intrinsic_type_arg

```mach
pub fun layout_intrinsic_type_arg(a: *ast.Ast, eid: id.ExprId) id.ExprId;
```

## fun fields_type_arg

```mach
pub fun fields_type_arg(a: *ast.Ast, eid: id.ExprId) id.ExprId;
```

## rec ArrayLitInfo

```mach
pub rec ArrayLitInfo;
```

## fun declared_float_width

```mach
pub fun declared_float_width(a: *ast.Ast, source: str, t: id.TypeId) float.FloatWidth;
```

## fun apply_declared_int_type

```mach
pub fun apply_declared_int_type(a: *ast.Ast, source: str, t: id.TypeId, value: CTValue) CTValue;
```

## fun float_width_unread

```mach
pub fun float_width_unread(v: CTValue) bool;
```

## fun decl_array_lit

```mach
pub fun decl_array_lit(a: *ast.Ast, decl_id: id.DeclId) opt[ArrayLitInfo];
```

## fun is_error_call

```mach
pub fun is_error_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun error_message

```mach
pub fun error_message[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
eid: id.ExprId,
interner: *intern.Interner) res[str, EvalFail];
```

## fun type_operand_value

```mach
pub fun type_operand_value(a: *ast.Ast, source: str, operand: id.ExprId) id.ExprId;
```

## fun is_type_comparison

```mach
pub fun is_type_comparison(a: *ast.Ast, source: str, bin: *expr.ExprBinary) bool;
```

## fun is_type_comparison_binary

```mach
pub fun is_type_comparison_binary(a: *ast.Ast, source: str, bin: *expr.ExprBinary,
lhs_is_field_type: bool, rhs_is_field_type: bool) bool;
```

## fun is_field_type_member

```mach
pub fun is_field_type_member(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_field_descriptor_member

```mach
pub fun is_field_descriptor_member(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_type_query_call

```mach
pub fun is_type_query_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun is_type_name_call

```mach
pub fun is_type_name_call(a: *ast.Ast, source: str, eid: id.ExprId) bool;
```

## fun ct_is_negative

```mach
pub fun ct_is_negative(v: CTValue) bool;
```

## fun is_case_literal

```mach
pub fun is_case_literal(a: *ast.Ast, eid: id.ExprId) bool;
```

a literal is a case literal when its head names a tag case, `T.c{...}` or `T.[c]{...}`; the
parser records the case only for a head with generic arguments, name resolution splits the
others, so the answer is complete once the literal's head has been bound

## fun is_comptime_path

```mach
pub fun is_comptime_path(a: *ast.Ast, e: id.ExprId) bool;
```

## val REMOVED_PROJECT_NAME_MSG

```mach
pub val REMOVED_PROJECT_NAME_MSG: str = "`$project.name` was removed in 5.0.0 with the `[project] name` manifest key
```

the two paths read manifest keys that were never used; they went with the keys
and are refused by name rather than as unknown paths (#3128)

## val REMOVED_PROJECT_DESC_MSG

```mach
pub val REMOVED_PROJECT_DESC_MSG: str = "`$project.description` was removed in 5.0.0 with the `[project] description` manifest key
```

## fun comptime_ident_name

```mach
pub fun comptime_ident_name(full: token.Span) token.Span;
```

## val MODE_DEBUG

```mach
pub val MODE_DEBUG:   u32 = 0
```

## val MODE_RELEASE

```mach
pub val MODE_RELEASE: u32 = 1
```

## fun ct_float

```mach
pub fun ct_float(f: f64, w: float.FloatWidth) CTValue;
```

## fun field_value

```mach
pub fun field_value(owner: u32, index: u32) CTValue;
```

## fun case_value

```mach
pub fun case_value(owner: u32, index: u32) CTValue;
```

## fun pack_elem_value

```mach
pub fun pack_elem_value(index: u32, ty: u32) CTValue;
```

## fun const_elem_untyped

```mach
pub fun const_elem_untyped(elem: u32) CTValue;
```

a constant element whose checked type is not yet known: name resolution and loading bind case
literals this way, and type checking rebinds them with the declared type

## fun const_elem_value

```mach
pub fun const_elem_value(elem: u32, ty: u32) CTValue;
```

## fun ct_str

```mach
pub fun ct_str(s: intern.StrId) CTValue;
```

## fun ct_u8

```mach
pub fun ct_u8(n: u8) CTValue;
```

## fun is_u8

```mach
pub fun is_u8(v: CTValue) bool;
```

## fun truth

```mach
pub fun truth(v: CTValue) bool;
```

## fun ct_type

```mach
pub fun ct_type(t: u32) CTValue;
```

## fun ct_uint

```mach
pub fun ct_uint(n: u64) CTValue;
```

## fun ct_zero_int

```mach
pub fun ct_zero_int(unsigned: bool, width: u8) CTValue;
```

