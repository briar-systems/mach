# mach.lang.me.ir.refs

## val REF_VALUE_USE

```mach
pub val REF_VALUE_USE:     RefRole = 0
```

## rec Ref

```mach
pub rec Ref;
```

## fun operand_is_block

```mach
pub fun operand_is_block(k: ir_instruction.InstrKind, o: u32) bool;
```

## fun operand_is_value_use

```mach
pub fun operand_is_value_use(k: ir_instruction.InstrKind, o: u32) bool;
```

## fun visit_operands

```mach
pub fun visit_operands[T](inst: *ir_instruction.Instruction, ctx: *T, f: fun(*T, *Ref) bool) bool;
```

