# mach.lang.me.ir

Middle-end intermediate representation. Defines the SSA-form data
model the rest of `me/` and `be/` operate on: a [`Module`](#module)
holds a list of [`Function`](#function)s; each function is a graph
of [`Block`](#block)s; each block is a sequence of
[`Instruction`](ir/instruction.md#instruction)s ending in a terminator;
operands are [`Value`](ir/value.md#value)s — either instruction results,
function parameters, constants, or globals. Types are owned by a
separate IR-level [`type`](ir/type.md) module, distinct from the
source-language [`lang.type`](../type.md) universe.

Source is `new/lang/me/ir.mach` (currently empty — this spec is the
design intent).

## Types

### `Module`

```mach
pub rec Module {
    alloc:        *Allocator;
    name:         intern.StrId;
    functions:    *Function;
    function_len: u32;
    globals:      *Global;
    global_len:   u32;
    types:        ir_type.IrTypeTable;
    init_fn:      Option[u32];
}
```

One IR module per source module (mapped 1:1 by
[`sess.ModuleId`](../session.md#moduleid) — the
[`Q_LOWER`](../query.md#integration) key carries that identity, so the
`Module` does not store it). Owned by the `Q_LOWER` result.

| Field        | Type                                          | Description                                          |
|--------------|-----------------------------------------------|------------------------------------------------------|
| alloc        | `*Allocator`                                  | Backing allocator for every owned array.             |
| name         | [`intern.StrId`](../intern.md#strid)          | Module's FQN (mirror of [`ModuleEntry.fqn`](../driver.md#moduleentry)). |
| functions    | [`*Function`](#function)                      | Array of functions defined in this module.           |
| function_len | `u32`                                         | Number of entries in `functions`.                    |
| globals      | [`*Global`](#global)                          | Array of module-level globals (val / var decls).     |
| global_len   | `u32`                                         | Number of entries in `globals`.                      |
| types        | [`ir_type.IrTypeTable`](ir/type.md#irtypetable) | Per-module IR-type interner; type ids are local to the module. |
| init_fn      | `Option[u32]`                                 | Index into `functions` of the synthesized module-init function that runs deferred (non-comptime) global initialisers before `main`; `none` when the module has no deferred initialisers. |

### `Function`

```mach
pub rec Function {
    name:        intern.StrId;
    sig:         ir_type.IrTypeId;
    params:      *Value;
    param_count: u32;
    blocks:      *Block;
    block_count: u32;
    flags:       u32;
    asm:         map.Map[id.InstructionId, IrAsm];
}
```

The entry block is always [`BlockId`](ir/id.md#blockid) `0`; it is not
stored as a field.

| Field       | Type                                          | Description                                                |
|-------------|-----------------------------------------------|------------------------------------------------------------|
| name        | [`intern.StrId`](../intern.md#strid)          | Function symbol (interned).                                |
| sig         | [`ir_type.IrTypeId`](ir/type.md#irtypeid)     | Function type — `(param types, return type, varargs flag)`. |
| params      | [`*Value`](ir/value.md#value)                 | Parameter Values; consumers reference them like any other Value. |
| param_count | `u32`                                         | Length of `params`.                                        |
| blocks      | [`*Block`](#block)                            | Block array indexed by [`BlockId`](ir/id.md#blockid).      |
| block_count | `u32`                                         | Length of `blocks`.                                        |
| flags       | `u32`                                         | Bitfield of [`FN_FLAG_*`](#constants).                     |
| asm         | `map.Map[`[`id.InstructionId`](ir/id.md#instructionid)`, `[`IrAsm`](#irasm)`]` | Side-table from each `OP_ASM` instruction to its inline-asm payload. Empty for functions with no inline asm. |

### `Block`

```mach
pub rec Block {
    id:           id.BlockId;
    instructions: *id.InstructionId;
    instr_count:  u32;
    phis:         *id.InstructionId;
    phi_count:    u32;
    terminator:   id.InstructionId;
    preds:        *id.BlockId;
    pred_count:   u32;
}
```

| Field        | Type                                                  | Description                                                |
|--------------|-------------------------------------------------------|------------------------------------------------------------|
| id           | [`id.BlockId`](ir/id.md#blockid)                      | Self-identifying index.                                    |
| instructions | [`*id.InstructionId`](ir/id.md#instructionid)         | Body of the block, in execution order. Excludes phis and the terminator. |
| instr_count  | `u32`                                                 | Length of `instructions`.                                  |
| phis         | [`*id.InstructionId`](ir/id.md#instructionid)         | Phi instructions; logically execute before the body. |
| phi_count    | `u32`                                                 | Length of `phis`.                                          |
| terminator   | [`id.InstructionId`](ir/id.md#instructionid)          | Single terminator (`br` / `cbr` / `ret` / `unreachable`); [`INSTR_NIL`](ir/id.md#instr_nil) until one is emitted. |
| preds        | [`*id.BlockId`](ir/id.md#blockid)                     | Predecessor block ids (maintained alongside terminator edges). |
| pred_count   | `u32`                                                 | Length of `preds`.                                         |

### `Global`

```mach
pub rec Global {
    name:    intern.StrId;
    ty:      ir_type.IrTypeId;
    init:    Value;
    is_mut:  bool;
    is_pub:  bool;
}
```

A module-level binding. A comptime-evaluable initialiser is folded
into a [`Value`](ir/value.md#value) constant in `init`. A non-comptime
initialiser leaves `init` zero-initialised and emits its real
initialisation logic into the module's [`init_fn`](#module) instead.

### `IrAsm`

```mach
pub rec IrAsm {
    isa:           intern.StrId;
    body:          intern.StrId;
    operands:      *IrAsmOperand;
    operand_count: u32;
}
```

Self-contained payload for one `OP_ASM` instruction, reached through
[`Function.asm`](#function). `isa` and `body` are interned strings —
the IR holds no [`token.Span`](../fe/token.md#span)s back into source,
so an [`ir.Module`](#module) is interpretable without the AST.

| Field         | Type                                  | Description                                                |
|---------------|---------------------------------------|------------------------------------------------------------|
| isa           | [`intern.StrId`](../intern.md#strid)  | Interned ISA identifier (e.g. `x86_64`); empty for portable MASM. |
| body          | [`intern.StrId`](../intern.md#strid)  | Interned raw assembly text.                                |
| operands      | [`*IrAsmOperand`](#irasmoperand)      | Operand descriptors, in source order.                      |
| operand_count | `u32`                                 | Length of `operands`.                                      |

The `IR_ASM_IN` / `IR_ASM_OUT` descriptors line up positionally, in
order, with the `OP_ASM` instruction's
[`Value`](ir/value.md#value) operand vector; `IR_ASM_CLOBBER`
descriptors carry no Value.

### `IrAsmOperand`

```mach
pub rec IrAsmOperand {
    role:       IrAsmOpRole;
    constraint: intern.StrId;
}
```

| Field      | Type                                  | Description                                                |
|------------|---------------------------------------|------------------------------------------------------------|
| role       | [`IrAsmOpRole`](#irasmoprole)         | One of [`IR_ASM_*`](#constants).                           |
| constraint | [`intern.StrId`](../intern.md#strid)  | Interned register / constraint string. For `IR_ASM_CLOBBER` this is the clobbered register name. |

### `IrAsmOpRole`

```mach
pub def IrAsmOpRole: u8;
```

Discriminator for an [`IrAsmOperand`](#irasmoperand); one of
[`IR_ASM_*`](#constants).

## Constants

```mach
pub val FN_FLAG_PUB:      u32 = 0x01;
pub val FN_FLAG_EXTERN:   u32 = 0x02;
pub val FN_FLAG_INLINE:   u32 = 0x04;
pub val FN_FLAG_NORETURN: u32 = 0x08;
pub val FN_FLAG_TEST:     u32 = 0x10;
```

| Constant            | Meaning                                              |
|---------------------|------------------------------------------------------|
| `FN_FLAG_PUB`       | Function is exported from its module.                |
| `FN_FLAG_EXTERN`    | Function is defined elsewhere (no `blocks`).          |
| `FN_FLAG_INLINE`    | Inlining is requested by the user / inferred.        |
| `FN_FLAG_NORETURN`  | Function never returns (panics, infinite loops).     |
| `FN_FLAG_TEST`      | Function is a lowered `test "..." { }` decl; the test runner enumerates these. |

```mach
pub val IR_ASM_IN:      IrAsmOpRole = 0;
pub val IR_ASM_OUT:     IrAsmOpRole = 1;
pub val IR_ASM_CLOBBER: IrAsmOpRole = 2;
```

| Constant         | Meaning                                              |
|------------------|------------------------------------------------------|
| `IR_ASM_IN`      | Input operand — a Value fed into the asm block.      |
| `IR_ASM_OUT`     | Output operand — a Value produced by the asm block.  |
| `IR_ASM_CLOBBER` | Clobbered register; no associated Value.             |

## Functions

### `init_module`

```mach
pub fun init_module(
    alloc: *Allocator,
    name:  intern.StrId,
) Result[Module, str]
```

Allocates an empty [`Module`](#module) with the given name and
`init_fn` set to `none`.

### `dnit_module`

```mach
pub fun dnit_module(m: *Module)
```

Releases every owned array, the per-module IR-type table, and every
function's `asm` side-table. `nil` is a no-op.

## Sub-modules

- [`ir.id`](ir/id.md) — index-handle types.
- [`ir.value`](ir/value.md) — operand model.
- [`ir.type`](ir/type.md) — IR-level type system.
- [`ir.instruction`](ir/instruction.md) — instruction kinds and encoding.
- [`ir.builder`](ir/builder.md) — emission convenience layer.
- [`ir.printer`](ir/printer.md) — textual debug dump.
- [`ir.verify`](ir/verify.md) — SSA / type invariant checker.

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`, `std.allocator`,
`std.collections.map`,
[`mach.lang.intern`](../intern.md),
[`mach.lang.me.ir.id`](ir/id.md),
[`mach.lang.me.ir.value`](ir/value.md),
[`mach.lang.me.ir.type`](ir/type.md).
