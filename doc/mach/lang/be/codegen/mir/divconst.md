# mach.lang.be.codegen.mir.divconst

the instruction sequence that divides by a constant: shifts and masks for a
power of two of either sign, and a multiply by the divisor's reciprocal
(Granlund and Montgomery) for any other divisor on a target that declares a
high multiply. a plan is data: the lowering emits its steps as MIR, and
`eval` runs the same steps, which is how every sequence is proven against
the division it replaces (#3350)

## val OPERAND_IMM

```mach
pub val OPERAND_IMM:  u8 = 0xFF
```

an operand slot that names no earlier value: the step's immediate, or no
second operand at all

## val OPERAND_NONE

```mach
pub val OPERAND_NONE: u8 = 0xFE
```

## val MAX_STEPS

```mach
pub val MAX_STEPS: u32 = 10
```

## rec DivStep

```mach
pub rec DivStep;
```

one instruction at the plan's width: value `a` and value `b` (or `imm`).
value 0 is the dividend extended to the width, value i + 1 the result of
step i

## rec DivPlan

```mach
pub rec DivPlan;
```

the steps run at `width` bytes over the dividend of `nbytes`, extended by
its signedness, and the last step's value is the quotient or remainder

## fun plan

```mach
pub fun plan(divisor: wide.Wide, nbytes: u8, signed: bool, rem: bool, pow2_width: u8, mul_hi_width: u32) opt[DivPlan];
```

the plan for dividing an `nbytes` value by `divisor`, or its remainder, or
none when the target's divide is the only sequence: a divisor of magnitude
below 2 (a zero divisor keeps its trap, and -1 its overflow behaviour), or a
divisor that is no power of two on a target without a high multiply at the
dividend's width. `pow2_width` is the width a shift sequence runs at, the
dividend's width raised to the target's ALU floor

