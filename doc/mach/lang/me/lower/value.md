# mach.lang.me.lower.value

a value lowering holds: a mirl value of the function being built, or a constant, a
function or a global the builder makes a value of where an instruction uses it. a
value of an aggregate type is the address of storage holding it, typed as the aggregate

## def ValueKind

```mach
pub def ValueKind: u8
```

## val VAL_NONE

```mach
pub val VAL_NONE:   ValueKind = 0
```

none:   no value, what a call of a function that gives nothing yields
ssa:    a mirl value of the function being built
int:    an integer constant, its bits in `bits`
float:  a float constant, the bits of its binary64 encoding in `bits`
null:   every bit zero
fn:     function `index` of the module, called by name or taken as an address
global: the address of global `index` of the module
const:  constant `index` of the module, a scalar or an aggregate laid out as its type

## val VAL_SSA

```mach
pub val VAL_SSA:    ValueKind = 1
```

## val VAL_INT

```mach
pub val VAL_INT:    ValueKind = 2
```

## val VAL_FLOAT

```mach
pub val VAL_FLOAT:  ValueKind = 3
```

## val VAL_NULL

```mach
pub val VAL_NULL:   ValueKind = 4
```

## val VAL_FN

```mach
pub val VAL_FN:     ValueKind = 5
```

## val VAL_GLOBAL

```mach
pub val VAL_GLOBAL: ValueKind = 6
```

## val VAL_CONST

```mach
pub val VAL_CONST:  ValueKind = 7
```

## rec Value

```mach
pub rec Value;
```

space: the address space of the storage an address points into, which a value retyped
       as its aggregate keeps
held:  what a stack slot was made to hold, none for any other value

## fun none

```mach
pub fun none() Value;
```

## fun ssa

```mach
pub fun ssa(at: id.Value, ty: Type, secret: bool) Value;
```

## fun const_int

```mach
pub fun const_int(bits: u64, ty: Type) Value;
```

## fun const_int_wide

```mach
pub fun const_int_wide(bits: wide.Wide, ty: Type) Value;
```

a constant up to 128 bits wide; a narrower type reads its low bits

## fun const_float

```mach
pub fun const_float(f: f64, ty: Type) Value;
```

## fun const_null

```mach
pub fun const_null(ty: Type) Value;
```

## fun fn

```mach
pub fun fn(index: u32, ptr: Type, sig: Type) Value;
```

function `index`, an address of `ptr` called through `sig`

## fun global

```mach
pub fun global(index: u32, ty: Type, space: u32) Value;
```

the address of global `index`, of `ty`, into storage of `space`

## fun constant

```mach
pub fun constant(c: id.Constant, ty: Type) Value;
```

## fun retype

```mach
pub fun retype(v: Value, ty: Type) Value;
```

## fun is_none

```mach
pub fun is_none(v: Value) bool;
```

## fun int_lit

```mach
pub fun int_lit(v: Value) u64;
```

an integer constant's low 64 bits

## fun is_int_const

```mach
pub fun is_int_const(v: Value) bool;
```

## fun is_constant

```mach
pub fun is_constant(v: Value) bool;
```

## fun float_lit

```mach
pub fun float_lit(v: Value) f64;
```

a float constant's value

