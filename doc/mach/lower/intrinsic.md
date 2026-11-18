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
its operand kinds alone, and its types are the author's to get right. each break is
refused with `op.signature`, naming the parameter

Source: [`src/lower/intrinsic.mach`](../../../src/lower/intrinsic.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.lower.context`](context.md), [`mach.type.kind`](../type/kind.md).

| Functions | |
| :--- | :--- |
| [`check`](#check) | what a call's declaration must be against its row, refused with `op.signature` |
| [`ends`](#ends) | whether row `r` ends its block, nothing falling through after it, as OpKill and OpTerminateInvocation |
| [`result_class`](#result_class) | the storage class a call's pointer result points into where its row's typing states it: the Image class where it relates the result to the texel of an image, as OpImageTexelPointer's result points into (spir-v 1.6, OpImageTexelPointer), PhysicalStorageBuffer where the result is a physical pointer, as OpConvertUToPtr makes |
| [`chain_base`](#chain_base) | the row operand a call's pointer result is chained from where its row's typing relates the result to an index chain, as OpInBoundsAccessChain's to its Base, the result pointing into the storage class that operand points into |
| [`chained`](#chained) | the pointer result of a chained row held to what its indexes reach: from what parameter `base` points to, each parameter after it steps into an array's element, a vector's component, or the member of a record its constant names, and the return type points to what the last step reaches |

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

### ends

```mach
pub fun ends(r: *masc.core.row.Row) bool
```

whether row `r` ends its block, nothing falling through after it, as OpKill and OpTerminateInvocation

### result_class

```mach
pub fun result_class(ctx: *mach.lower.context.LowerContext, r: *masc.core.row.Row) opt[u32]
```

the storage class a call's pointer result points into where its row's typing states it: the Image class where it
relates the result to the texel of an image, as OpImageTexelPointer's result points into (spir-v 1.6,
OpImageTexelPointer), PhysicalStorageBuffer where the result is a physical pointer, as OpConvertUToPtr makes. none for
any other row, whose pointer result keeps the default space or takes its base's (`chain_base`)

### chain_base

```mach
pub fun chain_base(ctx: *mach.lower.context.LowerContext, r: *masc.core.row.Row) opt[u8]
```

the row operand a call's pointer result is chained from where its row's typing relates the result to an index chain,
as OpInBoundsAccessChain's to its Base, the result pointing into the storage class that operand points into. none for
any other row

### chained

```mach
pub fun chained(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, name: str, fn: mach.type.kind.TypeId, lead: u32, base: u8, indexes: *opt[u64], n: u32) err[mach.base.fail.Fail]
```

the pointer result of a chained row held to what its indexes reach: from what parameter `base` points to, each
parameter after it steps into an array's element, a vector's component, or the member of a record its constant names,
and the return type points to what the last step reaches. `indexes` holds the constant each index parameter's argument
folds to, none for one known only at run time

