[mach](../README.md) / [type](README.md) / **prim**

# mach.type.prim

the primitive types: their catalog, widths, names and integer ranges, and vector spellings

a kind is primitive exactly when the catalog names it, never by its place in the numbering. the
vectors the type table seeds, and the lanes a name such as `u32x4` spells, are read here too.

Source: [`src/type/prim.mach`](../../../src/type/prim.mach)

Uses types from [`mach.base.float`](../base/float.md), [`mach.base.wide`](../base/wide.md).

| Type | | |
| :--- | :--- | :--- |
| [`PrimClass`](#primclass) | `def` |  |
| [`PrimDesc`](#primdesc) | `rec` | bits:  what a value occupies, in memory and as an operand width: the bits that hold its value, the width of mirl's row of its layout |
| [`VecShape`](#vecshape) | `rec` | one vector the type table seeds |
| [`IntRange`](#intrange) | `rec` | the magnitudes an integer kind admits: min_mag is the largest negated value (the signed minimum), max_mag the largest positive one, both exact to 128 bits |
| [`VecForm`](#vecform) | `rec` | what a vector spelling such as `u32x4` says |

| On `PrimClass` | |
| :--- | :--- |
| [`PRIM_CLASS_NONE`](#primclass), [`PRIM_CLASS_INT`](#primclass), [`PRIM_CLASS_FLOAT`](#primclass), [`PRIM_CLASS_PTR`](#primclass) | |

| On `PrimDesc` | |
| :--- | :--- |
| [`prim_desc`](#prim_desc) | the class, signedness and widths of a kind |

| On `VecShape` | |
| :--- | :--- |
| [`vec_shape`](#vec_shape) | the seeded vector at one position |

| On `IntRange` | |
| :--- | :--- |
| [`int_range`](#int_range) | the values an integer kind admits a kind wider than 128 bits answers what an i128 holds when signed, and 0 to the i128 maximum when unsigned |
| [`int_fits`](#int_fits) | whether a signed magnitude lies in a range |

| On `VecForm` | |
| :--- | :--- |
| [`vector_form`](#vector_form) | read a vector spelling such as `u32x4` |

| Functions | |
| :--- | :--- |
| [`is_prim`](#is_prim) | the primitive predicate: a kind is primitive exactly when the catalog names it, never because of where it sits in the TypeKind numbering |
| [`prim_index`](#prim_index) | the catalog position of a primitive kind, the index of every table sized by PRIM_COUNT |
| [`prim_kind_at`](#prim_kind_at) | the kind at a catalog position, for walkers over 0..PRIM_COUNT |
| [`prim_name`](#prim_name) | the spelling of a primitive kind |
| [`prim_spelling_len`](#prim_spelling_len) | the length of a primitive kind's spelling |
| [`int_kind_for`](#int_kind_for) | the integer primitive of a width and signedness, absent when the catalog has no such row; the one lookup every width-driven typing goes through |
| [`prim_from_view`](#prim_from_view) | the primitive a name spells exactly |
| [`int_fits_either_sign`](#int_fits_either_sign) | whether a magnitude fits a kind as a positive or as a negative value |
| [`float_width_of`](#float_width_of) | the width of a float kind; FLOAT_W_NONE for every other kind |
| [`is_vector_spelling`](#is_vector_spelling) | whether a name has the form of a vector spelling, well formed or not |

| Constants | |
| :--- | :--- |
| [`PRIM_COUNT`](#constants) | |
| [`VEC_COUNT`](#constants) | the vectors the type table seeds, by position below VEC_COUNT |
| [`MAX_VEC_LANES`](#constants), [`VEC_FORM_NONE`](#constants), [`VEC_FORM_OK`](#constants), [`VEC_FORM_TOO_WIDE`](#constants) | |
| [`VEC_FORM_BAD_ELEMENT`](#constants) | the element is a primitive no vector packs: a lane is 8 to 64 bits wide |
| [`VEC_LANE_MIN_BITS`](#constants), [`VEC_LANE_MAX_BITS`](#constants) | |

---

## PrimClass

```mach
pub def PrimClass: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `PRIM_CLASS_NONE` | `0` |  |
| `PRIM_CLASS_INT` | `1` |  |
| `PRIM_CLASS_FLOAT` | `2` |  |
| `PRIM_CLASS_PTR` | `3` |  |

## PrimDesc

```mach
pub rec PrimDesc {
    class:  PrimClass;
    signed: bool;
    bits:   u32;
    width:  u32;
}
```

bits:  what a value occupies, in memory and as an operand
width: the bits that hold its value, the width of mirl's row of its layout. only `u1`
       differs: a byte holding 0 or 1

### prim_desc

```mach
pub fun prim_desc(kind: TypeKind) PrimDesc
```

the class, signedness and widths of a kind

| Parameter | |
| :--- | :--- |
| `kind` | the kind |
| **returns** | its description, PRIM_CLASS_NONE with zero widths for a kind the catalog does not name |

## VecShape

```mach
pub rec VecShape {
    name:    str;
    element: TypeKind;
    lanes:   u32;
}
```

one vector the type table seeds

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | its spelling |
| `element` | `TypeKind` | its lane kind |
| `lanes` | `u32` | its lane count |

### vec_shape

```mach
pub fun vec_shape(index: u32) VecShape
```

the seeded vector at one position

| Parameter | |
| :--- | :--- |
| `index` | the position below VEC_COUNT |
| **returns** | its shape, a nil name and zero lanes at or past VEC_COUNT |

## IntRange

```mach
pub rec IntRange {
    min_negated: bool;
    min_mag:     mach.base.wide.Wide;
    max_mag:     mach.base.wide.Wide;
}
```

the magnitudes an integer kind admits: min_mag is the largest negated value
(the signed minimum), max_mag the largest positive one, both exact to 128 bits

### int_range

```mach
pub fun int_range(kind: TypeKind) opt[IntRange]
```

the values an integer kind admits
a kind wider than 128 bits answers what an i128 holds when signed, and 0 to the i128 maximum when unsigned

| Parameter | |
| :--- | :--- |
| `kind` | the kind |
| **returns** | its range, none for a kind that is not an integer |

### int_fits

```mach
pub fun int_fits(value: mach.base.wide.Wide, negated: bool, r: IntRange) bool
```

whether a signed magnitude lies in a range

| Parameter | |
| :--- | :--- |
| `value` | the magnitude |
| `negated` | whether the value is negative |
| `r` | the range |
| **returns** | true when it fits |

## VecForm

```mach
pub rec VecForm {
    status:  VecFormStatus;
    element: TypeKind;
    lanes:   u32;
    bits:    u32;
}
```

what a vector spelling such as `u32x4` says

| Field | Type | |
| :--- | :--- | :--- |
| `status` | `VecFormStatus` | whether it is a vector spelling and how it is malformed (`VEC_FORM_*`) |
| `element` | `TypeKind` | the lane kind |
| `lanes` | `u32` | the lane count, MAX_VEC_LANES + 1 for any count past it |
| `bits` | `u32` | the total width in bits, set only when well formed |

### vector_form

```mach
pub fun vector_form(name: str) VecForm
```

read a vector spelling such as `u32x4`

| Parameter | |
| :--- | :--- |
| `name` | the spelling, which may be nil |
| **returns** | its form, VEC_FORM_NONE when `name` is no vector spelling |

## Functions

### is_prim

```mach
pub fun is_prim(kind: TypeKind) bool
```

the primitive predicate: a kind is primitive exactly when the catalog names
it, never because of where it sits in the TypeKind numbering

### prim_index

```mach
pub fun prim_index(kind: TypeKind) opt[u32]
```

the catalog position of a primitive kind, the index of every table sized by
PRIM_COUNT

### prim_kind_at

```mach
pub fun prim_kind_at(index: u32) TypeKind
```

the kind at a catalog position, for walkers over 0..PRIM_COUNT

### prim_name

```mach
pub fun prim_name(kind: TypeKind) str
```

the spelling of a primitive kind

| Parameter | |
| :--- | :--- |
| `kind` | the kind |
| **returns** | its spelling, nil for a kind the catalog does not name |

### prim_spelling_len

```mach
pub fun prim_spelling_len(kind: TypeKind) usize
```

the length of a primitive kind's spelling

| Parameter | |
| :--- | :--- |
| `kind` | the kind |
| **returns** | the length in bytes, 0 for a kind the catalog does not name |

### int_kind_for

```mach
pub fun int_kind_for(width: u32, signed: bool) opt[TypeKind]
```

the integer primitive of a width and signedness, absent when the catalog has
no such row; the one lookup every width-driven typing goes through

### prim_from_view

```mach
pub fun prim_from_view(name: View) opt[TypeKind]
```

the primitive a name spells exactly

| Parameter | |
| :--- | :--- |
| `name` | the name |
| **returns** | the kind, none when it spells no primitive |

### int_fits_either_sign

```mach
pub fun int_fits_either_sign(value: mach.base.wide.Wide, kind: TypeKind) bool
```

whether a magnitude fits a kind as a positive or as a negative value

| Parameter | |
| :--- | :--- |
| `value` | the magnitude |
| `kind` | the kind |
| **returns** | true when either fits, and for a kind that is not an integer |

### float_width_of

```mach
pub fun float_width_of(kind: TypeKind) mach.base.float.FloatWidth
```

the width of a float kind; FLOAT_W_NONE for every other kind

### is_vector_spelling

```mach
pub fun is_vector_spelling(name: str) bool
```

whether a name has the form of a vector spelling, well formed or not

| Parameter | |
| :--- | :--- |
| `name` | the name |
| **returns** | true when it has |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `PRIM_COUNT` | `u32` | `19` |  |
| `VEC_COUNT` | `u32` | `10` | the vectors the type table seeds, by position below VEC_COUNT |
| `MAX_VEC_LANES` | `u32` | `65535` |  |
| `VEC_FORM_NONE` | `VecFormStatus` | `0` |  |
| `VEC_FORM_OK` | `VecFormStatus` | `1` |  |
| `VEC_FORM_TOO_WIDE` | `VecFormStatus` | `2` |  |
| `VEC_FORM_BAD_ELEMENT` | `VecFormStatus` | `4` | the element is a primitive no vector packs: a lane is 8 to 64 bits wide |
| `VEC_LANE_MIN_BITS` | `u32` | `8` |  |
| `VEC_LANE_MAX_BITS` | `u32` | `64` |  |

