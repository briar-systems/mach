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

## fun ct_cap

```mach
pub fun ct_cap(op: CtOp) opt[CtCap];
```

the capability a target must provide for an operation to be constant-time,
absent for an operation outside the catalog: an unknown operation is never
answered with "no capability needed"

## fun cap_at_sema

```mach
pub fun cap_at_sema(cap: CtCap) bool;
```

## def CtMulOp

```mach
pub def CtMulOp: u8
```

a multiply `$mach.build.ct_mul` asks about: the low half of a same-width
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

## rec CtMulCell

```mach
pub rec CtMulCell;
```

a multiply at an operand width in bits, `vector` for a lane multiply

## rec CtMulMask

```mach
pub rec CtMulMask;
```

the cells mirl runs in constant time, one bit per (op, width) in a scalar and a vector plane

## fun mul_cell

```mach
pub fun mul_cell(op: CtMulOp, width: u8, vector: bool) CtMulCell;
```

## fun mul_mask_none

```mach
pub fun mul_mask_none() CtMulMask;
```

## fun mul_cell_width_valid

```mach
pub fun mul_cell_width_valid(width: u8) bool;
```

the operand widths a cell may have

## fun mul_op_name

```mach
pub fun mul_op_name(op: CtMulOp) opt[str];
```

the spelling `$mach.build.ct_mul` reads, absent for NONE or an unknown tag

## val MUL_OP_SPELLINGS

```mach
pub val MUL_OP_SPELLINGS: str = "low, high_u, high_s, high_su, wide_u, wide_s"
```

the valid op spellings, for a refusal that lists them

## fun mul_mask_add

```mach
pub fun mul_mask_add(mask: *CtMulMask, cell: CtMulCell);
```

## fun mul_mask_has

```mach
pub fun mul_mask_has(mask: CtMulMask, cell: CtMulCell) bool;
```

whether the cells cover this multiply, never for an unknown cell

