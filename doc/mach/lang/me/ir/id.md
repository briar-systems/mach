# mach.lang.me.ir.id

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

the index of nothing: a slot, a block or an entry a table does not hold

## rec BlockId

```mach
pub rec BlockId;
```

a block's slot in its function

## val BLOCK_NIL

```mach
pub val BLOCK_NIL: BlockId = BlockId;
```

## fun block

```mach
pub fun block(index: u32) BlockId;
```

## fun block_index

```mach
pub fun block_index(id: BlockId) u32;
```

## fun block_same

```mach
pub fun block_same(left: BlockId, right: BlockId) bool;
```

## fun block_is_nil

```mach
pub fun block_is_nil(id: BlockId) bool;
```

## rec InstructionId

```mach
pub rec InstructionId;
```

an instruction's slot in its function

## val INSTR_NIL

```mach
pub val INSTR_NIL: InstructionId = InstructionId;
```

## fun instruction

```mach
pub fun instruction(index: u32) InstructionId;
```

## fun instruction_index

```mach
pub fun instruction_index(id: InstructionId) u32;
```

## fun instruction_same

```mach
pub fun instruction_same(left: InstructionId, right: InstructionId) bool;
```

## fun instruction_is_nil

```mach
pub fun instruction_is_nil(id: InstructionId) bool;
```

