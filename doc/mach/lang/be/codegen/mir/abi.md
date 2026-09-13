# mach.lang.be.codegen.mir.abi

## fun require_abi

```mach
pub fun require_abi(tgt: *target.Target) err[fail.Fail];
```

## fun abi_gp_arg_reg

```mach
pub fun abi_gp_arg_reg(tgt: *target.Target, index: i32, out: *i32) err[fail.Fail];
```

## fun abi_va_model

```mach
pub fun abi_va_model(tgt: *target.Target, out: *abi.VaModel) err[fail.Fail];
```

## fun classify_signature

```mach
pub fun classify_signature(ctx: *context.LowerCtx, sig: type.IrTypeId, ret_type: type.IrTypeId,
out: *abi.SigLayout) err[fail.Fail];
```

## fun sig_layout_free

```mach
pub fun sig_layout_free(ctx: *context.LowerCtx, layout: *abi.SigLayout);
```

## fun lower_call

```mach
pub fun lower_call(ctx: *context.LowerCtx, mb: *mir.MirBlock, inst: *instruction.Instruction, iid: id.InstructionId) err[fail.Fail];
```

## fun lower_params

```mach
pub fun lower_params(ctx: *context.LowerCtx, mb: *mir.MirBlock) err[fail.Fail];
```

## fun lower_ret

```mach
pub fun lower_ret(ctx: *context.LowerCtx, mb: *mir.MirBlock, inst: *instruction.Instruction) err[fail.Fail];
```

