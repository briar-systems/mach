# mach.lang.me.lower.irt

the mirl types lowering builds: interned in the module's type table, laid out by the
target's data layout and never by mach

## val NONE

```mach
pub val NONE: Type = Type;
```

no type: what a function that gives nothing returns

## fun is_none

```mach
pub fun is_none(t: Type) bool;
```

## fun same

```mach
pub fun same(a: Type, b: Type) bool;
```

## fun int

```mach
pub fun int(t: *types.Table, bits: u32) res[Type, fail.Fail];
```

a signless integer of `bits`, rounded up to a width mirl holds

## fun float

```mach
pub fun float(t: *types.Table, bits: u32) res[Type, fail.Fail];
```

## fun ptr

```mach
pub fun ptr(t: *types.Table) res[Type, fail.Fail];
```

## fun ptr_in

```mach
pub fun ptr_in(t: *types.Table, space: u32) res[Type, fail.Fail];
```

## fun vector

```mach
pub fun vector(t: *types.Table, element: Type, lanes: u32) res[Type, fail.Fail];
```

## fun array

```mach
pub fun array(t: *types.Table, element: Type, count: u64) res[Type, fail.Fail];
```

## fun structure

```mach
pub fun structure(t: *types.Table, members: *Type, n: u32) res[Type, fail.Fail];
```

## fun structure_at

```mach
pub fun structure_at(t: *types.Table, members: *Type, offsets: *u64, n: u32) res[Type, fail.Fail];
```

a structure whose members stand at the offsets mach states, as a packed record's do

## fun union_of

```mach
pub fun union_of(t: *types.Table, members: *Type, n: u32) res[Type, fail.Fail];
```

## fun handle

```mach
pub fun handle(t: *types.Table, kind: View) res[Type, fail.Fail];
```

## fun signature

```mach
pub fun signature(t: *types.Table, params: *Type, param_ext: *types.Extension, pn: u32, results: *Type, result_ext: *types.Extension, rn: u32) res[Type, fail.Fail];
```

a signature: its parameters and results, each narrow integer with its extension

## fun restated

```mach
pub fun restated(t: *types.Table, sig: Type, params: *Type, n: u32) res[Type, fail.Fail];
```

the signature `sig` with its `n` parameters of the types at `params`, its results,
extensions and convention kept

## fun shape

```mach
pub fun shape(t: *types.Table, ty: Type) opt[types.Shape];
```

## fun is_int

```mach
pub fun is_int(t: *types.Table, ty: Type) bool;
```

## fun int_width

```mach
pub fun int_width(t: *types.Table, ty: Type) u32;
```

an integer's width, 0 for any other type

## fun is_float

```mach
pub fun is_float(t: *types.Table, ty: Type) bool;
```

## fun float_width

```mach
pub fun float_width(t: *types.Table, ty: Type) u32;
```

a float's width, 0 for any other type

## fun space_of

```mach
pub fun space_of(t: *types.Table, ty: Type) u32;
```

the address space a pointer type points into, the default space for any other type

## fun is_ptr

```mach
pub fun is_ptr(t: *types.Table, ty: Type) bool;
```

## fun is_vector

```mach
pub fun is_vector(t: *types.Table, ty: Type) bool;
```

## fun is_function

```mach
pub fun is_function(t: *types.Table, ty: Type) bool;
```

## fun is_handle

```mach
pub fun is_handle(t: *types.Table, ty: Type) bool;
```

## fun is_struct

```mach
pub fun is_struct(t: *types.Table, ty: Type) bool;
```

## fun is_array

```mach
pub fun is_array(t: *types.Table, ty: Type) bool;
```

## fun is_union

```mach
pub fun is_union(t: *types.Table, ty: Type) bool;
```

## fun is_aggregate

```mach
pub fun is_aggregate(t: *types.Table, ty: Type) bool;
```

a structure, union or array: what lowering holds in storage and passes by its address

## fun is_float_like

```mach
pub fun is_float_like(t: *types.Table, ty: Type) bool;
```

a float or a vector of floats

## fun is_ptr_like

```mach
pub fun is_ptr_like(t: *types.Table, ty: Type) bool;
```

a pointer or a vector of pointers

## fun lanes

```mach
pub fun lanes(t: *types.Table, ty: Type) u32;
```

a vector's lane count, 0 for any other type

## fun element

```mach
pub fun element(t: *types.Table, ty: Type) Type;
```

the element of a vector or an array, the type itself for a scalar

## fun count

```mach
pub fun count(t: *types.Table, ty: Type) u64;
```

an array's element count, 0 for any other type

## fun member_count

```mach
pub fun member_count(t: *types.Table, ty: Type) u32;
```

a structure's or union's member count, 0 for any other type

## fun member

```mach
pub fun member(t: *types.Table, ty: Type, i: u32) Type;
```

member `i` of a structure or union, an array's element, NONE past the members

## fun result_of

```mach
pub fun result_of(t: *types.Table, sig: Type) Type;
```

a signature's first result, NONE when it gives none

## fun param_count

```mach
pub fun param_count(t: *types.Table, sig: Type) u32;
```

a signature's parameter count

## fun takes_pointer

```mach
pub fun takes_pointer(t: *types.Table, sig: Type) bool;
```

whether a signature takes a pointer

## fun param_of

```mach
pub fun param_of(t: *types.Table, sig: Type, i: u32) Type;
```

parameter `i` of a signature, NONE past them

## fun size

```mach
pub fun size(l: *mirl_target.Layout, ty: Type) res[u64, fail.Fail];
```

the size in bytes the target's data layout gives `ty`

## fun align

```mach
pub fun align(l: *mirl_target.Layout, ty: Type) res[u64, fail.Fail];
```

the alignment in bytes the target's data layout gives `ty`

## fun offset

```mach
pub fun offset(l: *mirl_target.Layout, ty: Type, i: u32) res[u64, fail.Fail];
```

the byte offset of member `i` of a structure or union

## fun stride

```mach
pub fun stride(l: *mirl_target.Layout, ty: Type) res[u64, fail.Fail];
```

the bytes from one element of the array `ty` to the next

