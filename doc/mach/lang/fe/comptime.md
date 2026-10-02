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

## val CT_KIND_CONST_ELEM

```mach
pub val CT_KIND_CONST_ELEM: CTKind = 6
```

## val CT_KIND_CASE

```mach
pub val CT_KIND_CASE: CTKind = 7
```

a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor

## val CT_KIND_NON_INTEGER

```mach
pub val CT_KIND_NON_INTEGER: CTKind = 8
```

an integer constant whose declared type is a def chain ending outside the integers;
data.s is the interned refusal naming the chain, and reading it fails with that refusal

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
pub val GATE_INACTIVE:          GateOutcome = 0
```

## val GATE_ACTIVE

```mach
pub val GATE_ACTIVE:            GateOutcome = 1
```

## val GATE_REJECTED

```mach
pub val GATE_REJECTED:          GateOutcome = 2
```

## val GATE_FAILED

```mach
pub val GATE_FAILED:            GateOutcome = 3
```

## val GATE_AWAITING_PHASE

```mach
pub val GATE_AWAITING_PHASE:    GateOutcome = 4
```

## val GATE_AWAITING_INSTANCE

```mach
pub val GATE_AWAITING_INSTANCE: GateOutcome = 5
```

## rec CTValue

```mach
pub rec CTValue;
```

an integer value is 128 bits in data.w; data.i is its low limb, and a signed
value sign-fills the high limb so every reader of a value that fits 64 bits
sees the same i64 it always did (#3511)

## rec NamedConst

```mach
pub rec NamedConst;
```

## val COMPTIME_TYPE_NEEDS_INSTANCE_MSG

```mach
pub val COMPTIME_TYPE_NEEDS_INSTANCE_MSG: str =
"a type comparison on an unsubstituted generic parameter is only decidable at an instantiation"
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

## val TYPE_QUERY_IS_RECORD

```mach
pub val TYPE_QUERY_IS_RECORD:    u8 = 0
```

## val TYPE_QUERY_IS_UNION

```mach
pub val TYPE_QUERY_IS_UNION:     u8 = 1
```

## val TYPE_QUERY_IS_POINTER

```mach
pub val TYPE_QUERY_IS_POINTER:   u8 = 2
```

## val TYPE_QUERY_NAME

```mach
pub val TYPE_QUERY_NAME:         u8 = 3
```

## val TYPE_QUERY_IS_SECRET

```mach
pub val TYPE_QUERY_IS_SECRET:    u8 = 4
```

## val TYPE_QUERY_IS_TAG

```mach
pub val TYPE_QUERY_IS_TAG:       u8 = 5
```

## val TYPE_QUERY_IS_INTEGER

```mach
pub val TYPE_QUERY_IS_INTEGER:   u8 = 6
```

## val TYPE_QUERY_IS_FLOAT

```mach
pub val TYPE_QUERY_IS_FLOAT:     u8 = 7
```

## val TYPE_QUERY_HOLDS_SECRET

```mach
pub val TYPE_QUERY_HOLDS_SECRET: u8 = 8
```

## rec NoCapabilityContext

```mach
pub rec NoCapabilityContext;
```

the context of an evaluation that holds no phase capability

## rec PhaseCapabilities

```mach
pub rec PhaseCapabilities[T];
```

## fun no_capabilities

```mach
pub fun no_capabilities() PhaseCapabilities[NoCapabilityContext];
```

no phase capability: an expression reads literals and the constants its context binds

## fun loading_capabilities

