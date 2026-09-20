# mach.lang.target.abi.win64

## val REG_RAX

```mach
pub val REG_RAX: i32 = 0
```

## val REG_RCX

```mach
pub val REG_RCX: i32 = 1
```

## val REG_RDX

```mach
pub val REG_RDX: i32 = 2
```

## val REG_RBX

```mach
pub val REG_RBX: i32 = 3
```

## val REG_RSP

```mach
pub val REG_RSP: i32 = 4
```

## val REG_RBP

```mach
pub val REG_RBP: i32 = 5
```

## val REG_RSI

```mach
pub val REG_RSI: i32 = 6
```

## val REG_RDI

```mach
pub val REG_RDI: i32 = 7
```

## val REG_R8

```mach
pub val REG_R8:  i32 = 8
```

## val REG_R9

```mach
pub val REG_R9:  i32 = 9
```

## val REG_R10

```mach
pub val REG_R10: i32 = 10
```

## val REG_R11

```mach
pub val REG_R11: i32 = 11
```

## val REG_R12

```mach
pub val REG_R12: i32 = 12
```

## val REG_R13

```mach
pub val REG_R13: i32 = 13
```

## val REG_R14

```mach
pub val REG_R14: i32 = 14
```

## val REG_R15

```mach
pub val REG_R15: i32 = 15
```

## val REG_XMM0

```mach
pub val REG_XMM0: i32 = (1 << 8) | 0
```

## val PARAM_SLOT_COUNT

```mach
pub val PARAM_SLOT_COUNT: i32 = 4
```

## val VEC_REG_BYTES

```mach
pub val VEC_REG_BYTES:          u64 = 16
```

## val GP_PARAM_COUNT

```mach
pub val GP_PARAM_COUNT:         i32 = 4
```

## val FP_PARAM_COUNT

```mach
pub val FP_PARAM_COUNT:         i32 = 4
```

## val CALLEE_SAVED_GP_COUNT

```mach
pub val CALLEE_SAVED_GP_COUNT:  i32 = 9
```

## val CALLEE_SAVED_XMM_COUNT

```mach
pub val CALLEE_SAVED_XMM_COUNT: i32 = 10
```

## val CALLEE_SAVED_COUNT

```mach
pub val CALLEE_SAVED_COUNT:     i32 = CALLEE_SAVED_GP_COUNT + CALLEE_SAVED_XMM_COUNT
```

## val STACK_ALIGN

```mach
pub val STACK_ALIGN:  u32 = 16
```

## val RED_ZONE

```mach
pub val RED_ZONE:     u32 = 0
```

## val SHADOW_SPACE

```mach
pub val SHADOW_SPACE: u64 = 32
```

## fun is_by_value_size

```mach
pub fun is_by_value_size(size: u64) bool;
```

## val INT128

```mach
pub val INT128: u64 = 16
```

a 16-byte integer has no Microsoft rule (MSVC has no such type). the
convention is the one GCC (mingw-w64) and Clang share: LLVM 18 made i128
match __int128 ("Changes to the X86 Backend", LLVM 18.1 release notes), and
llvm/llvm-project#115052 describes that convention as passing on the stack
by reference and returning in xmm0, "identical to i128" and "the same as
GCC". the clang -O0 probe under test/link/cases exhibits it: the argument
is a pointer to a caller copy like any other 16-byte object, the value
returns in XMM0 with the low half in the low lanes (#3511)

## fun slot_gp_reg

```mach
pub fun slot_gp_reg(index: i32) i32;
```

## fun slot_fp_reg

```mach
pub fun slot_fp_reg(index: i32) i32;
```

## fun classify_arg

```mach
pub fun classify_arg(index: i32, size: u64, align: u64,
is_float: bool, fp_eightbytes: u8, is_aggregate: bool, is_vector: bool,
gp_used: i32, fp_used: i32, hfa_members: u8, hfa_elem: u8,
agg: abi.AggLayout) abi.ParamSlot;
```

## fun classify_return

```mach
pub fun classify_return(size: u64, align: u64,
is_float: bool, fp_eightbytes: u8, is_aggregate: bool, is_vector: bool,
hfa_members: u8, hfa_elem: u8, agg: abi.AggLayout) abi.ParamSlot;
```

## fun gp_param_regs

```mach
pub fun gp_param_regs(out: *isa.Register) i32;
```

## fun callee_saved

```mach
pub fun callee_saved(out: *isa.Register) i32;
```

## fun stack_align

```mach
pub fun stack_align() u32;
```

## fun red_zone

```mach
pub fun red_zone() u32;
```

## fun shadow_space

```mach
pub fun shadow_space() u64;
```

## fun va_model

```mach
pub fun va_model() abi.VaModel;
```

## val EIGHTBYTE

```mach
pub val EIGHTBYTE: u32 = 8
```

## fun register_win64

```mach
pub fun register_win64(reg: *abi.AbiRegistry) err[fail.Fail];
```

