# mach.lang.be.codegen.mir.abi

## fun require_abi

```mach
pub fun require_abi(tgt: *resolved.Target) err[fail.Fail];
```

## fun abi_gp_arg_reg

```mach
pub fun abi_gp_arg_reg(tgt: *resolved.Target, index: i32, out: *i32) err[fail.Fail];
```

## fun abi_va_model

```mach
pub fun abi_va_model(tgt: *resolved.Target, out: *abi.VaModel) err[fail.Fail];
```

## fun classify_signature

```mach
pub fun classify_signature(ctx: *mir_context.LowerCtx, sig: ir_type.IrTypeId, ret_type: ir_type.IrTypeId,
out: *abi.SigLayout) err[fail.Fail];
```

## fun sig_layout_free

```mach
pub fun sig_layout_free(ctx: *mir_context.LowerCtx, layout: *abi.SigLayout);
```

## fun lower_call

```mach
pub fun lower_call(ctx: *mir_context.LowerCtx, mb: *codegen_mir.MirBlock, inst: *ir_instruction.Instruction, iid: ir_id.InstructionId) err[fail.Fail];
```

## fun lower_params

```mach
pub fun lower_params(ctx: *mir_context.LowerCtx, mb: *codegen_mir.MirBlock) err[fail.Fail];
```

## fun lower_ret

```mach
pub fun lower_ret(ctx: *mir_context.LowerCtx, mb: *codegen_mir.MirBlock, inst: *ir_instruction.Instruction) err[fail.Fail];
```