```mach
pub fun loading_capabilities[T](member: fun(*T, ast_id.ExprId) res[opt[CTValue], EvalFail],
cast: fun(*T, ast_id.ExprId, CTValue) res[CTValue, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun resolution_capabilities

```mach
pub fun resolution_capabilities[T](ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_name_capabilities

```mach
pub fun semantic_name_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[CTValue], EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun semantic_type_capabilities

```mach
pub fun semantic_type_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[CTValue], EvalFail],
type_: fun(*T, ast_id.ExprId) res[opt[u32], EvalFail],
field: fun(*T, u32, u32, u8) res[opt[CTValue], EvalFail],
query: fun(*T, u32, u8) res[opt[CTValue], EvalFail],
layout: fun(*T, u32) res[opt[CTValue], EvalFail],
cast: fun(*T, ast_id.ExprId, CTValue) res[CTValue, EvalFail],
scalar: fun(*T, ast_id.ExprId, CTValue) res[CTValue, EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
```

## fun lowering_capabilities

```mach
pub fun lowering_capabilities[T](
member: fun(*T, ast_id.ExprId) res[opt[CTValue], EvalFail],
type_: fun(*T, ast_id.ExprId) res[opt[u32], EvalFail],
field: fun(*T, u32, u32, u8) res[opt[CTValue], EvalFail],
query: fun(*T, u32, u8) res[opt[CTValue], EvalFail],
layout: fun(*T, u32) res[opt[CTValue], EvalFail],
cast: fun(*T, ast_id.ExprId, CTValue) res[CTValue, EvalFail],
scalar: fun(*T, ast_id.ExprId, CTValue) res[CTValue, EvalFail],
ident: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail]) res[PhaseCapabilities[T], fail.Fail];
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

## def EvalFailKind

```mach
pub def EvalFailKind: u8
```

## val EVAL_FAIL_REJECTED

```mach
pub val EVAL_FAIL_REJECTED:       EvalFailKind = 1
```

## val EVAL_FAIL_UNBOUND

```mach
pub val EVAL_FAIL_UNBOUND:        EvalFailKind = 2
```

## val EVAL_FAIL_NEEDS_MEMBER

```mach
pub val EVAL_FAIL_NEEDS_MEMBER:   EvalFailKind = 3
```

## val EVAL_FAIL_NEEDS_TYPES

```mach
pub val EVAL_FAIL_NEEDS_TYPES:    EvalFailKind = 4
```

## val EVAL_FAIL_NEEDS_LAYOUT

```mach
pub val EVAL_FAIL_NEEDS_LAYOUT:   EvalFailKind = 5
```

## val EVAL_FAIL_INTERNAL

```mach
pub val EVAL_FAIL_INTERNAL:       EvalFailKind = 6
```

## val EVAL_FAIL_NEEDS_INSTANCE

```mach
pub val EVAL_FAIL_NEEDS_INSTANCE: EvalFailKind = 7
```

## rec EvalFail

```mach
pub rec EvalFail;
```

diag: the diagnostic kind the failure is reported as, where it reaches the user

## fun eval_error

```mach
pub fun eval_error(kind: EvalFailKind, diag: diagnostic_kind.Kind, message: str) EvalFail;
```

## fun eval_from_fail

```mach
pub fun eval_from_fail(f: fail.Fail, diag: diagnostic_kind.Kind, rejected_message: str) EvalFail;
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
register_bits: u32,
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

## fun set_nan_rule

```mach
pub fun set_nan_rule(c: *ComptimeCtx, rule: float.NanRule);
```

## fun set_ct_mul

```mach
pub fun set_ct_mul(c: *ComptimeCtx, mask: ct.CtMulMask);
```

## fun set_extensions

```mach
pub fun set_extensions(c: *ComptimeCtx, view: isa.ExtensionView);
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

## fun set_source_context

```mach
pub fun set_source_context(c: *ComptimeCtx, module: intern.StrId, file: intern.StrId,
owner_id: intern.StrId, owner_ver: intern.StrId);
```

the module a context compiles, what `$mach.source.*` and `$mach.project.*` read

