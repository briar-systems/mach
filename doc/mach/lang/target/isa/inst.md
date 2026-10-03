# mach.lang.target.isa.inst

## val OPK_NONE

```mach
pub val OPK_NONE:  OperandKind = 0
```

## val OPK_REG

```mach
pub val OPK_REG:   OperandKind = 1
```

## val OPK_IMM

```mach
pub val OPK_IMM:   OperandKind = 2
```

## val OPK_MEM

```mach
pub val OPK_MEM:   OperandKind = 3
```

## val OPK_LABEL

```mach
pub val OPK_LABEL: OperandKind = 4
```

## val OPK_SYM

```mach
pub val OPK_SYM:   OperandKind = 5
```

## val REG_CLASS_ID_GP

```mach
pub val REG_CLASS_ID_GP: i32 = 0
```

## val REG_CLASS_ID_FP

```mach
pub val REG_CLASS_ID_FP: i32 = 1
```

## val REG_NONE

```mach
pub val REG_NONE:    i32 = 0
```

a register id carries its class plus one in its second byte and its hardware
number in its low byte, so no register is 0 and a zeroed operand names none.
each base is a class's id for hardware number 0, for tables that cannot call
regid_make

## val REG_GP_BASE

```mach
pub val REG_GP_BASE: i32 = 0x100
```

## val REG_FP_BASE

```mach
pub val REG_FP_BASE: i32 = 0x200
```

## def SymModifier

```mach
pub def SymModifier: u8
```

## val SYM_MOD_NONE

```mach
pub val SYM_MOD_NONE:     SymModifier = 0
```

## val SYM_MOD_PCREL_HI

```mach
pub val SYM_MOD_PCREL_HI: SymModifier = 1
```

## val SYM_MOD_PCREL_LO

```mach
pub val SYM_MOD_PCREL_LO: SymModifier = 2
```

## val SYM_MOD_GOT_HI

```mach
pub val SYM_MOD_GOT_HI:   SymModifier = 3
```

## val SYM_MOD_GOT_LO

```mach
pub val SYM_MOD_GOT_LO:   SymModifier = 4
```

## val SYM_MOD_GOT

```mach
pub val SYM_MOD_GOT: SymModifier = 5
```

a whole pc-relative reference to the symbol's GOT slot (x86-64 GOTPCREL)

## rec Operand

```mach
pub rec Operand;
```

## rec Inst

```mach
pub rec Inst;
```

## fun regid_make

```mach
pub fun regid_make(class_id: i32, index: i32) i32;
```

## fun regid_class

```mach
pub fun regid_class(regid: i32) i32;
```

the class of `regid`, -1 for REG_NONE

## fun regid_names

```mach
pub fun regid_names(regid: i32) bool;
```

whether `regid` names a register: neither REG_NONE nor a negative operand
sentinel an instruction set keeps for itself

## fun regid_index

```mach
pub fun regid_index(regid: i32) i32;
```

the hardware number of `regid` within its class

## fun make_none

```mach
pub fun make_none() Operand;
```

## fun make_reg

```mach
pub fun make_reg(id: i32, size: u8) Operand;
```

## fun make_imm

```mach
pub fun make_imm(value: i64, size: u8) Operand;
```

## fun make_mem

```mach
pub fun make_mem(base: i32, disp: i32, index: i32, scale: u8, size: u8) Operand;
```

## fun make_block_label

```mach
pub fun make_block_label(id: u32) Operand;
```

## fun make_local_label

```mach
pub fun make_local_label(forward: bool) Operand;
```

a target inside the same expansion, resolved by the encoder that emitted
it: not a block of the function and not a symbol. the direction is what
a walk needs: a forward skip only joins ahead, a backward one is a loop

## fun make_numbered_local_label

```mach
pub fun make_numbered_local_label(forward: bool, number: u32) Operand;
```

a local label with a number, so a listing can spell the jump as `1f` or
`1b` and its definition as `1:`; the encoder still patches the displacement

## fun local_label_number

```mach
pub fun local_label_number(op: *Operand) u32;
```

## fun label_is_block

```mach
pub fun label_is_block(op: *Operand) bool;
```

## fun label_is_forward_local

```mach
pub fun label_is_forward_local(op: *Operand) bool;
```

## fun make_sym

```mach
pub fun make_sym(sym_id: u32, size: u8) Operand;
```

## fun make_sym_mod

```mach
pub fun make_sym_mod(sym_id: u32, size: u8, mod: SymModifier) Operand;
```

## fun make_sym_addend

```mach
pub fun make_sym_addend(sym_id: u32, size: u8, mod: SymModifier, addend: i64) Operand;
```

## fun inst_blank

```mach
pub fun inst_blank() Inst;
```

