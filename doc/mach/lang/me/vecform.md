# mach.lang.me.vecform

## fun vec_op_of

```mach
pub fun vec_op_of(k: instruction.InstrKind, uniform_count: bool) opt[isa.VecOp];
```

the target-layer vector operation an instruction kind maps to, VEC_OP_NONE
for a kind that is not a vector operator; absent for a descriptor vector tag
outside the catalog, which the caller refuses as undeclared and names. a
shift is keyed by its count's form as well: `uniform_count` is one scalar
count for every lane, and otherwise the count is a vector of lane counts

## fun uniform_count

```mach
pub fun uniform_count(m: *ir.Module, inst: *instruction.Instruction) bool;
```

a vector shift whose count is one scalar rather than a vector of lane counts

## rec LaneDesc

```mach
pub rec LaneDesc;
```

from_bits and from_lanes are the operand's lane width and lane count, which
only a conversion, a widening multiply, a lane-halving extension, a lane
range or a widening group sum changes

## fun lane_desc

```mach
pub fun lane_desc(m: *ir.Module, inst: *instruction.Instruction, out: *LaneDesc) bool;
```

## fun scalar_bits

```mach
pub fun scalar_bits(m: *ir.Module, ty: ir_type.IrTypeId) u32;
```

## fun packs

```mach
pub fun packs(tgt: *target.Target, k: instruction.InstrKind, is_float: bool, lane_bits: u32, uniform_count: bool) bool;
```

whether the target packs `k` over lanes of `lane_bits`; a shift is the cell
of its count's form, one scalar for every lane when `uniform_count` and a
count per lane otherwise

## fun extends

```mach
pub fun extends(tgt: *target.Target, k: instruction.InstrKind, lane_bits: u32, from_bits: u32) bool;
```

a loop's lane-wise integer extension `k` from `from_bits` lanes to
`lane_bits` lanes, packed

## fun extends_directly

```mach
pub fun extends_directly(tgt: *target.Target) bool;
```

a lane-wise integer extension over more than one doubling is one
instruction on the target, rather than a chain of doublings (#4161)

## fun packed_lanes

```mach
pub fun packed_lanes(tgt: *target.Target, lane_bits: u32) u32;
```

## fun decide

```mach
pub fun decide(m: *ir.Module, tgt: *target.Target, inst: *instruction.Instruction) isa.VectorForm;
```

the target's declared outcome for one vector operator: packed, the scalar
expansion, or undeclared when the catalog names neither for its lane shape

## fun piece_lanes

```mach
pub fun piece_lanes(tgt: *target.Target, lane_bits: u32) u32;
```

the lanes of one register-width piece a vector of `lane_bits` lanes wider
than the register is split into (#3589): the target's declared vector width
over the lane, capped by its lane count. 0 where a vector is not split: no
vector register, or a target whose vectors are values rather than registers
(spir-v), which realizes a vector of any declared width whole

## fun splits

```mach
pub fun splits(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId) bool;
```

a vector the target splits at type legalization: wider than one register, of
a lane shape the vocabulary retains, on a target that splits

## fun realizes_packed

```mach
pub fun realizes_packed(m: *ir.Module, tgt: *target.Target, inst: *instruction.Instruction) bool;
```

packed where the catalog packs the cell and both sides are realized in
registers, whole or as the pieces a wider vector is split into, each of which
runs the packed instruction

## fun widening_of

```mach
pub fun widening_of(ext: instruction.InstrKind) opt[instruction.InstrKind];
```

the widening multiply that a multiply of two lane-wise extensions of `ext`
kind fuses into; absent for any other extension

## fun widening_packs

```mach
pub fun widening_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether a widening multiply of two `from_ty` vectors into `ty` is a cell the
target packs at these lanes; the catalog is the only judge

## fun widening_half_packs

```mach
pub fun widening_half_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the widening multiply of one half of two `from_ty` vectors into the
`ty` of half their lanes is a cell the target packs (#3589)

## fun high_of

```mach
pub fun high_of(kind: instruction.InstrKind) opt[instruction.InstrKind];
```

the high multiply that pairs with the plain multiply into the widening
multiply `kind`; absent for any other kind

## fun widening_pair_packs

```mach
pub fun widening_pair_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the target realizes the half `ty` of a widening multiply `kind` of
two `from_ty` vectors as the plain and the high multiply of the operands,
interleaved: every piece of the pair is a cell it packs (#4119). both halves
of one product then read the same pair

## fun sign_interleave_packs

```mach
pub fun sign_interleave_packs(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the target realizes the signed lane-halving extension of a
`from_ty` vector into the `ty` of half its lanes as the vector interleaved
with its sign mask: both are cells it packs, and an extension is not one
instruction of its own there. both halves of one extension then read the
same mask (#4198)

## fun zero_interleave_packs

```mach
pub fun zero_interleave_packs(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the target realizes the unsigned lane-halving extension of a
`from_ty` vector into the `ty` of half its lanes as the vector interleaved
with zero: the interleave is a cell it packs, and an extension is not one
instruction of its own there. every such half then reads one zero (#4198)

## fun concat_packs

```mach
pub fun concat_packs(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId) bool;
```

whether a lane join into `ty` is a cell the target packs (#3589)

## fun widen_half_packs

```mach
pub fun widen_half_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether a lane-halving extension of a `from_ty` vector into `ty` is a cell
the target packs at these lanes; the catalog is the only judge

## fun conversion_packs

```mach
pub fun conversion_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the lane-wise conversion `kind` of a `from_ty` vector into `ty` is a
cell the target packs at these lanes; the catalog is the only judge

## fun operation_packs

```mach
pub fun operation_packs(m: *ir.Module, tgt: *target.Target, kind: instruction.InstrKind, ty: ir_type.IrTypeId, operand_ty: ir_type.IrTypeId, n: u32) bool;
```

whether `kind` over `n` operands of `operand_ty` (one or two) into `ty` is a
cell the target packs at these lanes; the catalog is the only judge

## fun widen_sum_packs

```mach
pub fun widen_sum_packs(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether the widening group sum of a `from_ty` vector into `ty` is a cell
the target packs, both sides whole in one register (#4161)

## fun range_packs

```mach
pub fun range_packs(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId, from_ty: ir_type.IrTypeId) bool;
```

whether a lane range of a `from_ty` vector into `ty` is a cell the target
packs at these lanes; the catalog is the only judge

## fun holds_in_register

```mach
pub fun holds_in_register(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId) bool;
```

## fun vec_op_name

```mach
pub fun vec_op_name(op: isa.VecOp) opt[str];
```

the spelling of a vector operation in a diagnostic; absent for a tag
outside the catalog

