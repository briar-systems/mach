[mach](../README.md) / [lower](README.md) / **intrinsic**

# mach.lower.intrinsic

an `#[intrinsic]` declaration of the selected target held to the row its calls become

a declaration is held to its row where it is declared, whether or not anything calls it:
its parameter count, the result it must or must not return, a constant or literal operand
taken as an integer, an image, sampler or sampled image operand taken as a handle, a pointer
an instruction reads or writes through taken as a pointer, and a handle returned only where
the row's result is one. on spir-v the row's typing in masc (`masc.isa.spirv.typing`) holds
each slot further: the number its type is a scalar or vector of, its shape and widths, how
it relates to another slot's type, a pointer slot's being the type it points to, the
members of a record it is, and the types a typed row's data operand or result may be. a row
masc states no typing of is held to its operand kinds alone, and its types are the author's
to get right. each break is refused with `op.signature`, naming the parameter. a call holds
what depends on its arguments: an instruction the catalog does not hold or holds as no
`intrinsic`, the constants an index chain reads, the member a literal indexes, and in mirl the constants, enumerant
values, image operands, storage classes and capabilities of the call

Source: [`src/lower/intrinsic.mach`](../../../src/lower/intrinsic.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.front.sema.interface`](../front/sema/interface.md), [`mach.lower.context`](context.md), [`mach.lower.written`](written.md), [`mach.type.kind`](../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`Found`](#found) | `rec` | the instruction an `#[intrinsic]` names in the selected target's catalog |
| [`Named`](#named) | `tag` | what the selected target's catalog holds of the instruction an `#[intrinsic]` names |
| [`LastOf`](#lastof) | `rec` | a literal operand whose value a row's typing relates to a record, and the operand the record is, or points to |

| On `Named` | |
| :--- | :--- |
| [`named`](#named-1) | what the selected target's catalog holds of instruction `name` of set `set` |

| On `LastOf` | |
| :--- | :--- |
| [`last_of`](#last_of) | the literal a call's row indexes the last member of a record with, as OpArrayLength's Array member indexes the run-time array that ends the record its Structure points to |
| [`last_held`](#last_held) | a call's literal held to the record it indexes the last member of, the literal's argument folding to `value` |

| Functions | |
| :--- | :--- |
| [`declared`](#declared) | the `#[intrinsic]` declaration `op` of the selected target, of function type `fn`, held to its row where it is declared, at `span` |
| [`withdrawn`](#withdrawn) | a call of row `r`, named `name` at `span`, refused where masc's typing states that no environment admits the row, as GLSL.std.450 removed IMix |
| [`admitted`](#admitted) | a call of row `r`, named `name` at `span`, refused with `spirv.capability` where the selection the calling function compiles under, the build's widened by the function's own extensions, does not hold what the row requires, as SPIR-V 1.0 holds no subgroup operation |
| [`ends`](#ends) | whether row `r` ends its block, nothing falling through after it, as OpKill and OpTerminateInvocation |
| [`result_class`](#result_class) | the storage class a call's pointer result points into where its row's typing states it: the Image class where it relates the result to the texel of an image, as OpImageTexelPointer's result points into (spir-v 1.6, OpImageTexelPointer), PhysicalStorageBuffer where the result is a physical pointer, as OpConvertUToPtr makes |
| [`chain_base`](#chain_base) | the row operand a call's pointer result is chained from where its row's typing relates the result to an index chain, as OpInBoundsAccessChain's to its Base, the result pointing into the storage class that operand points into |
| [`chained`](#chained) | the pointer result of a chained row held to what its indexes reach: from what parameter `base` points to, each parameter after it steps into an array's element, a vector's component, or the member of a record its constant names, and the return type points to what the last step reaches |

---

## Found

```mach
pub rec Found {
    set:  *mirl.target.capability.OpSet;
    row:  *masc.core.row.Row;
    lead: u32;
}
```

the instruction an `#[intrinsic]` names in the selected target's catalog

| Field | Type | |
| :--- | :--- | :--- |
| `set` | `*mirl.target.capability.OpSet` | its set |
| `row` | `*masc.core.row.Row` | its row |
| `lead` | `u32` | the row operands its result, result type or set's import take |

## Named

```mach
pub tag Named: u8 {
    no_catalog;
    no_set;
    refused: mach.lower.written.Refusal;
    no_row;
    found: Found;
}
```

what the selected target's catalog holds of the instruction an `#[intrinsic]` names

| Case | Payload | |
| :--- | :--- | :--- |
| `no_catalog` |  | the catalog holds no instruction set |
| `no_set` |  | it holds no set of the name |
| `refused` | `mach.lower.written.Refusal` | the set holds the instruction, but no `intrinsic` may name it, for the reason given |
| `no_row` |  | the set holds no instruction of the name |
| `found` | `Found` | the instruction |

### named

```mach
pub fun named(ctx: *mach.lower.context.LowerContext, set: str, name: str) Named
```

what the selected target's catalog holds of instruction `name` of set `set`

## LastOf

```mach
pub rec LastOf {
    literal: u8;
    record:  u8;
}
```

a literal operand whose value a row's typing relates to a record, and the operand the record is, or points to

| Field | Type | |
| :--- | :--- | :--- |
| `literal` | `u8` | the row operand of the literal |
| `record` | `u8` | the row operand whose record the literal indexes the last member of |

### last_of

```mach
pub fun last_of(ctx: *mach.lower.context.LowerContext, r: *masc.core.row.Row) opt[LastOf]
```

the literal a call's row indexes the last member of a record with, as OpArrayLength's Array member indexes the run-time
array that ends the record its Structure points to. none for any other row

### last_held

```mach
pub fun last_held(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, name: str, fn: mach.type.kind.TypeId, lead: u32, at: LastOf, value: u64) err[mach.base.fail.Fail]
```

a call's literal held to the record it indexes the last member of, the literal's argument folding to `value`. the
declaration holds the record's shape, so a declaration refused there or one whose operand is no record is left to it

## Functions

### declared

```mach
pub fun declared(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, op: mach.front.sema.interface.Native, fn: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

the `#[intrinsic]` declaration `op` of the selected target, of function type `fn`, held to
its row where it is declared, at `span`. one whose instruction the catalog does not hold, or
holds as no `intrinsic`, is refused at each call instead, where the program uses it

### withdrawn

```mach
pub fun withdrawn(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, name: str, r: *masc.core.row.Row) err[mach.base.fail.Fail]
```

a call of row `r`, named `name` at `span`, refused where masc's typing states that no
environment admits the row, as GLSL.std.450 removed IMix. a declaration of it builds

### admitted

```mach
pub fun admitted(ctx: *mach.lower.context.LowerContext, span: mach.base.source.Span, name: str, r: *masc.core.row.Row) err[mach.base.fail.Fail]
```

a call of row `r`, named `name` at `span`, refused with `spirv.capability` where the selection
the calling function compiles under, the build's widened by the function's own extensions,
does not hold what the row requires, as SPIR-V 1.0 holds no subgroup operation. mirl's
verifier holds every target op to this rule, so the call is refused here, at the call,
naming the atom the back end would name. a function whose extensions the target cannot
widen by is refused where its extensions are checked, so it is held to the build's alone

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

