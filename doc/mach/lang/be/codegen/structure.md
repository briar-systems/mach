# mach.lang.be.codegen.structure

## fun analyze

```mach
pub fun analyze(mf: *lang_mir.MirFunction, alloc: *A.Allocator) res[mir_structure.Structure, fail.Fail];
```

## fun analyze_module

```mach
pub fun analyze_module(m: *lang_mir.MirModule, alloc: *A.Allocator) res[*mir_structure.Structure, fail.Fail];
```

the structure of every function of a MIR module, in function order. a
function the target cannot structure carries its refusal in its status, which
the emitter reports where it emits that function

## fun dominates

```mach
pub fun dominates(st: *mir_structure.Structure, a: u32, b: u32) bool;
```

