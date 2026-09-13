# mach.lang.target.abi.riscv

## rec RiscvAbi

```mach
pub rec RiscvAbi;
```

## val VARIADIC_RULE_UNDECLARED

```mach
pub val VARIADIC_RULE_UNDECLARED: u32 = 0xFFFFFFFF
```

## fun rv_abi

```mach
pub fun rv_abi(id: u32, name: str, arch_id: u32, xlen_bits: u32, flen_bits: u32,
int_reg_count: u32, gp_arg_count: i32, fp_arg_count: i32,
gp_callee_saved_count: i32, fp_callee_saved_count: i32,
variadic_float_bits: u32) RiscvAbi;
```

## val REG_A0

```mach
pub val REG_A0: i32 = 10
```

## val REG_A1

```mach
pub val REG_A1: i32 = 11
```

## val REG_S1

```mach
pub val REG_S1: i32 = 9
```

## val REG_S2

```mach
pub val REG_S2: i32 = 18
```

## val STACK_ALIGN

```mach
pub val STACK_ALIGN: u32 = 16
```

## val RED_ZONE

```mach
pub val RED_ZONE:    u32 = 0
```

## fun v_lp64

```mach
pub fun v_lp64() RiscvAbi;
```

## fun v_lp64f

```mach
pub fun v_lp64f() RiscvAbi;
```

## fun v_lp64d

```mach
pub fun v_lp64d() RiscvAbi;
```

## fun v_ilp32

```mach
pub fun v_ilp32() RiscvAbi;
```

## fun v_ilp32f

```mach
pub fun v_ilp32f() RiscvAbi;
```

## fun v_ilp32d

```mach
pub fun v_ilp32d() RiscvAbi;
```

## fun xlen_bytes

```mach
pub fun xlen_bytes(v: RiscvAbi) u64;
```

## fun max_reg_ret

```mach
pub fun max_reg_ret(v: RiscvAbi) u64;
```

## fun float_in_fp_bank

```mach
pub fun float_in_fp_bank(v: RiscvAbi, width: u64) bool;
```

## fun gp_param_reg

```mach
pub fun gp_param_reg(v: RiscvAbi, index: i32) i32;
```

## fun fp_param_reg

```mach
pub fun fp_param_reg(v: RiscvAbi, index: i32) i32;
```

## fun classify_arg_v

```mach
pub fun classify_arg_v(v: RiscvAbi, index: i32, size: u64, align: u64,
is_float: bool, fp_eightbytes: u8, is_aggregate: bool,
is_vector: bool,
gp_used: i32, fp_used: i32, hfa_members: u8, hfa_elem: u8,
agg: abi.AggLayout) abi.ParamSlot;
```

## fun classify_return_v

```mach
pub fun classify_return_v(v: RiscvAbi, size: u64, align: u64,
is_float: bool, fp_eightbytes: u8, is_aggregate: bool,
is_vector: bool, hfa_members: u8, hfa_elem: u8,
agg: abi.AggLayout) abi.ParamSlot;
```

## fun gp_param_regs_v

```mach
pub fun gp_param_regs_v(v: RiscvAbi, out: *isa.Register) i32;
```

## fun callee_saved_v

```mach
pub fun callee_saved_v(v: RiscvAbi, out: *isa.Register) i32;
```

## fun va_model_v

```mach
pub fun va_model_v(v: RiscvAbi) abi.VaModel;
```

## fun register

```mach
pub fun register(reg: *abi.AbiRegistry) err[fail.Fail];
```

