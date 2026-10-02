# mach.lang.target.isa.arm64.rules

## fun select

```mach
pub fun select(a: *A.Allocator, tgt: *isa.BackendTarget, f: *codegen_mir.MirFunction) err[fail.Fail];
```

## fun is_reg_move

```mach
pub fun is_reg_move(opcode: u32) bool;
```

## fun is_trap_terminator

```mach
pub fun is_trap_terminator(opcode: u32) bool;
```

