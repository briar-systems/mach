# mach.lang.me.analysis.loops

## val NONE_U32

```mach
pub val NONE_U32: u32 = 0xFFFFFFFF
```

## rec InductionVar

```mach
pub rec InductionVar;
```

## rec CountedInfo

```mach
pub rec CountedInfo;
```

## rec Loop

```mach
pub rec Loop;
```

## rec LoopAnalysis

```mach
pub rec LoopAnalysis;
```

the loops of a function over its dominator tree, which the analysis owns
when `analyze` built it and borrows when `analyze_on` was handed it

## fun analyze

```mach
pub fun analyze(fn: *me_ir.Function, types: *ir_type.IrTypeTable, alloc: *A.Allocator) res[LoopAnalysis, fail.Fail];
```

the loops of `fn` over a dominator tree built for them; the type table reads
a counted loop's induction variable at its width

## fun analyze_on

```mach
pub fun analyze_on(dom: *dominance.Dominance, fn: *me_ir.Function, types: *ir_type.IrTypeTable, alloc: *A.Allocator) res[LoopAnalysis, fail.Fail];
```

the loops of `fn` over `dom`, its dominator tree as it stands, which must
outlive the analysis

## fun dnit

```mach
pub fun dnit(la: *LoopAnalysis);
```

## fun loop_contains

```mach
pub fun loop_contains(la: *LoopAnalysis, loop_ix: u32, b: ir_id.BlockId) bool;
```

## fun is_invariant

```mach
pub fun is_invariant(la: *LoopAnalysis, loop_ix: u32, v: value.Value) bool;
```

## fun set_instr_block

```mach
pub fun set_instr_block(la: *LoopAnalysis, iid: ir_id.InstructionId, b: ir_id.BlockId) err[fail.Fail];
```

## fun induction

```mach
pub fun induction(la: *LoopAnalysis, fn: *me_ir.Function, loop_ix: u32, phi_id: ir_id.InstructionId, out: *InductionVar) bool;
```

whether a header phi is an induction, entered as `init` from the preheader
and stepped by an invariant `step` on the latch, recognized as the loop's
own iv is

## fun innermost_first

```mach
pub fun innermost_first(la: *LoopAnalysis, order: *u32);
```

the loop indices deepest first, so a pass that moves code outward meets an
inner loop before the loop around it

