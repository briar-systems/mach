# mach.lang.me.legalize.wide.helper

the helpers the compiler provides with the program for an operation on an
integer wider than the target's ALU that no target has an instruction for:
division and remainder (div) and the float conversions (conv). a
helper is an ordinary function of the module that needs it, weak so every
module's copy coalesces at link, as a generic instantiation does, and never
inlined so a program pays for the code once. this module holds what the two
passes share: the width test and the function synthesis

## fun bits

```mach
pub fun bits(m: *me_ir.Module, tgt: *resolved.Target, ty: ir_type.IrTypeId) u32;
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
pub fun find_or_add(m: *me_ir.Module, itn: *intern.Interner, text: str, ret_ty: ir_type.IrTypeId,
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
pub fun rewrite_to_call(m: *me_ir.Module, ins: *ir_instruction.Instruction, callee: u32, args: *value.Value, count: u32) err[fail.Fail];
```

a call in place of the instruction: the given arguments follow the callee,
the result keeps the instruction's own value id and type

## fun em_init

```mach
pub fun em_init(m: *me_ir.Module, idx: u32) builder.Builder;
```

a builder over helper `idx` in its sticky mode, so an algorithm reads as its
arithmetic rather than as a check per operation

## fun finish

```mach
pub fun finish(b: *builder.Builder, out: value.Value) err[fail.Fail];
```

the function's result: the first failure, or the return

