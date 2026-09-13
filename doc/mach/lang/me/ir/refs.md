# mach.lang.me.ir.refs

## def RefRole

```mach
pub def RefRole: u8
```

## val REF_VALUE_USE

```mach
pub val REF_VALUE_USE:     RefRole = 0
```

## val REF_CFG_EDGE

```mach
pub val REF_CFG_EDGE:      RefRole = 1
```

## val REF_PHI_EDGE

```mach
pub val REF_PHI_EDGE:      RefRole = 2
```

## val REF_ASM_BIND

```mach
pub val REF_ASM_BIND:      RefRole = 3
```

## val REF_DEBUG_USE

```mach
pub val REF_DEBUG_USE:     RefRole = 4
```

## val REF_INLINE_PARENT

```mach
pub val REF_INLINE_PARENT: RefRole = 5
```

## val REF_GLOBAL

```mach
pub val REF_GLOBAL:        RefRole = 6
```

## val REF_FUNCTION

```mach
pub val REF_FUNCTION:      RefRole = 7
```

## val REF_METADATA

```mach
pub val REF_METADATA:      RefRole = 8
```

## def MetaKind

```mach
pub def MetaKind: u8
```

## val META_NONE

```mach
pub val META_NONE:           MetaKind = 0
```

## val META_ASM

```mach
pub val META_ASM:            MetaKind = 1
```

## val META_DBG_VAR

```mach
pub val META_DBG_VAR:        MetaKind = 2
```

## val META_ALLOCA_DBG_VAR

```mach
pub val META_ALLOCA_DBG_VAR: MetaKind = 3
```

## val META_DBG_EXPR

```mach
pub val META_DBG_EXPR:       MetaKind = 4
```

## val META_INLINE_SITE

```mach
pub val META_INLINE_SITE:    MetaKind = 5
```

## val META_KIND_COUNT

```mach
pub val META_KIND_COUNT: u32 = 6
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

## fun visit

```mach
pub fun visit[T](fn: *ir.Function, iid: id.InstructionId, ctx: *T, f: fun(*T, *Ref) bool) bool;
```

