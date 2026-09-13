# mach.lang.be.codegen.rules

## def RuleKind

```mach
pub def RuleKind: u8
```

## val RULE_RETAG

```mach
pub val RULE_RETAG:  RuleKind = 0
```

## val RULE_PASS

```mach
pub val RULE_PASS:   RuleKind = 1
```

## val RULE_EXPAND

```mach
pub val RULE_EXPAND: RuleKind = 2
```

## def RuleGate

```mach
pub def RuleGate: u8
```

## val GATE_ANY

```mach
pub val GATE_ANY:    RuleGate = 0
```

## val GATE_SCALAR

```mach
pub val GATE_SCALAR: RuleGate = 1
```

## val GATE_VECTOR

```mach
pub val GATE_VECTOR: RuleGate = 2
```

## def GuardFn

```mach
pub def GuardFn: fun(*isa.BackendTarget, *mir.MirFunction, *mir.MirInstr) bool
```

## rec ExpansionBuilder

```mach
pub rec ExpansionBuilder;
```

## def ExpandFn

```mach
pub def ExpandFn: fun(*ExpansionBuilder) err[fail.Fail]
```

## rec Rule

```mach
pub rec Rule;
```

## rec RulePack

```mach
pub rec RulePack;
```

## fun capture_operand_banks

```mach
pub fun capture_operand_banks(f: *mir.MirFunction, mi: *mir.MirInstr) err[fail.Fail];
```

## fun select_function

```mach
pub fun select_function(pack: *RulePack, a: *A.Allocator, tgt: *isa.BackendTarget, f: *mir.MirFunction) err[fail.Fail];
```

## fun is_reg_move

```mach
pub fun is_reg_move(pack: *RulePack, opcode: u32) bool;
```

## fun find_rule

```mach
pub fun find_rule(pack: *RulePack, tgt: *isa.BackendTarget, f: *mir.MirFunction, mi: *mir.MirInstr) *Rule;
```

## fun emit_instr

```mach
pub fun emit_instr(builder: *ExpansionBuilder, opcode: u32, operands: *mir.MirOperand, operand_count: u32) res[*mir.MirInstr, fail.Fail];
```

## fun operand_rides_fp_bank

```mach
pub fun operand_rides_fp_bank(f: *mir.MirFunction, op: *mir.MirOperand) bool;
```

## fun guard_fp_bank_move

```mach
pub fun guard_fp_bank_move(tgt: *isa.BackendTarget, f: *mir.MirFunction, mi: *mir.MirInstr) bool;
```

