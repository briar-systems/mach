[mach](../README.md) / [target](README.md) / **ct**

# mach.target.ct

what a target must provide for an operation on secret data to run in constant time

type checking classifies the operator of a secret operand (`CtOp`) and reports one no target
runs in constant time, division, remainder or a float operation. a multiply is asked about by
cell, an operation at an operand width, and a target's facts hold the cells mirl runs in
constant time as a mask, which `$mach.build.ct_mul` reads.

Source: [`src/target/ct.mach`](../../../src/target/ct.mach)

| Type | | |
| :--- | :--- | :--- |
| [`CtOp`](#ctop) | `def` |  |
| [`CtCap`](#ctcap) | `def` |  |
| [`CtMulOp`](#ctmulop) | `def` | a multiply `$mach.build.ct_mul` asks about: the low half of a same-width product, the high half by operand signedness, or the full double-width product |
| [`CtMulCell`](#ctmulcell) | `rec` | a multiply at an operand width in bits, `vector` for a lane multiply |
| [`CtMulMask`](#ctmulmask) | `rec` | the cells mirl runs in constant time, one bit per (op, width) in a scalar and a vector plane |

| On `CtOp` | |
| :--- | :--- |
| [`CT_OP_NONE`](#ctop), [`CT_OP_INT_DIVMOD`](#ctop), [`CT_OP_FLOAT`](#ctop), [`CT_OP_INT_MUL`](#ctop), [`CT_OP_VAR_SHIFT`](#ctop) | |
| [`ct_cap`](#ct_cap) | the capability a target must provide for an operation to be constant-time, absent for an operation outside the catalog: an unknown operation is never answered with "no capability needed" |

| On `CtCap` | |
| :--- | :--- |
| [`CT_CAP_OK`](#ctcap), [`CT_CAP_ALWAYS`](#ctcap), [`CT_CAP_MUL`](#ctcap) | |
| [`cap_at_sema`](#cap_at_sema) | whether type checking reports an operation needing `cap` itself, since no target provides it |

| On `CtMulOp` | |
| :--- | :--- |
| [`CT_MUL_NONE`](#ctmulop), [`CT_MUL_LOW`](#ctmulop), [`CT_MUL_HIGH_U`](#ctmulop), [`CT_MUL_HIGH_S`](#ctmulop), [`CT_MUL_HIGH_SU`](#ctmulop), [`CT_MUL_WIDE_U`](#ctmulop), [`CT_MUL_WIDE_S`](#ctmulop) | |
| [`mul_cell`](#mul_cell) | the cell of a multiply |
| [`mul_op_name`](#mul_op_name) | the spelling `$mach.build.ct_mul` reads, absent for NONE or an unknown tag |

| On `CtMulMask` | |
| :--- | :--- |
| [`mul_mask_none`](#mul_mask_none) | a mask holding no cell |
| [`mul_mask_add`](#mul_mask_add) | add a cell to a mask, in its vector plane for a lane multiply |
| [`mul_mask_has`](#mul_mask_has) | whether the cells cover this multiply, never for an unknown cell |

| Functions | |
| :--- | :--- |
| [`mul_cell_width_valid`](#mul_cell_width_valid) | the operand widths a cell may have |

| Constants | |
| :--- | :--- |
| [`CT_MUL_OP_COUNT`](#constants) | |
| [`MUL_OP_SPELLINGS`](#constants) | the valid op spellings, for a refusal that lists them |

---

## CtOp

```mach
pub def CtOp: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CT_OP_NONE` | `0` |  |
| `CT_OP_INT_DIVMOD` | `1` |  |
| `CT_OP_FLOAT` | `2` |  |
| `CT_OP_INT_MUL` | `3` |  |
| `CT_OP_VAR_SHIFT` | `4` |  |

### ct_cap

```mach
pub fun ct_cap(op: CtOp) opt[CtCap]
```

the capability a target must provide for an operation to be constant-time,
absent for an operation outside the catalog: an unknown operation is never
answered with "no capability needed"

## CtCap

```mach
pub def CtCap: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CT_CAP_OK` | `0` |  |
| `CT_CAP_ALWAYS` | `1` |  |
| `CT_CAP_MUL` | `2` |  |

### cap_at_sema

```mach
pub fun cap_at_sema(cap: CtCap) bool
```

whether type checking reports an operation needing `cap` itself, since no target provides it

| Parameter | |
| :--- | :--- |
| `cap` | the capability the operation needs |
| **returns** | true for CT_CAP_ALWAYS |

## CtMulOp

```mach
pub def CtMulOp: u8
```

a multiply `$mach.build.ct_mul` asks about: the low half of a same-width
product, the high half by operand signedness, or the full double-width
product

| Name | Value | |
| :--- | :--- | :--- |
| `CT_MUL_NONE` | `0` |  |
| `CT_MUL_LOW` | `1` |  |
| `CT_MUL_HIGH_U` | `2` |  |
| `CT_MUL_HIGH_S` | `3` |  |
| `CT_MUL_HIGH_SU` | `4` |  |
| `CT_MUL_WIDE_U` | `5` |  |
| `CT_MUL_WIDE_S` | `6` |  |

### mul_cell

```mach
pub fun mul_cell(op: CtMulOp, width: u8, vector: bool) CtMulCell
```

the cell of a multiply

| Parameter | |
| :--- | :--- |
| `op` | the multiply |
| `width` | the operand width in bits |
| `vector` | whether it is a lane multiply |
| **returns** | the cell |

### mul_op_name

```mach
pub fun mul_op_name(op: CtMulOp) opt[str]
```

the spelling `$mach.build.ct_mul` reads, absent for NONE or an unknown tag

## CtMulCell

```mach
pub rec CtMulCell {
    op:     CtMulOp;
    width:  u8;
    vector: bool;
}
```

a multiply at an operand width in bits, `vector` for a lane multiply

## CtMulMask

```mach
pub rec CtMulMask {
    scalar: u64;
    vector: u64;
}
```

the cells mirl runs in constant time, one bit per (op, width) in a scalar and a vector plane

### mul_mask_none

```mach
pub fun mul_mask_none() CtMulMask
```

a mask holding no cell

### mul_mask_add

```mach
pub fun mul_mask_add(mask: *CtMulMask, cell: CtMulCell)
```

add a cell to a mask, in its vector plane for a lane multiply. an unknown cell adds nothing

| Parameter | |
| :--- | :--- |
| `mask` | the mask to add to |
| `cell` | the cell |

### mul_mask_has

```mach
pub fun mul_mask_has(mask: CtMulMask, cell: CtMulCell) bool
```

whether the cells cover this multiply, never for an unknown cell

## Functions

### mul_cell_width_valid

```mach
pub fun mul_cell_width_valid(width: u8) bool
```

the operand widths a cell may have

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CT_MUL_OP_COUNT` | `u32` | `7` |  |
| `MUL_OP_SPELLINGS` | `str` | `"low, high_u, high_s, high_su, wide_u, wide_s"` | the valid op spellings, for a refusal that lists them |

