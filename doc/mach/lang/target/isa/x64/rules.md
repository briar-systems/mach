# mach.lang.target.isa.x64.rules

## fun select

```mach
pub fun select(a: *A.Allocator, tgt: *isa.BackendTarget, f: *codegen_mir.MirFunction) err[fail.Fail];
```

## fun is_reg_move

```mach
pub fun is_reg_move(opcode: u32) bool;
```

## fun int_imm_fits

```mach
pub fun int_imm_fits(value: u64, bits: u32) bool;
```

one instruction takes an integer whose sign extension from 32 bits is
itself as its immediate; any other is a movabs of its own first, the rule
the middle end hoists a loop's constants by (#3807)

## fun is_trap_terminator

```mach
pub fun is_trap_terminator(opcode: u32) bool;
```

