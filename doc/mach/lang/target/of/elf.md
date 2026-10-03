# mach.lang.target.of.elf

## val R_X86_64_64

```mach
pub val R_X86_64_64:       u32 = 1
```

## val R_X86_64_PC32

```mach
pub val R_X86_64_PC32:     u32 = 2
```

## val R_X86_64_PLT32

```mach
pub val R_X86_64_PLT32:    u32 = 4
```

## val R_X86_64_GOTPCREL

```mach
pub val R_X86_64_GOTPCREL: u32 = 9
```

## val R_X86_64_32

```mach
pub val R_X86_64_32:       u32 = 10
```

## val R_X86_64_32S

```mach
pub val R_X86_64_32S:      u32 = 11
```

## val R_X86_64_PC64

```mach
pub val R_X86_64_PC64:     u32 = 24
```

## val R_AARCH64_ABS64

```mach
pub val R_AARCH64_ABS64:               u32 = 257
```

## val R_AARCH64_ABS32

```mach
pub val R_AARCH64_ABS32:               u32 = 258
```

## val R_AARCH64_PREL64

```mach
pub val R_AARCH64_PREL64:              u32 = 260
```

## val R_AARCH64_PREL32

```mach
pub val R_AARCH64_PREL32:              u32 = 261
```

## val R_AARCH64_JUMP26

```mach
pub val R_AARCH64_JUMP26:              u32 = 282
```

## val R_AARCH64_CALL26

```mach
pub val R_AARCH64_CALL26:              u32 = 283
```

## val R_AARCH64_ADR_PREL_PG_HI21

```mach
pub val R_AARCH64_ADR_PREL_PG_HI21:    u32 = 275
```

## val R_AARCH64_ADD_ABS_LO12_NC

```mach
pub val R_AARCH64_ADD_ABS_LO12_NC:     u32 = 277
```

## val R_AARCH64_LDST8_ABS_LO12_NC

```mach
pub val R_AARCH64_LDST8_ABS_LO12_NC:   u32 = 278
```

## val R_AARCH64_LDST16_ABS_LO12_NC

```mach
pub val R_AARCH64_LDST16_ABS_LO12_NC:  u32 = 284
```

## val R_AARCH64_LDST32_ABS_LO12_NC

```mach
pub val R_AARCH64_LDST32_ABS_LO12_NC:  u32 = 285
```

## val R_AARCH64_LDST64_ABS_LO12_NC

```mach
pub val R_AARCH64_LDST64_ABS_LO12_NC:  u32 = 286
```

## val R_AARCH64_LDST128_ABS_LO12_NC

```mach
pub val R_AARCH64_LDST128_ABS_LO12_NC: u32 = 299
```

## val R_AARCH64_ADR_GOT_PAGE

```mach
pub val R_AARCH64_ADR_GOT_PAGE:        u32 = 311
```

## val R_AARCH64_LD64_GOT_LO12_NC

```mach
pub val R_AARCH64_LD64_GOT_LO12_NC:    u32 = 312
```

## val R_RISCV_32

```mach
pub val R_RISCV_32:           u32 = 1
```

## val R_RISCV_64

```mach
pub val R_RISCV_64:           u32 = 2
```

## val R_RISCV_JAL

```mach
pub val R_RISCV_JAL:          u32 = 17
```

## val R_RISCV_CALL_PLT

```mach
pub val R_RISCV_CALL_PLT:     u32 = 19
```

## val R_RISCV_GOT_HI20

```mach
pub val R_RISCV_GOT_HI20:     u32 = 20
```

## val R_RISCV_PCREL_HI20

```mach
pub val R_RISCV_PCREL_HI20:   u32 = 23
```

## val R_RISCV_PCREL_LO12_I

```mach
pub val R_RISCV_PCREL_LO12_I: u32 = 24
```

## val R_RISCV_PCREL_LO12_S

```mach
pub val R_RISCV_PCREL_LO12_S: u32 = 25
```

## val R_RISCV_32_PCREL

```mach
pub val R_RISCV_32_PCREL:     u32 = 57
```

## fun machine_for_arch

```mach
pub fun machine_for_arch(arch_id: u32) u16;
```

the e_machine of an instruction set, 0 for one ELF does not cover

## val VTABLE

```mach
pub val VTABLE: target_of.OfVTable = target_of.OfVTable;
```

## fun parse_object

```mach
pub fun parse_object(alloc: *A.Allocator, itn: *intern.Interner, buf: *u8, buf_size: usize, out: *target_of.ObjectImage) err[fail.Fail];
```

