[mach](../README.md) / [lower](README.md) / **intrinsic**

# mach.lower.intrinsic

an `#[intrinsic]` declaration of the selected target held to the row its call becomes

a call is where a declaration is read against the target's catalog, so it is where the
declaration's types are held to its row: the result it must or must not return, a
constant or literal operand taken as an integer, an image, sampler or sampled image
operand taken as a handle, a pointer an instruction reads or writes through taken as a
pointer, and a handle returned only where the row's result is one. on spir-v the row's
typing in masc (`masc.isa.spirv.typing`) holds each slot further: the number its type is
a scalar or vector of, its shape and widths, and how it relates to another slot's type,
a pointer slot's being the type it points to. a row masc states no typing of is held to
its kinds alone. each break is refused with `op.signature`, naming the parameter

Source: [`src/lower/intrinsic.mach`](../../../src/lower/intrinsic.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.lower.context`](context.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`check`](#check) | what a call's declaration must be against its row, refused with `op.signature` |
| [`result_class`](#result_class) | the storage class a call's pointer result points into where its row's typing relates the result to the texel of an image: the Image class, as OpImageTexelPointer's result points into (spir-v 1.6, OpImageTexelPointer), none for any other row, whose pointer result keeps the default space |

---

## Functions

### check

```mach
pub fun check(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, name: str, r: *masc.core.row.Row, lead: u32, fn: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

what a call's declaration must be against its row, refused with `op.signature`

| Parameter | |
| :--- | :--- |
| `span` | the call |
| `name` | the instruction, as the declaration names it |
| `r` | its row |
| `lead` | the row operands its result, result type or set's import take |
| `fn` | the declaration's function type |

### result_class

```mach
pub fun result_class(ctx: *mach.lower.context.LowerContext, r: *masc.core.row.Row) opt[u32]
```

the storage class a call's pointer result points into where its row's typing relates the result to the texel of an
image: the Image class, as OpImageTexelPointer's result points into (spir-v 1.6, OpImageTexelPointer), none for any
other row, whose pointer result keeps the default space

