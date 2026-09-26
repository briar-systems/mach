# mach.lang.me.ir.refs

## def RefRole

```mach
pub def RefRole: u8
```

## val REF_VALUE_USE

```mach
pub val REF_VALUE_USE: RefRole = 0
```

## val REF_CFG_EDGE

```mach
pub val REF_CFG_EDGE:  RefRole = 1
```

## val REF_PHI_EDGE

```mach
pub val REF_PHI_EDGE:  RefRole = 2
```

## val REF_DEBUG_USE

```mach
pub val REF_DEBUG_USE: RefRole = 4
```

## val REF_GLOBAL

```mach
pub val REF_GLOBAL:        RefRole = 6
```

## val REF_FUNCTION

```mach
pub val REF_FUNCTION:      RefRole = 7
```

## def MetaKind

```mach
pub def MetaKind: u8
```

## val META_NONE

```mach
pub val META_NONE: MetaKind = 0
```

## val SLOT_NONE

```mach
pub val SLOT_NONE: u32 = 0xFFFFFFFF
```

## rec Ref

```mach
pub rec Ref;
```

## fun operand_is_block

```mach
pub fun operand_is_block(k: instruction.InstrKind, o: u32) bool;
```

## fun operand_is_value_use

```mach
pub fun operand_is_value_use(k: instruction.InstrKind, o: u32) bool;
```

## fun visit_operands

```mach
pub fun visit_operands[T](inst: *instruction.Instruction, ctx: *T, f: fun(*T, *Ref) bool) bool;
```

