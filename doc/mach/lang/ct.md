# mach.lang.ct

## def CtOp

```mach
pub def CtOp: u8
```

## val CT_OP_NONE

```mach
pub val CT_OP_NONE:       CtOp = 0
```

## val CT_OP_INT_DIVMOD

```mach
pub val CT_OP_INT_DIVMOD: CtOp = 1
```

## val CT_OP_FLOAT

```mach
pub val CT_OP_FLOAT:      CtOp = 2
```

## val CT_OP_INT_MUL

```mach
pub val CT_OP_INT_MUL:    CtOp = 3
```

## val CT_OP_VAR_SHIFT

```mach
pub val CT_OP_VAR_SHIFT:  CtOp = 4
```

## def CtCap

```mach
pub def CtCap: u8
```

## val CT_CAP_OK

```mach
pub val CT_CAP_OK:        CtCap = 0
```

## val CT_CAP_ALWAYS

```mach
pub val CT_CAP_ALWAYS:    CtCap = 1
```

## val CT_CAP_MUL

```mach
pub val CT_CAP_MUL:       CtCap = 2
```

## val CT_CAP_VAR_SHIFT

```mach
pub val CT_CAP_VAR_SHIFT: CtCap = 3
```

## val CT_OP_COUNT

```mach
pub val CT_OP_COUNT: u32 = 5
```

## fun ct_op_valid

```mach
pub fun ct_op_valid(op: CtOp) bool;
```

## fun ct_cap

```mach
pub fun ct_cap(op: CtOp) opt[CtCap];
```

the capability a target must provide for an operation to be constant-time,
absent for an operation outside the catalog: an unknown operation is never
answered with "no capability needed"

## fun ct_cap_or_always

```mach
pub fun ct_cap_or_always(op: CtOp) CtCap;
```

tests and the sema gate: the declared capability, or the one no target
provides for an operation outside the catalog

## fun cap_at_sema

```mach
pub fun cap_at_sema(cap: CtCap) bool;
```

## fun cap_at_mir

```mach
pub fun cap_at_mir(cap: CtCap) bool;
```

## def CtMulOp

```mach
pub def CtMulOp: u8
```

the multiply a machine instruction realizes: the low half of a same-width
product, the high half by operand signedness, or the full double-width
product

## val CT_MUL_NONE

```mach
pub val CT_MUL_NONE:    CtMulOp = 0
```

## val CT_MUL_LOW

```mach
pub val CT_MUL_LOW:     CtMulOp = 1
```

## val CT_MUL_HIGH_U

```mach
pub val CT_MUL_HIGH_U:  CtMulOp = 2
```

## val CT_MUL_HIGH_S

```mach
pub val CT_MUL_HIGH_S:  CtMulOp = 3
```

## val CT_MUL_HIGH_SU

```mach
pub val CT_MUL_HIGH_SU: CtMulOp = 4
```

## val CT_MUL_WIDE_U

```mach
pub val CT_MUL_WIDE_U:  CtMulOp = 5
```

## val CT_MUL_WIDE_S

```mach
pub val CT_MUL_WIDE_S:  CtMulOp = 6
```

## val CT_MUL_OP_COUNT

```mach
pub val CT_MUL_OP_COUNT: u32 = 7
```

## def CtMulCond

```mach
pub def CtMulCond: u8
```

what a declared multiply row needs before its timing is data-independent

## val CT_MUL_ALWAYS

```mach
pub val CT_MUL_ALWAYS:    CtMulCond = 0
```

## val CT_MUL_DIT_MODE

```mach
pub val CT_MUL_DIT_MODE:  CtMulCond = 1
```

## val CT_MUL_EXTENSION

```mach
pub val CT_MUL_EXTENSION: CtMulCond = 2
```

## val CT_MUL_COND_COUNT

```mach
pub val CT_MUL_COND_COUNT: u32 = 3
```

## val DIT_NEED_MARKER

```mach
pub val DIT_NEED_MARKER:   str = "__mach_needs_dit"
```

