# mach.lang.be.codegen.mir.lower

## fun lower_module

```mach
pub fun lower_module(tgt: *target.Target, m: *ir.Module, alloc: *A.Allocator, interner: *intern.Interner, srcmap: *source.SourceMap,
diags: *diagnostic.DiagnosticStore) res[mir.MirModule, fail.Fail];
```

## fun pure_gep_mem

```mach
pub fun pure_gep_mem(ctx: *lctx.LowerCtx, inst: *instr.Instruction) opt[mir.MirOperand];
```

the memory operand of an element address that needs no instruction of its
own: a register, frame or symbol base, constant indices folded into the
displacement, and at most one runtime index whose stride the addressing
mode scales, on a register base. blocks are lowered in index order and the
address may be defined after its uses, so a folded address is computed
from the IR alone, never from lowered state

