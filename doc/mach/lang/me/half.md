# mach.lang.me.half

## rec Half

```mach
pub rec Half;
```

an emitter over the caller's builder that remembers the first failure, so
the algorithms read as their arithmetic. a value emitted after a failure is
the nil value and is never used: the caller reads `done` before its result

## fun carrier

```mach
pub fun carrier(types: *ir_type.IrTypeTable) res[ir_type.IrTypeId, fail.Fail];
```

the ir type an f16 value is carried in: an i16 whose form says its bits are
a binary16, so a calling convention can place it as the float it is (#3800)

## fun init

```mach
pub fun init(b: *builder.Builder, model: *isa.MachineModel) res[Half, fail.Fail];
```

model is the target's machine model: its half rows and its NaN rule

## fun done

```mach
pub fun done(h: *Half, v: value.Value) res[value.Value, fail.Fail];
```

the value, or the first failure on the way to it

## fun widen

```mach
pub fun widen(h: *Half, x: value.Value) value.Value;
```

the carried bits `x` widened exactly to binary64, a NaN as the target's
rule converts it

## fun narrow

```mach
pub fun narrow(h: *Half, d: value.Value) value.Value;
```

the binary64 `d` rounded once, to nearest with ties to even, into the carried bits

## fun neg

```mach
pub fun neg(h: *Half, x: value.Value) value.Value;
```

the carried bits negated: the sign flipped, as every float negation does

## fun arith

```mach
pub fun arith(h: *Half, k: instruction.InstrKind, a: value.Value, b: value.Value) value.Value;
```

`a k b` on carried bits, k one of OP_ADD, OP_SUB, OP_MUL and OP_DIV_U, the
float division

## fun compare_operand

```mach
pub fun compare_operand(h: *Half, x: value.Value) value.Value;
```

the operand a comparison of carried bits reads: the f16 itself where the
target compares at 16 bits, the exact binary32 widening where it converts
there (a comparison makes no NaN, so quieting one changes nothing), else the
exact binary64 widening

## fun from_float

```mach
pub fun from_float(h: *Half, v: value.Value, bits: u32) value.Value;
```

the float `v` of `bits` (32 or 64) converted to carried bits

## fun to_float

```mach
pub fun to_float(h: *Half, x: value.Value, bits: u32) value.Value;
```

carried bits converted exactly to a float of `bits` (32 or 64)

## fun from_int

```mach
pub fun from_int(h: *Half, v: value.Value, bits: u32, signed: bool) value.Value;
```

the integer `v` of `bits` converted to carried bits, rounding to nearest even
and overflowing to infinity. binary64 rounds an integer past 2^53, but any
such value lies past binary16's range, where the narrowing saturates anyway

## fun to_int

```mach
pub fun to_int(h: *Half, x: value.Value, ity: ir_type.IrTypeId, bits: u32, signed: bool) value.Value;
```

carried bits converted to the integer type `ity` of `bits`, truncating toward
zero; an operand out of range converts as the target's binary64 conversion
does, the rule every float width follows

