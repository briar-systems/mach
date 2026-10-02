# mach.lang.target.isa.riscv.rules

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

one addi builds a 12-bit signed value and one lui a 32-bit one whose low 12
bits are zero; any other takes lui and addi or more, the rule the middle end
hoists a loop's constants by (#3807). the value is read sign-extended from
its width, as the materialization reads it

## fun is_trap_terminator

```mach
pub fun is_trap_terminator(opcode: u32) bool;
```

