# mach.lang.target.abi.aapcs64

## val REG_X0

```mach
pub val REG_X0: i32 = 0
```

## val REG_X1

```mach
pub val REG_X1: i32 = 1
```

## val REG_X8

```mach
pub val REG_X8:  i32 = 8
```

## val REG_X19

```mach
pub val REG_X19: i32 = 19
```

## val GP_PARAM_COUNT

```mach
pub val GP_PARAM_COUNT:        i32 = 8
```

## val FP_PARAM_COUNT

```mach
pub val FP_PARAM_COUNT:        i32 = 8
```

## val GP_CALLEE_SAVED_COUNT

```mach
pub val GP_CALLEE_SAVED_COUNT: i32 = 10
```

## val FP_CALLEE_SAVED_COUNT

```mach
pub val FP_CALLEE_SAVED_COUNT: i32 = 8
```

## val CALLEE_SAVED_COUNT

```mach
pub val CALLEE_SAVED_COUNT:    i32 = 18
```

## val STACK_ALIGN

```mach
pub val STACK_ALIGN: u32 = 16
```

## val RED_ZONE

```mach
pub val RED_ZONE:    u32 = 0
```

## val MAX_REG_RET

```mach
pub val MAX_REG_RET: u64 = 16
```

## def ArgPassingFn

```mach
pub def ArgPassingFn: abi.ArgPassingFn
```

## def RetPassingFn

```mach
pub def RetPassingFn: abi.RetPassingFn
```

## def RegFileFn

```mach
pub def RegFileFn:    abi.RegFileFn
```

## def VaModelFn

```mach
pub def VaModelFn:    abi.VaModelFn
```

## val WORD

```mach
pub val WORD: u32 = 8
```

## val DOUBLE_WORD

```mach
pub val DOUBLE_WORD: u64 = 16
```

a 16-byte integer scalar: C.10 rounds NGRN up to even when its alignment is
16 and passes it in the next two registers, else C.11 sets NGRN to 8 and it
goes to the stack at its natural 16-byte alignment (#3511). darwin hands the
classifier an alignment of 8, so the pair starts in any register (#3922)

## fun gp_param_reg

```mach
pub fun gp_param_reg(index: i32) i32;
```

## fun fp_param_reg

```mach
pub fun fp_param_reg(index: i32) i32;
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

## fun va_model

```mach
pub fun va_model() abi.VaModel;
```

## fun register

```mach
pub fun register(reg: *abi.AbiRegistry) err[fail.Fail];
```