module: the module's fully qualified name
file: its file, relative to the root of the project that owns it
owner_id: the owning project's `[project].id`
owner_ver: the owning project's `[project].version`

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
pub fun gate_state(c: *ComptimeCtx, eid: ast_id.ExprId) GateState;
```

## fun set_gate_state

```mach
pub fun set_gate_state(c: *ComptimeCtx, eid: ast_id.ExprId, state: GateState);
```

## fun load_walked

```mach
pub fun load_walked(c: *ComptimeCtx, did: ast_id.DeclId) bool;
```

## fun mark_load_walked

```mach
pub fun mark_load_walked(c: *ComptimeCtx, did: ast_id.DeclId);
```

an unbound mark is the stronger fact and survives the walk's own mark

## fun use_unbound

```mach
pub fun use_unbound(c: *ComptimeCtx, did: ast_id.DeclId) bool;
```

## fun mark_use_unbound

```mach
pub fun mark_use_unbound(c: *ComptimeCtx, did: ast_id.DeclId);
```

## fun defer_float_width

```mach
pub fun defer_float_width(c: *ComptimeCtx, name: intern.StrId) err[fail.Fail];
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
e: ast_id.ExprId,
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
e: ast_id.ExprId,
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
pub fun scan_lit_int(source: str, span: token.Span) res[LitInt, EvalFail];
```

## fun scan_lit_float

```mach
pub fun scan_lit_float(source: str, span: token.Span) res[LitFloat, EvalFail];
```

a literal's value at the width its suffix names, binary64 when it has none

## fun lit_float_at

```mach
pub fun lit_float_at(source: str, span: token.Span, w: float.FloatWidth) res[float.Rounded, fail.Fail];
```

a literal rounded once its type is known: at its suffix's width when it has one, else at
`w`, the width its context gives it. every consumer of a literal's value, the parser, the
comptime evaluator, lowering, and the rule sema applies, reads it from here

source: the text the span indexes
span: the literal's token
w: the width of the literal's type
ret: the rounded value and how it fit, or the scan's refusal

## fun lit_float_spells

```mach
pub fun lit_float_spells(source: str, span: token.Span, s: *float.Shortest) bool;
```

true when the literal's significant digits, leading and trailing zeros and exponent
spelling aside, are exactly the shortest decimal that rounds back to `s`

## fun eval_lit_char

```mach
pub fun eval_lit_char(source: str, span: token.Span) res[CTValue, EvalFail];
```

## fun cast_scalar

```mach
pub fun cast_scalar(types: *type.TypeInterner, pointer_width: u32, nan_rule: float.NanRule, value: CTValue,
from_ty: type.TypeId, to_ty: type.TypeId, reinterpret: bool) res[CTValue, EvalFail];
```

a float converted to another float width folds by `nan_rule`, the target's, so the
folded conversion is the one the target makes at run time

## fun intrinsic_takes_type_operand

```mach
pub fun intrinsic_takes_type_operand(source: str, full: token.Span) bool;
```

## fun is_type_of_call

```mach
pub fun is_type_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun type_of_arg

```mach
pub fun type_of_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun is_fields_call

```mach
pub fun is_fields_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_cases_call

```mach
pub fun is_cases_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

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

## val GATE_PROBE_TYPE_COMPARISON

```mach
pub val GATE_PROBE_TYPE_COMPARISON:  GateProbe = 2
```

## def GateNodeVerdict

```mach
pub def GateNodeVerdict: u8
```

## val GATE_NODE_NO

```mach
pub val GATE_NODE_NO:    GateNodeVerdict = 0
```

## val GATE_NODE_PRUNE

```mach
pub val GATE_NODE_PRUNE: GateNodeVerdict = 2
```

## rec GateProbes

```mach
pub rec GateProbes[T];
```

## fun gate_probes

```mach
pub fun gate_probes[T](
comptime_param: fun(*T, ast_id.ExprId) bool,
each_loopvar: fun(*T, ast_id.ExprId) bool,
field_loopvar: fun(*T, ast_id.ExprId) bool,
field_type_operand: fun(*T, ast_id.ExprId) bool,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail]) GateProbes[T];
```

## fun gate_walk

```mach
pub fun gate_walk[T](a: *ast.Ast, source: str, obs: *T,
action: fun(*T, ast_id.ExprId, *ast_expr.Expr) GateNodeVerdict,
expression: fun(*T, *ast.Ast, ast_id.ExprId) res[ast_expr.Expr, EvalFail],
eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun gate_chain_depends_on

```mach
pub fun gate_chain_depends_on[T](
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe,
branches_start: u32, branches_len: u32) res[bool, fail.Fail];
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

## rec GateVerdict

```mach
pub rec GateVerdict;
```

## fun evaluate_gate

```mach
pub fun evaluate_gate[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
cond: ast_id.ExprId,
interner: *intern.Interner,
scope: GateScope,
cache: bool) res[GateVerdict, fail.Fail];
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
a: *ast.Ast, source: str, obs: *T, probes: GateProbes[T], probe: GateProbe, eid: ast_id.ExprId) res[bool, fail.Fail];
```

## fun is_layout_intrinsic_call

```mach
pub fun is_layout_intrinsic_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_type_id_call

