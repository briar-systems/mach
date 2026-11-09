# mach.lang.fe.comptime.fold

comptime arithmetic through mirl's constant folder: every operation a constant
expression computes is one row of mirl's opcode table folded on constants, so a
constant means at compile time what the same operation means at run time and no
fold rule is stated here. comptime keeps only the language's own rules over the
folded results: which operations it admits, and when a result is out of range

## val WIDTH

```mach
pub val WIDTH: u32 = 128
```

the width comptime integers are computed at: the widest integer the language admits

## tag Kind

```mach
pub tag Kind: u8 {
    integer: u32;
    float:   float.FloatWidth;
}
```

the type an operand or a result is read at

integer: an integer of this many bits
float: a float of this width

## tag Answer

```mach
pub tag Answer: u8 {
    folded: [2]wide.Wide;
    zero;
    overflow;
    nan;
    refused;
}
```

what one fold gives

folded: its results, at most two, each as the low 128 bits of its value: a float's
          are its own encoding at its width
zero: the folder refused it by a division or remainder by zero
overflow: the folder refused it by a quotient that does not fit
nan: the folder refused it by a nan result whose bits the target's rule for
          its format leaves unknown
refused: the folder refused it otherwise, which no request comptime makes can do

## fun fold_op

```mach
pub fun fold_op(op: Opcode, k: Kind, x: [3]wide.Wide, n: u32, to: opt[Kind], nans: target_facts.Nans) Answer;
```

fold `op` over `n` operands of kind `k`, the results read at `out`, with the
immediate type `to` where the row takes one

op: the operation
k: the kind of every operand
x: the operands' bits
n: how many, at most three
to: the kind the row's type immediate states, none for a row with no immediate
nans: the nan the target's float operations return in each format

## fun int

```mach
pub fun int(op: Opcode, x: wide.Wide, y: wide.Wide, n: u32) Answer;
```

an integer operation at comptime's width over one or two operands

## fun flt

```mach
pub fun flt(op: Opcode, w: float.FloatWidth, x: f64, y: f64, n: u32, nans: target_facts.Nans) Answer;
```

a float operation at width `w` over one or two operands, each the f64 that holds
its value at that width, a nan result made by the target's rule `nans`

## fun convert

```mach
pub fun convert(op: Opcode, from: Kind, x: wide.Wide, to: Kind, nans: target_facts.Nans) Answer;
```

a conversion of `x`, read as `from`, to `to` by mirl's conversion row `op`, a nan
result made by the target's rule `nans`

## fun first

```mach
pub fun first(a: Answer) wide.Wide;
```

the first result of a fold the caller knows cannot trap

## fun holds

```mach
pub fun holds(a: Answer, index: u32) bool;
```

whether a one-bit result is set

## fun float_of

```mach
pub fun float_of(a: Answer, w: float.FloatWidth) f64;
```

the float a fold at width `w` gave

## fun at_width

```mach
pub fun at_width(x: wide.Wide, width: u32, signed: bool) wide.Wide;
```

the bits of `x` cut to `width` and extended back to comptime's width: sign
extended for a signed width, zero extended otherwise

