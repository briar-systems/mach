# mach.lang.be.codegen.mir.lower

## fun lower_module

```mach
pub fun lower_module(tgt: *resolved.Target, m: *me_ir.Module, alloc: *A.Allocator, interner: *intern.Interner, srcmap: *lang_source.SourceMap,
diags: *diagnostic.DiagnosticStore) res[codegen_mir.MirModule, fail.Fail];
```

## fun pure_gep_mem

```mach
pub fun pure_gep_mem(ctx: *mir_context.LowerCtx, inst: *ir_instruction.Instruction) opt[codegen_mir.MirOperand];
```

the memory operand of an element address that needs no instruction of its
own: a register, frame or symbol base, constant indices folded into the
displacement, and at most one runtime index whose stride the addressing
mode scales, on a register base. blocks are lowered in index order and the
address may be defined after its uses, so a folded address is computed
from the IR alone, never from lowered state