```mach
pub fun is_type_id_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_length_of_call

```mach
pub fun is_length_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_offset_of_call

```mach
pub fun is_offset_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_size_of_call

```mach
pub fun is_size_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_align_of_call

```mach
pub fun is_align_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun layout_intrinsic_type_arg

```mach
pub fun layout_intrinsic_type_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun fields_type_arg

```mach
pub fun fields_type_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## rec ArrayLitInfo

```mach
pub rec ArrayLitInfo;
```

## fun declared_float_width

```mach
pub fun declared_float_width(a: *ast.Ast, source: str, t: ast_id.TypeId) float.FloatWidth;
```

## fun apply_declared_int_type

```mach
pub fun apply_declared_int_type(a: *ast.Ast, source: str, t: ast_id.TypeId, value: CTValue) CTValue;
```

## fun cast_to_int

```mach
pub fun cast_to_int(value: CTValue, width: u32, signed: bool) res[CTValue, EvalFail];
```

an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign,
an untyped one as it stands, then the result is cut or extended to the destination

## fun typed_int

```mach
pub fun typed_int(value: CTValue, unsigned: bool, width: u8) CTValue;
```

## fun float_width_unread

```mach
pub fun float_width_unread(v: CTValue) bool;
```

## fun decl_array_lit

```mach
pub fun decl_array_lit(a: *ast.Ast, decl_id: ast_id.DeclId) opt[ArrayLitInfo];
```

## fun is_error_call

```mach
pub fun is_error_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun error_message

```mach
pub fun error_message[T](
c: *ComptimeCtx,
cap_ctx: *T,
caps: PhaseCapabilities[T],
a: *ast.Ast,
source: str,
eid: ast_id.ExprId,
interner: *intern.Interner) res[str, EvalFail];
```

## fun is_type_comparison_binary

```mach
pub fun is_type_comparison_binary(a: *ast.Ast, source: str, bin: *ast_expr.ExprBinary,
lhs_is_field_type: bool, rhs_is_field_type: bool) bool;
```

## fun is_field_type_member

```mach
pub fun is_field_type_member(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_type_query_call

```mach
pub fun is_type_query_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun type_question

```mach
pub fun type_question(a: *ast.Ast, source: str, eid: ast_id.ExprId) res[ast_id.ExprId, fail.Fail];
```

the first call in an expression that asks a type question, EXPR_NIL when none does

## fun type_question_message

```mach
pub fun type_question_message(interner: *intern.Interner, a: *ast.Ast, source: str, call: ast_id.ExprId, through: str) res[intern.StrId, fail.Fail];
```

a declaring gate may not ask a type question; `through` names the constants it asks it through,
as ` through `T` -> `S``, or is empty when the gate asks it itself

## fun ct_is_negative

```mach
pub fun ct_is_negative(v: CTValue) bool;
```

## fun ct_int_u64

```mach
pub fun ct_int_u64(v: CTValue, out: *u64) bool;
```

an integer value that is neither negative nor above u64, read as a u64

## fun ct_int_text

```mach
pub fun ct_int_text(v: CTValue, buf: *u8) str;
```

the decimal text of an integer value in a wide.FORMAT_CAP buffer

## fun non_integer

```mach
pub fun non_integer(message: intern.StrId) CTValue;
```

## fun is_case_literal

```mach
pub fun is_case_literal(a: *ast.Ast, eid: ast_id.ExprId) bool;
```

a literal is a case literal when its head names a tag case, `T.c{...}` or `T.[c]{...}`; the
parser records the case only for a head with generic arguments, name resolution splits the
others, so the answer is complete once the literal's head has been bound

## fun is_path_call

```mach
pub fun is_path_call(a: *ast.Ast, e: ast_id.ExprId) bool;
```

a call whose callee is a rooted comptime path: `$mach.build.ct_mul(low, 64)`

## fun is_comptime_value

```mach
pub fun is_comptime_value(a: *ast.Ast, e: ast_id.ExprId) bool;
```

a rooted comptime path, or a call on one: both fold to a constant

## fun is_comptime_path

```mach
pub fun is_comptime_path(a: *ast.Ast, e: ast_id.ExprId) bool;
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

