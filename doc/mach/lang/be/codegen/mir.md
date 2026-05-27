# mach.lang.be.codegen.mir

Machine IR. A target-aware intermediate form between the SSA IR
([`ir`](../../me/ir.md)) and the encoded bytes. Each MIR
instruction has a target-specific opcode (via
[`target.arch`](../../target/isa.md#isavtable)) but still uses
virtual registers; later codegen passes
([`regalloc`](regalloc.md), [`frame`](frame.md)) rewrite operands
into physical registers and stack slots.

Source is `new/lang/be/codegen/mir.mach` (currently empty).

## Types

### `MirModule`, `MirFunction`, `MirBlock`, `MirInstr`

```mach
pub rec MirModule {
    functions:    *MirFunction;
    function_len: u32;
}

pub rec MirFunction {
    name:        intern.StrId;
    blocks:      *MirBlock;
    block_count: u32;
    vregs:       *MirVReg;
    vreg_count:  u32;
    frame:       MirFrame;
}

pub rec MirBlock {
    id:          u32;
    instrs:      *MirInstr;
    instr_count: u32;
    preds:       *u32;
    pred_count:  u32;
}

pub rec MirInstr {
    opcode:      u32;
    operands:    *MirOperand;
    operand_count: u32;
}
```

The opcode is target-specific — for x86\_64 it's the encoder's
internal opcode id (`MOV_RR`, `ADD_RR`, `JMP_REL32`, ...); for ARM64
it's the AArch64 encoder's id. The opcode tables themselves live in
the per-arch ISA impl ([`isa.x64`](../../target/isa/x64.md),
[`isa.arm64`](../../target/isa/arm64.md)).

### `MirOperand`

```mach
pub rec MirOperand {
    kind:    MirOperandKind;
    vreg:    u32;
    preg:    u32;
    imm:     i64;
    sym:     intern.StrId;
    sym_off: i32;
}
```

Tagged union over operand kinds.

| `MIR_OP_*` | `kind` | Active fields                                              |
|------------|--------|------------------------------------------------------------|
| `VREG`     | 0      | `vreg` — virtual register index, resolved by regalloc.     |
| `PREG`     | 1      | `preg` — physical register index in its [`RegClass`](../../target/isa.md#regclass). |
| `IMM`      | 2      | `imm` — immediate constant.                                |
| `MEM`      | 3      | `vreg` (base) + `imm` (displacement); addressing-mode shape is per-arch. |
| `SYM`      | 4      | `sym` + `sym_off` — symbol reference for a relocation.     |
| `BLOCK`    | 5      | `imm` — `MirBlock.id` for branch targets.                  |

### `MirVReg`

```mach
pub rec MirVReg {
    id:           u32;
    class:        u32;
    spill_slot:   i32;
    assigned:     u32;
}
```

A virtual register's regalloc state. `class` is an index into
[`target.arch.reg_classes`](../../target/isa.md#isavtable);
`assigned` is the physical register id after regalloc; `spill_slot`
is a stack-frame slot when the vreg is spilled.

### `MirFrame`

```mach
pub rec MirFrame {
    slots:       *MirSlot;
    slot_count:  u32;
    size:        u32;
    align:       u32;
}
```

The function's stack layout, populated by [`frame.run`](frame.md).

## Functions

### `lower_ir`

```mach
pub fun lower_ir(
    tgt: *target.Target,
    ir:  *ir.Module,
) Result[MirModule, str]
```

Per-function translation:

- Every [`Block`](../../me/ir.md#block) becomes a
  [`MirBlock`](#mirmodule-mirfunction-mirblock-mirinstr) with the same id.
- SSA value references become `MIR_OP_VREG` operands; one fresh
  virtual register per SSA result.
- `OP_CALL` lowers to the ABI-classified marshalling of arguments
  (move into the canonical arg registers per
  [`target.abi`](../../target/abi.md#abivtable)) + a call
  pseudo-instruction.
- `OP_PHI` survives into MIR; regalloc resolves it during the
  rewrite phase.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.intern`](../../intern.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.me.ir`](../../me/ir.md).
