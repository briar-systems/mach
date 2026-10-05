# mach.lang.target.isa.rules

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
pub fun capture_operand_banks(f: *lang_mir.MirFunction, mi: *lang_mir.MirInstr) err[fail.Fail];
```

## fun select_function

```mach
pub fun select_function(pack: *RulePack, a: *A.Allocator, tgt: *isa.BackendTarget, f: *lang_mir.MirFunction) err[fail.Fail];
```

## fun is_reg_move

```mach
pub fun is_reg_move(pack: *RulePack, opcode: u32) bool;
```

## fun find_rule

```mach
pub fun find_rule(pack: *RulePack, tgt: *isa.BackendTarget, f: *lang_mir.MirFunction, mi: *lang_mir.MirInstr) *Rule;
```

## fun emit_instr

```mach
pub fun emit_instr(builder: *expansion.ExpansionBuilder, opcode: u32, operands: *lang_mir.MirOperand, operand_count: u32) res[*lang_mir.MirInstr, fail.Fail];
```

## fun guard_fp_bank_move

```mach
pub fun guard_fp_bank_move(tgt: *isa.BackendTarget, f: *lang_mir.MirFunction, mi: *lang_mir.MirInstr) bool;
```

