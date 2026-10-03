# mach.lang.be.codegen.mir.abi

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

