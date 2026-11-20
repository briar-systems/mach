[mach](../README.md) / [lower](README.md) / **irt**

# mach.lower.irt

the mirl types lowering builds: interned in the module's type table, laid out by the
target's data layout and never by mach

Source: [`src/lower/irt.mach`](../../../src/lower/irt.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Functions | |
| :--- | :--- |
| [`is_none`](#is_none) | whether `t` is NONE, the type of what gives nothing |
| [`same`](#same) | whether two interned types are the same type |
| [`int`](#int) | a signless integer of `bits`, rounded up to a width mirl holds |
| [`float`](#float) | a float of `bits` |
| [`ptr`](#ptr) | a pointer into the default address space |
| [`ptr_in`](#ptr_in) | a pointer into the address space `space` |
| [`vector`](#vector) | a vector of `lanes` lanes of `element` |
| [`array`](#array) | an array of `count` elements of `element`, at the stride the data layout gives |
| [`structure_at`](#structure_at) | a structure whose members stand at the offsets mach states, as a constant's byte runs do |
| [`signature`](#signature) | a signature: its parameters and results, each narrow integer with its extension |
| [`restated`](#restated) | the signature `sig` with its `n` parameters of the types at `params`, its results, extensions and convention kept |
| [`variadic`](#variadic) | the signature `sig` taking unnamed arguments after its parameters, as a C function declared with `...` does |
| [`shape`](#shape) | the shape of `ty` as the table holds it, valid until the table next interns a type |
| [`is_int`](#is_int) | whether `ty` is an integer |
| [`int_width`](#int_width) | an integer's width, 0 for any other type |
| [`is_float`](#is_float) | whether `ty` is a float |
| [`float_width`](#float_width) | a float's width, 0 for any other type |
| [`space_of`](#space_of) | the address space a pointer type points into, the default space for any other type |
| [`is_ptr`](#is_ptr) | whether `ty` is a pointer |
| [`is_vector`](#is_vector) | whether `ty` is a vector |
| [`is_function`](#is_function) | whether `ty` is a signature |
| [`is_handle`](#is_handle) | whether `ty` is a handle of any kind |
| [`handle`](#handle) | the handle of the kind `kind`, as its family mints it, made of the `n` operands at `operands` |
| [`handle_of`](#handle_of) | whether `ty` is a handle of the kind `kind` |
| [`is_struct`](#is_struct) | whether `ty` is a structure |
| [`is_array`](#is_array) | whether `ty` is an array |
| [`is_union`](#is_union) | whether `ty` is a union |
| [`is_aggregate`](#is_aggregate) | a structure, union or array: what lowering holds in storage and passes by its address |
| [`is_float_like`](#is_float_like) | a float or a vector of floats |
| [`is_ptr_like`](#is_ptr_like) | a pointer or a vector of pointers |
| [`lanes`](#lanes) | a vector's lane count, 0 for any other type |
| [`element`](#element) | the element of a vector or an array, the type itself for a scalar |
| [`count`](#count) | an array's element count, 0 for any other type |
| [`member_count`](#member_count) | a structure's or union's member count, 0 for any other type |
| [`member`](#member) | member `i` of a structure or union, an array's element, NONE past the members |
| [`result_of`](#result_of) | a signature's first result, NONE when it gives none |
| [`param_count`](#param_count) | a signature's parameter count |
| [`takes_pointer`](#takes_pointer) | whether a signature takes a pointer |
| [`param_of`](#param_of) | parameter `i` of a signature, NONE past them |
| [`size`](#size) | the size in bytes the target's data layout gives `ty` |
| [`align`](#align) | the alignment in bytes the target's data layout gives `ty` |
| [`offset`](#offset) | the byte offset of member `i` of a structure or union |
| [`stride`](#stride) | the bytes from one element of the array `ty` to the next |
| [`padded`](#padded) | whether `ty` lays out a byte no member or element of it covers: a gap between a structure's members or after its last, a union member shorter than the union, or such a byte inside any member or element |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | no type: what a function that gives nothing returns |

---

## Functions

### is_none

```mach
pub fun is_none(t: Type) bool
```

whether `t` is NONE, the type of what gives nothing

| Parameter | |
| :--- | :--- |
| `t` | the type to test |
| **returns** | true when `t` is NONE |

### same

```mach
pub fun same(a: Type, b: Type) bool
```

whether two interned types are the same type

| Parameter | |
| :--- | :--- |
| `a` | one type |
| `b` | the other type |
| **returns** | true when both name one entry of the table |

### int

```mach
pub fun int(t: *mirl.ir.type.table.Table, bits: u32) res[Type, mach.base.fail.Fail]
```

a signless integer of `bits`, rounded up to a width mirl holds

### float

```mach
pub fun float(t: *mirl.ir.type.table.Table, bits: u32) res[Type, mach.base.fail.Fail]
```

a float of `bits`

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `bits` | the float width in bits |
| **returns** | the interned type, or a failure when mirl has no format of that width or the table refuses it |

### ptr

```mach
pub fun ptr(t: *mirl.ir.type.table.Table) res[Type, mach.base.fail.Fail]
```

a pointer into the default address space

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| **returns** | the interned type, or the table's refusal |

### ptr_in

```mach
pub fun ptr_in(t: *mirl.ir.type.table.Table, space: u32) res[Type, mach.base.fail.Fail]
```

a pointer into the address space `space`

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `space` | the address space the pointer points into |
| **returns** | the interned type, or the table's refusal |

### vector

```mach
pub fun vector(t: *mirl.ir.type.table.Table, element: Type, lanes: u32) res[Type, mach.base.fail.Fail]
```

a vector of `lanes` lanes of `element`

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `element` | the lane type |
| `lanes` | the lane count |
| **returns** | the interned type, or the table's refusal |

### array

```mach
pub fun array(t: *mirl.ir.type.table.Table, element: Type, count: u64) res[Type, mach.base.fail.Fail]
```

an array of `count` elements of `element`, at the stride the data layout gives

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `element` | the element type |
| `count` | the element count |
| **returns** | the interned type, or the table's refusal |

### structure_at

```mach
pub fun structure_at(t: *mirl.ir.type.table.Table, members: *Type, offsets: *u64, n: u32) res[Type, mach.base.fail.Fail]
```

a structure whose members stand at the offsets mach states, as a constant's byte runs do

### signature

```mach
pub fun signature(t: *mirl.ir.type.table.Table, params: *Type, param_ext: *mirl.ir.type.table.Extension, pn: u32, results: *Type, result_ext: *mirl.ir.type.table.Extension, rn: u32) res[Type, mach.base.fail.Fail]
```

a signature: its parameters and results, each narrow integer with its extension

### restated

```mach
pub fun restated(t: *mirl.ir.type.table.Table, sig: Type, params: *Type, n: u32) res[Type, mach.base.fail.Fail]
```

the signature `sig` with its `n` parameters of the types at `params`, its results,
extensions and convention kept

### variadic

```mach
pub fun variadic(t: *mirl.ir.type.table.Table, sig: Type) res[Type, mach.base.fail.Fail]
```

the signature `sig` taking unnamed arguments after its parameters, as a C function
declared with `...` does

### shape

```mach
pub fun shape(t: *mirl.ir.type.table.Table, ty: Type) opt[*mirl.ir.type.table.Shape]
```

the shape of `ty` as the table holds it, valid until the table next interns a type

### is_int

```mach
pub fun is_int(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is an integer

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for an integer, false for any other type or NONE |

### int_width

```mach
pub fun int_width(t: *mirl.ir.type.table.Table, ty: Type) u32
```

an integer's width, 0 for any other type

### is_float

```mach
pub fun is_float(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a float

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a float, false for any other type or NONE |

### float_width

```mach
pub fun float_width(t: *mirl.ir.type.table.Table, ty: Type) u32
```

a float's width, 0 for any other type

### space_of

```mach
pub fun space_of(t: *mirl.ir.type.table.Table, ty: Type) u32
```

the address space a pointer type points into, the default space for any other type

### is_ptr

```mach
pub fun is_ptr(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a pointer

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a pointer, false for any other type or NONE |

### is_vector

```mach
pub fun is_vector(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a vector

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a vector, false for any other type or NONE |

### is_function

```mach
pub fun is_function(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a signature

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a function type, false for any other type or NONE |

### is_handle

```mach
pub fun is_handle(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a handle of any kind

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a handle, false for any other type or NONE |

### handle

```mach
pub fun handle(t: *mirl.ir.type.table.Table, kind: View, operands: *mirl.ir.type.table.HandleOperand, n: u32) res[Type, mach.base.fail.Fail]
```

the handle of the kind `kind`, as its family mints it, made of the `n` operands at `operands`

### handle_of

```mach
pub fun handle_of(t: *mirl.ir.type.table.Table, ty: Type, kind: View) bool
```

whether `ty` is a handle of the kind `kind`

### is_struct

```mach
pub fun is_struct(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a structure

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a structure, false for any other type or NONE |

### is_array

```mach
pub fun is_array(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is an array

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for an array, false for any other type or NONE |

### is_union

```mach
pub fun is_union(t: *mirl.ir.type.table.Table, ty: Type) bool
```

whether `ty` is a union

| Parameter | |
| :--- | :--- |
| `t` | the module's type table |
| `ty` | the type to test |
| **returns** | true for a union, false for any other type or NONE |

### is_aggregate

```mach
pub fun is_aggregate(t: *mirl.ir.type.table.Table, ty: Type) bool
```

a structure, union or array: what lowering holds in storage and passes by its address

### is_float_like

```mach
pub fun is_float_like(t: *mirl.ir.type.table.Table, ty: Type) bool
```

a float or a vector of floats

### is_ptr_like

```mach
pub fun is_ptr_like(t: *mirl.ir.type.table.Table, ty: Type) bool
```

a pointer or a vector of pointers

### lanes

```mach
pub fun lanes(t: *mirl.ir.type.table.Table, ty: Type) u32
```

a vector's lane count, 0 for any other type

### element

```mach
pub fun element(t: *mirl.ir.type.table.Table, ty: Type) Type
```

the element of a vector or an array, the type itself for a scalar

### count

```mach
pub fun count(t: *mirl.ir.type.table.Table, ty: Type) u64
```

an array's element count, 0 for any other type

### member_count

```mach
pub fun member_count(t: *mirl.ir.type.table.Table, ty: Type) u32
```

a structure's or union's member count, 0 for any other type

### member

```mach
pub fun member(t: *mirl.ir.type.table.Table, ty: Type, i: u32) Type
```

member `i` of a structure or union, an array's element, NONE past the members

### result_of

```mach
pub fun result_of(t: *mirl.ir.type.table.Table, sig: Type) Type
```

a signature's first result, NONE when it gives none

### param_count

```mach
pub fun param_count(t: *mirl.ir.type.table.Table, sig: Type) u32
```

a signature's parameter count

### takes_pointer

```mach
pub fun takes_pointer(t: *mirl.ir.type.table.Table, sig: Type) bool
```

whether a signature takes a pointer

### param_of

```mach
pub fun param_of(t: *mirl.ir.type.table.Table, sig: Type, i: u32) Type
```

parameter `i` of a signature, NONE past them

### size

```mach
pub fun size(l: *mirl.target.layout.Layout, ty: Type) res[u64, mach.base.fail.Fail]
```

the size in bytes the target's data layout gives `ty`

### align

```mach
pub fun align(l: *mirl.target.layout.Layout, ty: Type) res[u64, mach.base.fail.Fail]
```

the alignment in bytes the target's data layout gives `ty`

### offset

```mach
pub fun offset(l: *mirl.target.layout.Layout, ty: Type, i: u32) res[u64, mach.base.fail.Fail]
```

the byte offset of member `i` of a structure or union

### stride

```mach
pub fun stride(l: *mirl.target.layout.Layout, ty: Type) res[u64, mach.base.fail.Fail]
```

the bytes from one element of the array `ty` to the next

### padded

```mach
pub fun padded(l: *mirl.target.layout.Layout, t: *mirl.ir.type.table.Table, ty: Type) res[bool, mach.base.fail.Fail]
```

whether `ty` lays out a byte no member or element of it covers: a gap between a
structure's members or after its last, a union member shorter than the union, or such a
byte inside any member or element

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `Type` | `0xFFFFFFFF` | no type: what a function that gives nothing returns |

