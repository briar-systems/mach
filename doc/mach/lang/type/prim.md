# mach.lang.type.prim

## val PRIM_COUNT

```mach
pub val PRIM_COUNT: u32 = 14
```

## def PrimClass

```mach
pub def PrimClass: u8
```

## val PRIM_CLASS_NONE

```mach
pub val PRIM_CLASS_NONE:  PrimClass = 0
```

## val PRIM_CLASS_INT

```mach
pub val PRIM_CLASS_INT:   PrimClass = 1
```

## val PRIM_CLASS_FLOAT

```mach
pub val PRIM_CLASS_FLOAT: PrimClass = 2
```

## val PRIM_CLASS_PTR

```mach
pub val PRIM_CLASS_PTR:   PrimClass = 3
```

## rec PrimDesc

```mach
pub rec PrimDesc;
```

## fun is_prim

```mach
pub fun is_prim(kind: TypeKind) bool;
```

the primitive predicate: a kind is primitive exactly when the catalog names
it, never because of where it sits in the TypeKind numbering

## fun prim_index

```mach
pub fun prim_index(kind: TypeKind) opt[u32];
```

the catalog position of a primitive kind, the index of every table sized by
PRIM_COUNT

## fun prim_kind_at

```mach
pub fun prim_kind_at(index: u32) TypeKind;
```

the kind at a catalog position, for walkers over 0..PRIM_COUNT

## fun prim_desc

```mach
pub fun prim_desc(kind: TypeKind) PrimDesc;
```

## val VEC_COUNT

```mach
pub val VEC_COUNT: u32 = 10
```

the vectors the type table seeds, by position below VEC_COUNT

## rec VecShape

```mach
pub rec VecShape;
```

## fun vec_shape

```mach
pub fun vec_shape(index: u32) VecShape;
```

## fun prim_name

```mach
pub fun prim_name(kind: TypeKind) str;
```

## fun prim_spelling_len

```mach
pub fun prim_spelling_len(kind: TypeKind) usize;
```

## fun int_kind_for

```mach
pub fun int_kind_for(bits: u32, signed: bool) opt[TypeKind];
```

the integer primitive of a width and signedness, absent when the catalog has
no such row; the one lookup every width-driven typing goes through

## fun prim_from_view

```mach
pub fun prim_from_view(name: View) opt[TypeKind];
```

## rec IntRange

```mach
pub rec IntRange;
```

the magnitudes an integer kind admits: min_mag is the largest negated value
(the signed minimum), max_mag the largest positive one, both exact to 128 bits

## fun int_range

```mach
pub fun int_range(kind: TypeKind) opt[IntRange];
```

## fun int_fits

```mach
pub fun int_fits(value: wide.Wide, negated: bool, r: IntRange) bool;
```

## fun int_fits_either_sign

```mach
pub fun int_fits_either_sign(value: wide.Wide, kind: TypeKind) bool;
```

## fun float_width_of

```mach
pub fun float_width_of(kind: TypeKind) float.FloatWidth;
```

the width of a float kind; FLOAT_W_NONE for every other kind

## fun float_kind_of

```mach
pub fun float_kind_of(w: float.FloatWidth) TypeKind;
```

the float kind of a width, f64 for FLOAT_W_NONE, the unsuffixed default

## val MAX_VEC_LANES

```mach
pub val MAX_VEC_LANES: u32 = 65535
```

## val VEC_FORM_NONE

```mach
pub val VEC_FORM_NONE:      VecFormStatus = 0
```

## val VEC_FORM_OK

```mach
pub val VEC_FORM_OK:        VecFormStatus = 1
```

## val VEC_FORM_TOO_WIDE

```mach
pub val VEC_FORM_TOO_WIDE:  VecFormStatus = 2
```

## val VEC_FORM_BAD_ELEMENT

```mach
pub val VEC_FORM_BAD_ELEMENT: VecFormStatus = 4
```

the element is a primitive no vector packs: a lane is 8 to 64 bits wide

## val VEC_LANE_MIN_BITS

```mach
pub val VEC_LANE_MIN_BITS: u32 = 8
```

## val VEC_LANE_MAX_BITS

```mach
pub val VEC_LANE_MAX_BITS: u32 = 64
```

## rec VecForm

```mach
pub rec VecForm;
```

## fun is_vector_spelling

```mach
pub fun is_vector_spelling(name: str) bool;
```

## fun vector_form

```mach
pub fun vector_form(name: str) VecForm;
```

