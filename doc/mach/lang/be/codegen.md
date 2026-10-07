# mach.lang.be.codegen

## rec DebugInfo

```mach
pub rec DebugInfo;
```

## fun no_debug

```mach
pub fun no_debug() DebugInfo;
```

## fun codegen_unit

```mach
pub fun codegen_unit(s: *session.Session, tgt: *lang_target.Binding,
irmod: *me_ir.Module, deps: mir_unit.IrSet, asm_out: *io_writer.Writer,
dbg: DebugInfo, diags: *diagnostic.DiagnosticStore) res[target_of.ObjectImage, fail.Fail];
```

the back half of one module. a rejection of the program by any pass is an
error appended to `diags`, the module's diagnostic store, and answers
`reported`; a `message` failure is the apparatus (allocator, I/O, a target
without a model) or a compiler defect
the module's scratch arena grows from the session's backing allocator

