# mach.lang.target.isa.arm64.rules

## val RULE_COUNT

```mach
pub val RULE_COUNT: usize = 100
```

## val COPY_OPCODE_COUNT

```mach
pub val COPY_OPCODE_COUNT: usize = 3
```

## val FUSED_CC_COUNT

```mach
pub val FUSED_CC_COUNT: usize = 6
```

## fun select

```mach
pub fun select(a: *std_allocator.Allocator, tgt: *isa.BackendTarget, f: *codegen_mir.MirFunction) err[fail.Fail];
```

## fun is_reg_move

```mach
pub fun is_reg_move(opcode: u32) bool;
```

## fun is_trap_terminator

```mach
pub fun is_trap_terminator(opcode: u32) bool;
```

## val RULES

```mach
pub val RULES: [RULE_COUNT]codegen_rules.Rule = [RULE_COUNT]codegen_rules.Rule;
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
pub val PACK: codegen_rules.RulePack = codegen_rules.RulePack;
```

