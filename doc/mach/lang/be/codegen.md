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
pub fun codegen_unit(s: *session.Session, tgt: *lang_target.Target,
irmod: *me_ir.Module, deps: be_codegen_unit.IrSet, asm_out: *io_writer.Writer,
dbg: DebugInfo, diags: *diagnostic.DiagnosticStore) res[target_of.ObjectImage, fail.Fail];
```

the back half of one module. a rejection of the program by any pass is an
error appended to `diags`, the module's diagnostic store, and answers
`reported`; a `message` failure is the apparatus (allocator, I/O, a target
without a model) or a compiler defect

## fun prepare_debug

```mach
pub fun prepare_debug(s: *session.Session) err[fail.Fail];
```

the debug view holds a field table for every instance the producer reaches:
every instance of the program and what those reach. An expansive generic has
unboundedly many instances (`rec L[T] { next: *L[*T]; }`), so one minted here
is prepared only when held by value, and the producer describes a pointer to an
unprepared one untyped