the DIT runtime contract between the compiler and the std start code (#3508).
a module that multiplies a secret in a DIT_MODE cell carries the marker: a
local absolute symbol, value 1, that names the need and reaches no linked
image. on a target whose os declares the mode guaranteed, the linker defines
the cell in every executable: a hidden one-byte read-only object, 1 when any
input carries the marker, else 0. the start code reads the cell and, when it
is set, turns PSTATE.DIT on or fails closed. the names and the byte are
frozen: std builds against them

## val DIT_REQUIRED_CELL

```mach
pub val DIT_REQUIRED_CELL: str = "__mach_dit_required"
```

## val DIT_NEED_VALUE

```mach
pub val DIT_NEED_VALUE:    u64 = 1
```

## rec CtMulForm

```mach
pub rec CtMulForm;
```

one declared row: this multiply at this operand width (and, for a lane
multiply, this lane count) has data-independent timing under `cond`; `ext`
names the ISA extension bits an EXTENSION row needs

## rec CtMulCell

```mach
pub rec CtMulCell;
```

the multiply an emitted instruction realizes, at an operand width in bits;
`vector` for a lane multiply

## rec CtMulMask

```mach
pub rec CtMulMask;
```

the admitted cells, one bit per (op, width) in a scalar and a vector plane

## fun mul_cell

```mach
pub fun mul_cell(op: CtMulOp, width: u8, vector: bool) CtMulCell;
```

## fun mul_mask_none

```mach
pub fun mul_mask_none() CtMulMask;
```

## fun mul_op_valid

```mach
pub fun mul_op_valid(op: CtMulOp) bool;
```

## fun mul_width_valid

```mach
pub fun mul_width_valid(width: u8) bool;
```

the operand widths a target may declare a row at

## fun mul_cell_width_valid

```mach
pub fun mul_cell_width_valid(width: u8) bool;
```

the operand widths a cell may have: a 128-bit cell is never declared, it is
admitted through the cells that realize it (#3511)

## fun mul_cond_valid

```mach
pub fun mul_cond_valid(cond: CtMulCond) bool;
```

## fun mul_op_name

```mach
pub fun mul_op_name(op: CtMulOp) opt[str];
```

the spelling `$mach.build.ct_mul` reads, absent for NONE or an unknown tag

## fun mul_op_by_name

```mach
pub fun mul_op_by_name(name: str) opt[CtMulOp];
```

## val MUL_OP_SPELLINGS

```mach
pub val MUL_OP_SPELLINGS: str = "low, high_u, high_s, high_su, wide_u, wide_s"
```

the valid op spellings, for a refusal that lists them

## fun mul_mask_add

```mach
pub fun mul_mask_add(mask: *CtMulMask, cell: CtMulCell);
```

## fun mul_mask_equals

```mach
pub fun mul_mask_equals(a: CtMulMask, b: CtMulMask) bool;
```

## fun mul_mask_has

```mach
pub fun mul_mask_has(mask: CtMulMask, cell: CtMulCell) bool;
```

whether the admitted cells cover this multiply; an unknown cell never is

## fun mul_form_refusal

```mach
pub fun mul_form_refusal(form: *CtMulForm, ext_domain: u64) opt[str];
```

why a declared row is malformed, absent for a well-formed one; `ext_domain`
is every extension bit the ISA models

## fun mul_forms_refusal

```mach
pub fun mul_forms_refusal(forms: *CtMulForm, len: u32, ext_domain: u64, has_mul: bool) opt[str];
```

why a row table is malformed: a malformed row, a cell declared twice, or any
row on a machine with no multiply

## fun target_provides

```mach
pub fun target_provides(mul: CtMulMask, cell: CtMulCell, trust_var_shift: bool, cap: CtCap) bool;
```

whether a target covers a capability; the multiply capability asks for the
exact cell the site emits. false for every other capability, which no
target provides (a partition, enumerated by its test)

## fun ct_op_gates_count_only

```mach
pub fun ct_op_gates_count_only(op: CtOp) bool;
```

## rec BindSecrecy

```mach
pub rec BindSecrecy;
```

the two secrecy facts an inline-asm binding carries into the constant-time scan.
`value` when the binding's own storage is secret (`^usize`, `^*T`): the register it is
staged into holds a secret, so it is refused as an address. `pointee` when the binding is
a pointer whose target reaches a secret (`*^T`): the register holds a public address, and a
load through it produces a secret. a pointer to a secret is a public address and a secret
load; a secret pointer is a secret address

## fun bind_secrecy

```mach
pub fun bind_secrecy(value: bool, pointee: bool) BindSecrecy;
```

## fun bind_public

```mach
pub fun bind_public() BindSecrecy;
```

## fun bind_tracked

```mach
pub fun bind_tracked(b: BindSecrecy) bool;
```

true when the scan has anything to track for the binding

## rec AsmSecret

```mach
pub rec AsmSecret;
```

a binding the scan tracks, by the name its `{name}` operand spells

## fun asm_secret

```mach
pub fun asm_secret(name: str, secrecy: BindSecrecy) AsmSecret;
```

## fun asm_secret_value

```mach
pub fun asm_secret_value(name: str) AsmSecret;
```

## fun asm_secret_pointee

```mach
pub fun asm_secret_pointee(name: str) AsmSecret;
```

## rec AsmClass

```mach
pub rec AsmClass;
```

## fun asm_unknown

```mach
pub fun asm_unknown() AsmClass;
```

## fun flags_untouched

```mach
pub fun flags_untouched(a: AsmClass) AsmClass;
```

## fun flags_defined

```mach
pub fun flags_defined(a: AsmClass) AsmClass;
```

## fun flags_written

```mach
pub fun flags_written(a: AsmClass) AsmClass;
```

## fun flags_opaque

```mach
pub fun flags_opaque(a: AsmClass) AsmClass;
```

## fun flags_read

```mach
pub fun flags_read(a: AsmClass) AsmClass;
```

## fun asm_op

```mach
pub fun asm_op(op: CtOp) AsmClass;
```

## fun asm_mul

```mach
pub fun asm_mul(mul: CtMulOp) AsmClass;
```

a multiply instruction realizing `mul` at its first operand's width

## fun asm_mul_forms

```mach
pub fun asm_mul_forms(mul: CtMulOp, one: CtMulOp) AsmClass;
```

a multiply realizing `mul` with two or more explicit operands and `one` with
a single one

## fun asm_mul_lanes

```mach
pub fun asm_mul_lanes(mul: CtMulOp, lane_bits: u8) AsmClass;
```

a lane multiply at a fixed lane width

## fun asm_mul_width

```mach
pub fun asm_mul_width(mul: CtMulOp, bits: u8) AsmClass;
```

a fixed-width multiply, whatever its operands name

## fun asm_mul_cell

```mach
pub fun asm_mul_cell(cls: AsmClass, explicit_ops: u32, op0_bytes: u8) CtMulCell;
```

the cell an emitted or assembled multiply realizes, from its class, its
count of explicit operands and its first operand's byte width

## fun asm_branch_flags

```mach
pub fun asm_branch_flags() AsmClass;
```

## fun asm_branch_reg

```mach
pub fun asm_branch_reg() AsmClass;
```

## fun untouched_branch_reg

```mach
pub fun untouched_branch_reg() AsmClass;
```

## def OperandRole

```mach
pub def OperandRole: u8
```

the role an emitted instruction gives one of its operand positions
(isa.Inst dst, src1, src2, src3), the part of the effect description that
ct_class does not carry: which registers a notification defines and which
it only reads, and whether a memory operand is loaded, stored, or only its
address computed

## val ROLE_NONE

```mach
pub val ROLE_NONE:  OperandRole = 0
```

## val ROLE_READ

```mach
pub val ROLE_READ:  OperandRole = 1
```

## val ROLE_WRITE

```mach
pub val ROLE_WRITE: OperandRole = 2
```

## val ROLE_RW

```mach
pub val ROLE_RW:    OperandRole = 3
```

## val ROLE_ADDR

```mach
pub val ROLE_ADDR: OperandRole = 4
```

the address registers are read as values and no memory is accessed (lea)

## val INST_OPERANDS

```mach
pub val INST_OPERANDS: u32 = 4
```

## def Transfer

```mach
pub def Transfer: u8
```

## val XFER_NONE

```mach
pub val XFER_NONE: Transfer = 0
```

## val XFER_BRANCH

```mach
pub val XFER_BRANCH: Transfer = 1
```

conditional: may fall through

## val XFER_JUMP

```mach
pub val XFER_JUMP: Transfer = 2
```

unconditional: never falls through

## val XFER_CALL

```mach
pub val XFER_CALL: Transfer = 3
```

falls through after the callee returns

## val XFER_RET

```mach
pub val XFER_RET:  Transfer = 4
```

## val XFER_TRAP

```mach
pub val XFER_TRAP: Transfer = 5
```

## def TargetKind

```mach
pub def TargetKind: u8
```

## val TARGET_NONE

```mach
pub val TARGET_NONE: TargetKind = 0
```

## val TARGET_BLOCK

```mach
pub val TARGET_BLOCK: TargetKind = 1
```

a basic block of the same function

## val TARGET_LOCAL

```mach
pub val TARGET_LOCAL:    TargetKind = 2
```

an offset inside the same expansion or inline-asm body, backward or of
unknown direction

## val TARGET_EXTERNAL

```mach
pub val TARGET_EXTERNAL: TargetKind = 3
```

## val TARGET_REGISTER

```mach
pub val TARGET_REGISTER: TargetKind = 4
```

through a register operand

## val TARGET_LOCAL_FWD

```mach
pub val TARGET_LOCAL_FWD: TargetKind = 5
```

an offset inside the same expansion, known to lie ahead

## rec InstEffects

```mach
pub rec InstEffects;
```

the complete effect description of one emitted instruction: the closed
class table's row plus the operand roles and the implicit register and
memory effects. masks are over general-purpose register indexes

## fun inst_effects_unknown

```mach
pub fun inst_effects_unknown() InstEffects;
```

## fun reg_bit

```mach
pub fun reg_bit(index: i32) u64;
```

