# mach.lang.target.isa.convplan

## def Place

```mach
pub def Place: u8
```

## val PLACE_OPERAND

```mach
pub val PLACE_OPERAND: Place = 0
```

## val PLACE_WORK

```mach
pub val PLACE_WORK:    Place = 1
```

## val PLACE_TEMP

```mach
pub val PLACE_TEMP:    Place = 2
```

## rec Step

```mach
pub rec Step;
```

op:  the target's machine opcode
dst: the place the step writes
src: the place the step reads besides a two-address destination
arg: the target's per-step argument, an immediate or a lane width

## val STEPS_MAX

```mach
pub val STEPS_MAX: u32 = 24
```

the longest sequence any target's table needs, with room to spare

## rec Plan

```mach
pub rec Plan;
```

overflow: a step was dropped because the sequence was full, so the plan must be refused

## fun plan

```mach
pub fun plan() Plan;
```

## fun push

```mach
pub fun push(p: *Plan, op: u32, dst: Place, src: Place, arg: u32);
```

## fun late_operand_read

```mach
pub fun late_operand_read(p: *Plan) u32;
```

the first step that reads the operand after an earlier step wrote the work register, or len for none.
a step that reads the operand and writes the work register in one instruction reads before it writes

## fun alias_safe

```mach
pub fun alias_safe(p: *Plan) bool;
```

whether the work register may be the operand's own register

## fun in_destination

```mach
pub fun in_destination(p: *Plan, dst_is_reg: bool, dst_aliases_operand: bool, dst_is_temp: bool) bool;
```

whether the result forms directly in the destination register

dst_is_reg: the destination is a register, not a frame slot
dst_aliases_operand: the destination register is the operand's register
dst_is_temp: the destination register is the target's temporary

