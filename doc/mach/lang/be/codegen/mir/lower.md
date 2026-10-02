# mach.lang.be.codegen.mir.lower

## fun lower_module

```mach
pub fun lower_module(tgt: *resolved.Target, m: *me_ir.Module, alloc: *A.Allocator, interner: *intern.Interner, srcmap: *lang_source.SourceMap,
diags: *diagnostic.DiagnosticStore) res[codegen_mir.MirModule, fail.Fail];
```

