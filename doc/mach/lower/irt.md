[mach](../README.md) / [lower](README.md) / **irt**

# mach.lower.irt

the mirl types lowering builds: interned in the module's type table, laid out by the
target's data layout and never by mach

<sub>source: [`src/lower/irt.mach`](../../../src/lower/irt.mach) · imported by 8 modules</sub>

<sub>`types` is `mirl.ir.type.table` · `fail` is [`mach.base.fail`](../base/fail.md) · `mirl_layout` is `mirl.target.layout`</sub>

| Functions | |
| :--- | :--- |
| [`is_none`](#is_none) · [`same`](#same) | |
| [`int`](#int) | a signless integer of `bits`, rounded up to a width mirl holds |
| [`float`](#float) · [`ptr`](#ptr) · [`ptr_in`](#ptr_in) · [`vector`](#vector) · [`array`](#array) | |
| [`structure_at`](#structure_at) | a structure whose members stand at the offsets mach states, as a constant's byte runs do |
| [`signature`](#signature) | a signature: its parameters and results, each narrow integer with its extension |
| [`restated`](#restated) | the signature `sig` with its `n` parameters of the types at `params`, its results, extensions and convention kept |
| [`variadic`](#variadic) | the signature `sig` taking unnamed arguments after its parameters, as a C function declared with `...` does |
| [`shape`](#shape) | the shape of `ty` as the table holds it, valid until the table next interns a type |
| [`is_int`](#is_int) | |
| [`int_width`](#int_width) | an integer's width, 0 for any other type |
| [`is_float`](#is_float) | |
| [`float_width`](#float_width) | a float's width, 0 for any other type |
| [`space_of`](#space_of) | the address space a pointer type points into, the default space for any other type |
| [`is_ptr`](#is_ptr) · [`is_vector`](#is_vector) · [`is_function`](#is_function) · [`is_handle`](#is_handle) | |
| [`handle`](#handle) | the handle of the kind `kind`, as its family mints it, made of the `n` operands at `operands` |
| [`handle_of`](#handle_of) | whether `ty` is a handle of the kind `kind` |
| [`is_struct`](#is_struct) · [`is_array`](#is_array) · [`is_union`](#is_union) | |
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

### same

```mach
pub fun same(a: Type, b: Type) bool
```

### int

```mach
pub fun int(t: *types.Table, bits: u32) res[Type, fail.Fail]
```

a signless integer of `bits`, rounded up to a width mirl holds

### float

```mach
pub fun float(t: *types.Table, bits: u32) res[Type, fail.Fail]
```

### ptr

```mach
pub fun ptr(t: *types.Table) res[Type, fail.Fail]
```

### ptr_in

```mach
pub fun ptr_in(t: *types.Table, space: u32) res[Type, fail.Fail]
```

### vector

```mach
pub fun vector(t: *types.Table, element: Type, lanes: u32) res[Type, fail.Fail]
```

### array

```mach
pub fun array(t: *types.Table, element: Type, count: u64) res[Type, fail.Fail]
```

### structure_at

```mach
pub fun structure_at(t: *types.Table, members: *Type, offsets: *u64, n: u32) res[Type, fail.Fail]
```

a structure whose members stand at the offsets mach states, as a constant's byte runs do

### signature

```mach
pub fun signature(t: *types.Table, params: *Type, param_ext: *types.Extension, pn: u32, results: *Type, result_ext: *types.Extension, rn: u32) res[Type, fail.Fail]
```

a signature: its parameters and results, each narrow integer with its extension

### restated

```mach
pub fun restated(t: *types.Table, sig: Type, params: *Type, n: u32) res[Type, fail.Fail]
```

the signature `sig` with its `n` parameters of the types at `params`, its results,
extensions and convention kept

### variadic

```mach
pub fun variadic(t: *types.Table, sig: Type) res[Type, fail.Fail]
```

the signature `sig` taking unnamed arguments after its parameters, as a C function
declared with `...` does

### shape

```mach
pub fun shape(t: *types.Table, ty: Type) opt[*types.Shape]
```

the shape of `ty` as the table holds it, valid until the table next interns a type

### is_int

```mach
pub fun is_int(t: *types.Table, ty: Type) bool
```

### int_width

```mach
pub fun int_width(t: *types.Table, ty: Type) u32
```

an integer's width, 0 for any other type

### is_float

```mach
pub fun is_float(t: *types.Table, ty: Type) bool
```

### float_width

```mach
pub fun float_width(t: *types.Table, ty: Type) u32
```

a float's width, 0 for any other type

### space_of

```mach
pub fun space_of(t: *types.Table, ty: Type) u32
```

the address space a pointer type points into, the default space for any other type

### is_ptr

```mach
pub fun is_ptr(t: *types.Table, ty: Type) bool
```

### is_vector

```mach
pub fun is_vector(t: *types.Table, ty: Type) bool
```

### is_function

```mach
pub fun is_function(t: *types.Table, ty: Type) bool
```

### is_handle

```mach
pub fun is_handle(t: *types.Table, ty: Type) bool
```

### handle

```mach
pub fun handle(t: *types.Table, kind: View, operands: *types.HandleOperand, n: u32) res[Type, fail.Fail]
```

the handle of the kind `kind`, as its family mints it, made of the `n` operands at `operands`

### handle_of

```mach
pub fun handle_of(t: *types.Table, ty: Type, kind: View) bool
```

whether `ty` is a handle of the kind `kind`

### is_struct

```mach
pub fun is_struct(t: *types.Table, ty: Type) bool
```

### is_array

```mach
pub fun is_array(t: *types.Table, ty: Type) bool
```

### is_union

```mach
pub fun is_union(t: *types.Table, ty: Type) bool
```

### is_aggregate

```mach
pub fun is_aggregate(t: *types.Table, ty: Type) bool
```

a structure, union or array: what lowering holds in storage and passes by its address

### is_float_like

```mach
pub fun is_float_like(t: *types.Table, ty: Type) bool
```

a float or a vector of floats

### is_ptr_like

```mach
pub fun is_ptr_like(t: *types.Table, ty: Type) bool
```

a pointer or a vector of pointers

### lanes

```mach
pub fun lanes(t: *types.Table, ty: Type) u32
```

a vector's lane count, 0 for any other type

### element

```mach
pub fun element(t: *types.Table, ty: Type) Type
```

the element of a vector or an array, the type itself for a scalar

### count

```mach
pub fun count(t: *types.Table, ty: Type) u64
```

an array's element count, 0 for any other type

### member_count

```mach
pub fun member_count(t: *types.Table, ty: Type) u32
```

a structure's or union's member count, 0 for any other type

### member

```mach
pub fun member(t: *types.Table, ty: Type, i: u32) Type
```

member `i` of a structure or union, an array's element, NONE past the members

### result_of

```mach
pub fun result_of(t: *types.Table, sig: Type) Type
```

a signature's first result, NONE when it gives none

### param_count

```mach
pub fun param_count(t: *types.Table, sig: Type) u32
```

a signature's parameter count

### takes_pointer

```mach
pub fun takes_pointer(t: *types.Table, sig: Type) bool
```

whether a signature takes a pointer

### param_of

```mach
pub fun param_of(t: *types.Table, sig: Type, i: u32) Type
```

parameter `i` of a signature, NONE past them

### size

```mach
pub fun size(l: *mirl_layout.Layout, ty: Type) res[u64, fail.Fail]
```

the size in bytes the target's data layout gives `ty`

### align

```mach
pub fun align(l: *mirl_layout.Layout, ty: Type) res[u64, fail.Fail]
```

the alignment in bytes the target's data layout gives `ty`

### offset

```mach
pub fun offset(l: *mirl_layout.Layout, ty: Type, i: u32) res[u64, fail.Fail]
```

the byte offset of member `i` of a structure or union

### stride

```mach
pub fun stride(l: *mirl_layout.Layout, ty: Type) res[u64, fail.Fail]
```

the bytes from one element of the array `ty` to the next

### padded

```mach
pub fun padded(l: *mirl_layout.Layout, t: *types.Table, ty: Type) res[bool, fail.Fail]
```

whether `ty` lays out a byte no member or element of it covers: a gap between a
structure's members or after its last, a union member shorter than the union, or such a
byte inside any member or element

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `Type` | `0xFFFFFFFF` | no type: what a function that gives nothing returns |

