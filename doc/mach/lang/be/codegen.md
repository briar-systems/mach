# mach.lang.be.codegen

## rec DebugInfo

```mach
pub rec DebugInfo;
```

## fun no_debug

```mach
pub fun no_debug() DebugInfo;
```

## def EmitModuleFn

```mach
pub def EmitModuleFn: isa.EmitModuleFn
```

## fun codegen_unit

```mach
pub fun codegen_unit(s: *session.Session, tgt: *target.Target,
irmod: *ir.Module, deps: unit.IrSet, asm_out: *writer.Writer,
placement_policy: u32, dbg: DebugInfo,
diags: *diagnostic.DiagnosticStore) res[of.ObjectImage, fail.Fail];
```

the back half of one module. a rejection of the program by any pass is an
error appended to `diags`, the module's diagnostic store, and answers
`reported`; a `message` failure is the apparatus (allocator, I/O, a target
without a model) or a compiler defect

## val PLACEMENT_POLICY_VERSION

```mach
pub val PLACEMENT_POLICY_VERSION: u32 = 1
```

## fun resolve_placement_policy

```mach
pub fun resolve_placement_policy(requested: u32) u32;
```

## fun prepare_debug

```mach
pub fun prepare_debug(s: *session.Session) err[fail.Fail];
```

