# mach.lang.me.pass.wideconv

a conversion between a float and an integer wider than the target's ALU has
no instruction on any target, so each becomes a call to a helper the
compiler provides with the program: `__mach_u128_to_f64`, `__mach_i128_to_f32`,
`__mach_f64_to_u128` and so on, one per pair of types. every conversion is
correctly rounded and branch-free. to a float: the integer is normalized by a
binary search over its leading zeros, its top lane keeps a sticky bit for
everything below, the lane converts natively (exact rounding needs the round
bit and one sticky bit, which a 64-bit lane has room for beyond any float
significand) and the result is scaled by the exact power of two the
normalization removed. from a float: the value splits at 2^64 into two lanes
by exact float arithmetic, each converting natively; a value outside the
integer's range converts as the target's own lane conversion does with an
out-of-range operand, the same contract as every `::` into an integer. a
signed conversion is the unsigned one on the magnitude with the sign
restored. the helpers cover a two-lane width, the widest any target here
realizes (#3511)

## fun run

```mach
pub fun run(m: *me_ir.Module, tgt: *lang_target.Target, itn: *intern.Interner) res[bool, fail.Fail];
```

