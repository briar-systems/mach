# mach.lang.me.half

scalar f16 as ordinary ir, written once and shared by every target (#3799).
an f16 value is carried as its 16 bits in an i16. an operation the target
declares in its half rows (isa.half_native) is its own instruction on those
bits; every other one is the expansion here, which calls nothing, so no
runtime symbol appears for a freestanding link to provide.

arithmetic widens both operands exactly to the wide format, binary64 or
binary32 where the model says (#4332), computes there and narrows once. both
carry at least twice binary16's precision plus two bits, binary32 exactly
that much, so that one rounding is the correctly rounded binary16 result, the
rule comptime folds by. a target whose rows convert f16 to and from binary32
computes in binary32 through them (#3801). an arithmetic operand widens with
a NaN kept as it is, signaling or quiet, so the wide unit picks and quiets
the NaN as the target's half unit would. a conversion goes through the wide
format the same way, and a comparison compares the widened values. widening
and narrowing are integer code on the wide encoding in 32-bit words (a
binary64 narrowed under binary32 rounds to odd there first): sign, exponent
and significand extraction, round to nearest even,
subnormals, overflow to infinity, and a converted NaN made by the target's
NaN rule, as float.narrow_bits and float.widen_bits define them, so the
expansion converts as the target's own half instructions would. every step
is branch-free, a select being a mask

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
pub fun init(b: *builder.Builder, tgt: *target.Target) res[Half, fail.Fail];
```

tgt is the target: its machine model's half rows, packed rows and NaN rule

## fun done

```mach
pub fun done(h: *Half, v: value.Value) res[value.Value, fail.Fail];
```

the value, or the first failure on the way to it

## fun wide_bits

```mach
pub fun wide_bits(h: *Half) u32;
```

the width of the format the expansion computes in, 32 or 64

## fun widen

```mach
pub fun widen(h: *Half, x: value.Value) value.Value;
```

the carried bits `x` widened exactly to the wide format, a NaN as the
target's rule converts it

## fun narrow

```mach
pub fun narrow(h: *Half, d: value.Value) value.Value;
```

the wide `d` rounded once, to nearest with ties to even, into the carried bits

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
exact wide widening

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
and overflowing to infinity. the wide format rounds an integer only past
2^24 (binary32) or 2^53 (binary64), but any such value lies past binary16's
range, where the narrowing saturates anyway

## fun to_int

```mach
pub fun to_int(h: *Half, x: value.Value, ity: ir_type.IrTypeId, bits: u32, signed: bool) value.Value;
```

carried bits converted to the integer type `ity` of `bits`, truncating toward
zero; an operand out of range converts as the target's conversion from the
wide format does, the rule every float width follows

## fun vec_arith

```mach
pub fun vec_arith(h: *Half, k: instruction.InstrKind, a: value.Value, b: value.Value) value.Value;
```

the lanes `a k b` of two f16 vectors, k one of OP_ADD, OP_SUB, OP_MUL and
OP_DIV_U, the float division

## fun vec_compare

```mach
pub fun vec_compare(h: *Half, k: instruction.InstrKind, a: value.Value, b: value.Value, mask_ty: ir_type.IrTypeId) value.Value;
```

the lane mask of `a k b` over two f16 vectors, k one of OP_CMP_EQ, OP_CMP_NE,
OP_CMP_LT_U and OP_CMP_LE_U: all ones where it holds. a comparison makes no
NaN and every widening is exact, so the f16 lanes, their binary32 lanes and
each lane's scalar comparison agree

## fun vec_convert

```mach
pub fun vec_convert(h: *Half, v: value.Value, dst: ir_type.IrTypeId, from_fp: bool, from_bits: u32, from_signed: bool,
to_fp: bool, to_bits: u32, to_signed: bool) value.Value;
```

the lane-wise `::` of a vector with f16 lanes on one side or both into
`dst`: `from_fp`, `from_bits` and `from_signed` describe the operand's lanes
and the `to_` ones the result's, as the scalar conversions above take them

