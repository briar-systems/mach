# mach.lang.me.pass.widehelper

## val INLINE_BITS

```mach
pub val INLINE_BITS: u32 = 64
```

the widest integer the legalizer expands in place: a 64-bit value on a 32-bit
ALU (rv32's i64) divides and converts inline, as it always has. an integer
wider than this goes to a helper on every machine: its inline expansion is
prohibitive, and the helpers' lanes are 64-bit (#3511, ruling Q5)

## fun wide_bits

```mach
pub fun wide_bits(m: *ir.Module, tgt: *target.Target, ty: ir_type.IrTypeId) u32;
```

the bit width of a scalar integer wider than both the target's ALU and the
inline limit that the target realizes, else 0: a width the target does not
realize is left for the lowering to refuse at the program's own location

## rec Helper

```mach
pub rec Helper;
```

a helper looked up by name, or added empty with its signature and params

## fun find_or_add

```mach
pub fun find_or_add(m: *ir.Module, itn: *intern.Interner, text: str, ret_ty: ir_type.IrTypeId,
params: *ir_type.IrTypeId, count: u32) res[Helper, fail.Fail];
```

## fun name_width

```mach
pub fun name_width(alloc: *A.Allocator, stem: str, bits: u32) res[str, fail.Fail];
```

`__mach_<stem><bits>`, the name of a helper over one integer width

## fun name_conv

```mach
pub fun name_conv(alloc: *A.Allocator, from: str, from_bits: u32, to: str, to_bits: u32) res[str, fail.Fail];
```

`__mach_<from>_to_<to>`, the name of a conversion helper between two types

## fun rewrite_to_call

```mach
pub fun rewrite_to_call(m: *ir.Module, ins: *instruction.Instruction, callee: u32, args: *value.Value, count: u32) err[fail.Fail];
```

a call in place of the instruction: the given arguments follow the callee,
the result keeps the instruction's own value id and type

## rec Em

```mach
pub rec Em;
```

a straight-line emitter that remembers the first failure, so an algorithm
reads as its arithmetic rather than as a check per operation. a value emitted
after a failure is the nil value and is never used: the caller reads `failed`
before it returns the function

## fun em_init

```mach
pub fun em_init(m: *ir.Module, idx: u32) Em;
```

## fun take

```mach
pub fun take(e: *Em, r: res[value.Value, fail.Fail]) value.Value;
```

## fun add

```mach
pub fun add(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun sub

```mach
pub fun sub(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun mul

```mach
pub fun mul(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun band

```mach
pub fun band(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun bor

```mach
pub fun bor(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun bxor

```mach
pub fun bxor(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun shl

```mach
pub fun shl(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun shr_u

```mach
pub fun shr_u(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun shr_s

```mach
pub fun shr_s(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun eq

```mach
pub fun eq(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun ne

```mach
pub fun ne(e: *Em, a: value.Value, b: value.Value) value.Value;
```

## fun trunc

```mach
pub fun trunc(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun zext

```mach
pub fun zext(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun sext

```mach
pub fun sext(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun bitcast

```mach
pub fun bitcast(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun fp_ext

```mach
pub fun fp_ext(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun ui_to_fp

```mach
pub fun ui_to_fp(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun fp_to_ui

```mach
pub fun fp_to_ui(e: *Em, a: value.Value, to: ir_type.IrTypeId) value.Value;
```

## fun call

```mach
pub fun call(e: *Em, callee: u32, args: *value.Value, count: u32, ret_ty: ir_type.IrTypeId) value.Value;
```

## fun finish

```mach
pub fun finish(e: *Em, out: value.Value) err[fail.Fail];
```

the function's result: the first failure, or the return

