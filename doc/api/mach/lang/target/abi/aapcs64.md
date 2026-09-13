# mach.lang.target.abi.aapcs64

## val REG_X0

```mach
pub val REG_X0:  i32 = 0
```

## val REG_X1

```mach
pub val REG_X1:  i32 = 1
```

## val REG_X2

```mach
pub val REG_X2:  i32 = 2
```

## val REG_X3

```mach
pub val REG_X3:  i32 = 3
```

## val REG_X4

```mach
pub val REG_X4:  i32 = 4
```

## val REG_X5

```mach
pub val REG_X5:  i32 = 5
```

## val REG_X6

```mach
pub val REG_X6:  i32 = 6
```

## val REG_X7

```mach
pub val REG_X7:  i32 = 7
```

## val REG_X8

```mach
pub val REG_X8:  i32 = 8
```

## val REG_X19

```mach
pub val REG_X19: i32 = 19
```

## val REG_X20

```mach
pub val REG_X20: i32 = 20
```

## val REG_X21

```mach
pub val REG_X21: i32 = 21
```

## val REG_X22

```mach
pub val REG_X22: i32 = 22
```

## val REG_X23

```mach
pub val REG_X23: i32 = 23
```

## val REG_X24

```mach
pub val REG_X24: i32 = 24
```

## val REG_X25

```mach
pub val REG_X25: i32 = 25
```

## val REG_X26

```mach
pub val REG_X26: i32 = 26
```

## val REG_X27

```mach
pub val REG_X27: i32 = 27
```

## val REG_X28

```mach
pub val REG_X28: i32 = 28
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

## def ClassifyFn

```mach
pub def ClassifyFn:   abi.ClassifyFn
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

## fun gp_param_reg

```mach
pub fun gp_param_reg(index: i32) i32;
```

## fun fp_param_reg

```mach
pub fun fp_param_reg(index: i32) i32;
```

## fun classify

```mach
pub fun classify(size: u64, align: u64, is_float: bool, fp_eightbytes: u8,
is_aggregate: bool, gp_used: i32, fp_used: i32,
hfa_members: u8, hfa_elem: u8) abi.ParamSlot;
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

