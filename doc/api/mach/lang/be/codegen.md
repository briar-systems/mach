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
placement_policy: u32, dbg: DebugInfo) res[of.ObjectImage, fail.Fail];
```

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

