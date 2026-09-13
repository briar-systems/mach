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

dom_pre and dom_post number the dominator tree in one depth-first walk, so
a dominance query is an interval test rather than a walk up the idom chain

## fun analyze

```mach
pub fun analyze(fn: *ir.Function, alloc: *A.Allocator) res[LoopAnalysis, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(la: *LoopAnalysis);
```

## fun dominates

```mach
pub fun dominates(la: *LoopAnalysis, a: id.BlockId, b: id.BlockId) bool;
```

## fun loop_contains

```mach
pub fun loop_contains(la: *LoopAnalysis, loop_ix: u32, b: id.BlockId) bool;
```

## fun is_invariant

```mach
pub fun is_invariant(la: *LoopAnalysis, loop_ix: u32, v: value.Value) bool;
```

## fun set_instr_block

```mach
pub fun set_instr_block(la: *LoopAnalysis, iid: id.InstructionId, b: id.BlockId) err[fail.Fail];
```

