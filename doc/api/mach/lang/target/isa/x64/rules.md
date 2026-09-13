# mach.lang.target.isa.x64.rules

## val RULE_COUNT

```mach
pub val RULE_COUNT: usize = 60
```

## val COPY_OPCODE_COUNT

```mach
pub val COPY_OPCODE_COUNT: usize = 2
```

## val FUSED_CC_COUNT

```mach
pub val FUSED_CC_COUNT: usize = 6
```

## fun select

```mach
pub fun select(a: *A.Allocator, tgt: *isa.BackendTarget, f: *mir.MirFunction) err[fail.Fail];
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

