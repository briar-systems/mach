[mach](../README.md) / [lower](README.md) / **value**

# mach.lower.value

a value lowering holds: a mirl value of the function being built, or a constant, a
function or a global the builder makes a value of where an instruction uses it. a
value of an aggregate type is the address of storage holding it, typed as the aggregate,
or, where it is `whole`, a mirl value of the aggregate itself with no storage

<sub>source: [`src/lower/value.mach`](../../../src/lower/value.mach) · imported by 8 modules</sub>

<sub>`id` is `mirl.ir.id` · `wide` is [`mach.base.wide`](../base/wide.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`ValueKind`](#valuekind) | `def` |  |
| [`Value`](#value) | `rec` | space: the address space of the storage an address points into, which a value retyped        as its aggregate keeps held:  what a stack slot was made to hold, none for any other value whole: an aggregate held as a mirl value of its own type, which has no address |

| On `ValueKind` | |
| :--- | :--- |
| [`VAL_NONE`](#valuekind) | none:   no value, what a call of a function that gives nothing yields ssa:    a mirl value of the function being built int:    an integer constant, its bits in `bits` float:  a float constant, the bits of its binary64 encoding in `bits` null:   every bit zero fn:     function `index` of the module, called by name or taken as an address global: the address of global `index` of the module const:  constant `index` of the module, a scalar or an aggregate laid out as its type |
| [`VAL_SSA`](#valuekind) · [`VAL_INT`](#valuekind) · [`VAL_FLOAT`](#valuekind) · [`VAL_NULL`](#valuekind) · [`VAL_FN`](#valuekind) · [`VAL_GLOBAL`](#valuekind) · [`VAL_CONST`](#valuekind) | |

| On `Value` | |
| :--- | :--- |
| [`none`](#none) · [`ssa`](#ssa) | |
| [`whole`](#whole) | the aggregate mirl value `at` of `ty`, held whole with no storage |
| [`zero_whole`](#zero_whole) | the aggregate of `ty` every byte of which is zero, held whole |
| [`const_int`](#const_int) | |
| [`const_int_wide`](#const_int_wide) | a constant up to 128 bits wide; a narrower type reads its low bits |
| [`const_float`](#const_float) · [`const_null`](#const_null) | |
| [`fn`](#fn) | function `index`, an address of `ptr` called through `sig` |
| [`global`](#global) | the address of global `index`, of `ty`, into storage of `space` |
| [`constant`](#constant) · [`retype`](#retype) · [`is_none`](#is_none) | |
| [`int_lit`](#int_lit) | an integer constant's low 64 bits |
| [`is_int_const`](#is_int_const) · [`is_constant`](#is_constant) | |
| [`float_lit`](#float_lit) | a float constant's value |

---

## ValueKind

```mach
pub def ValueKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `VAL_NONE` | `0` | none:   no value, what a call of a function that gives nothing yields ssa:    a mirl value of the function being built int:    an integer constant, its bits in `bits` float:  a float constant, the bits of its binary64 encoding in `bits` null:   every bit zero fn:     function `index` of the module, called by name or taken as an address global: the address of global `index` of the module const:  constant `index` of the module, a scalar or an aggregate laid out as its type |
| `VAL_SSA` | `1` |  |
| `VAL_INT` | `2` |  |
| `VAL_FLOAT` | `3` |  |
| `VAL_NULL` | `4` |  |
| `VAL_FN` | `5` |  |
| `VAL_GLOBAL` | `6` |  |
| `VAL_CONST` | `7` |  |

## Value

```mach
pub rec Value {
    kind:   ValueKind;
    ty:     Type;
    secret: bool;
    id:     id.Value;
    bits:   wide.Wide;
    index:  u32;
    # the signature a function is called through
    sig:   Type;
    space: u32;
    held:  Type;
    # the i1 a bool byte was widened from, which a branch or a select reads in its place
    truth:  id.Value;
    truthy: bool;
    whole:  bool;
}
```

space: the address space of the storage an address points into, which a value retyped
       as its aggregate keeps
held:  what a stack slot was made to hold, none for any other value
whole: an aggregate held as a mirl value of its own type, which has no address

### none

```mach
pub fun none() Value
```

### ssa

```mach
pub fun ssa(at: id.Value, ty: Type, secret: bool) Value
```

### whole

```mach
pub fun whole(at: id.Value, ty: Type, secret: bool) Value
```

the aggregate mirl value `at` of `ty`, held whole with no storage

### zero_whole

```mach
pub fun zero_whole(ty: Type) Value
```

the aggregate of `ty` every byte of which is zero, held whole

### const_int

```mach
pub fun const_int(bits: u64, ty: Type) Value
```

### const_int_wide

```mach
pub fun const_int_wide(bits: wide.Wide, ty: Type) Value
```

a constant up to 128 bits wide; a narrower type reads its low bits

### const_float

```mach
pub fun const_float(f: f64, ty: Type) Value
```

### const_null

```mach
pub fun const_null(ty: Type) Value
```

### fn

```mach
pub fun fn(index: u32, ptr: Type, sig: Type) Value
```

function `index`, an address of `ptr` called through `sig`

### global

```mach
pub fun global(index: u32, ty: Type, space: u32) Value
```

the address of global `index`, of `ty`, into storage of `space`

### constant

```mach
pub fun constant(c: id.Constant, ty: Type) Value
```

### retype

```mach
pub fun retype(v: Value, ty: Type) Value
```

### is_none

```mach
pub fun is_none(v: Value) bool
```

### int_lit

```mach
pub fun int_lit(v: Value) u64
```

an integer constant's low 64 bits

### is_int_const

```mach
pub fun is_int_const(v: Value) bool
```

### is_constant

```mach
pub fun is_constant(v: Value) bool
```

### float_lit

```mach
pub fun float_lit(v: Value) f64
```

a float constant's value

