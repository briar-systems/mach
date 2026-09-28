# mach.lang.target.isa.riscv.rules

## val RULE_COUNT

```mach
pub val RULE_COUNT: usize = 68
```

## val COPY_OPCODE_COUNT

```mach
pub val COPY_OPCODE_COUNT: usize = 3
```

## val FUSED_CC_COUNT

```mach
pub val FUSED_CC_COUNT: usize = 6
```

## val RULES

```mach
pub val RULES: [RULE_COUNT]rules.Rule = [RULE_COUNT]rules.Rule;
```

## val COPY_OPCODES

```mach
pub val COPY_OPCODES: [COPY_OPCODE_COUNT]u32 = [COPY_OPCODE_COUNT]u32;
```

## val FUSED_CCS

```mach
pub val FUSED_CCS: [FUSED_CC_COUNT]u32 = [FUSED_CC_COUNT]u32;
```

## val PACK

```mach
pub val PACK: rules.RulePack = rules.RulePack;
```

## fun select

```mach
pub fun select(a: *A.Allocator, tgt: *isa.BackendTarget, f: *mir.MirFunction) err[fail.Fail];
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

