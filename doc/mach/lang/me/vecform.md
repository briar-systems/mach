# mach.lang.me.vecform

## fun vec_op_of

```mach
pub fun vec_op_of(k: instruction.InstrKind) opt[isa.VecOp];
```

the target-layer vector operation an instruction kind maps to, VEC_OP_NONE
for a kind that is not a vector operator; absent for a descriptor vector tag
outside the catalog, which the caller refuses as undeclared and names

## rec LaneDesc

```mach
pub rec LaneDesc;
```

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
pub fun packs(tgt: *target.Target, k: instruction.InstrKind, is_float: bool, lane_bits: u32) bool;
```

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

## fun realizes_packed

```mach
pub fun realizes_packed(m: *ir.Module, tgt: *target.Target, inst: *instruction.Instruction) bool;
```

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

