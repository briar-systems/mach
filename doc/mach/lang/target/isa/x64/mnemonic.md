# mach.lang.target.isa.x64.mnemonic

the inline assembly mnemonic rows: each spelling, its operand pattern, widths and the registers it clobbers

## val PATTERN_NONE

```mach
pub val PATTERN_NONE: u16 = 1
```

## val PATTERN_ONE

```mach
pub val PATTERN_ONE: u16 = 2
```

## val PATTERN_TWO

```mach
pub val PATTERN_TWO: u16 = 3
```

## val PATTERN_PORTIO

```mach
pub val PATTERN_PORTIO: u16 = 8
```

## val PATTERN_WIDEREG

```mach
pub val PATTERN_WIDEREG: u16 = 9
```

one 32- or 64-bit general-purpose register and nothing else

## val PATTERN_VECG

```mach
pub val PATTERN_VECG: u16 = 10
```

a vector register and a 32-bit register or memory, with an 8-bit immediate

## val PATTERN_TRIPLE

```mach
pub val PATTERN_TRIPLE: u16 = 11
```

two general-purpose operands and a third: an immediate, or `cl` where the
row takes a count register; imul alone also stands with two

## val PATTERN_BRANCH

```mach
pub val PATTERN_BRANCH: u16 = 4
```

## val PATTERN_XFER

```mach
pub val PATTERN_XFER: u16 = 5
```

## val PATTERN_VEC

```mach
pub val PATTERN_VEC: u16 = 6
```

## val PATTERN_VECI

```mach
pub val PATTERN_VECI: u16 = 7
```

## val PATTERN_VEC3

```mach
pub val PATTERN_VEC3: u16 = 12
```

a vector register written from a vector register and a vector register or memory

## val PATTERN_STRING

```mach
pub val PATTERN_STRING: u16 = 13
```

a string instruction: no operand, rsi, rdi and rcx implicit, and an optional
repeat prefix (X64_PREFIXES)

## val PATTERN_MOVQ

```mach
pub val PATTERN_MOVQ: u16 = 14
```

movq between a vector register's low quadword and a 64-bit general register

## val PATTERN_VSHIFT

```mach
pub val PATTERN_VSHIFT: u16 = 15
```

a packed shift of a vector register by an 8-bit immediate, or for a row with
a count form by a vector register or memory's low quadword

## val WIDTH_NARROW

```mach
pub val WIDTH_NARROW: u16 = 0x1000
```

## val WIDTH_FIXED64

```mach
pub val WIDTH_FIXED64: u16 = 0x2000
```

## val WIDTH_UNSIZED

```mach
pub val WIDTH_UNSIZED: u16 = 0x3000
```

## val FLAG_LOCK

```mach
pub val FLAG_LOCK: u16 = 0x0100
```

## val FLAG_INDIRECT

```mach
pub val FLAG_INDIRECT: u16 = 0x0200
```

## val FLAG_VSTORE

```mach
pub val FLAG_VSTORE: u16 = 0x0400
```

a packed move, which may also store its vector register into memory

## val FLAG_REP

```mach
pub val FLAG_REP: u16 = 0x0800
```

a string instruction's repeat prefix: f3 or f2, and whether it was spelled
with a condition (repe, repz, repne, repnz), which only a compare takes

## val FLAG_REPNE

```mach
pub val FLAG_REPNE: u16 = 0x4000
```

## val FLAG_REPCND

```mach
pub val FLAG_REPCND: u16 = 0x8000
```

## val SYSCALL_WRITES

```mach
pub val SYSCALL_WRITES: u32 = 0x803
```

## val STRING_RCX

```mach
pub val STRING_RCX: u32 = 0x02
```

## val ALL_GP

```mach
pub val ALL_GP: u32 = 0xFFFF
```

## val ALL_FP

```mach
pub val ALL_FP: u32 = 0xFFFF
```

## fun pattern

```mach
pub fun pattern(flags: u16) u16;
```

## fun width_class

```mach
pub fun width_class(flags: u16) u16;
```

## val ROW_COUNT

```mach
pub val ROW_COUNT: usize = 187
```

## val ROWS

```mach
pub val ROWS: [ROW_COUNT]isa_asm.Mnemonic = [ROW_COUNT]isa_asm.Mnemonic;
```

## rec Prefix

```mach
pub rec Prefix;
```

a string instruction's repeat prefixes as GNU as spells them. rep repeats a
move, store or load rcx times; the conditioned spellings repeat a compare
while its elements are equal (f3) or differ (f2), rcx times at most

## val PREFIX_COUNT

```mach
pub val PREFIX_COUNT: usize = 5
```

## val PREFIXES

```mach
pub val PREFIXES: [PREFIX_COUNT]Prefix = [PREFIX_COUNT]Prefix;
```

