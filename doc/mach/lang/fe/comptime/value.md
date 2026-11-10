# mach.lang.fe.comptime.value

the comptime value model: its kinds, constructors, integer and float arithmetic and
casts between scalars

## def CTKind

```mach
pub def CTKind: u8
```

## val CT_KIND_INT

```mach
pub val CT_KIND_INT:   CTKind = 0
```

## val CT_KIND_FLOAT

```mach
pub val CT_KIND_FLOAT: CTKind = 1
```

## val CT_KIND_STR

```mach
pub val CT_KIND_STR:   CTKind = 2
```

## val CT_KIND_TYPE

```mach
pub val CT_KIND_TYPE: CTKind = 3
```

## val CT_KIND_FIELD

```mach
pub val CT_KIND_FIELD: CTKind = 4
```

## val CT_KIND_PACK_ELEM

```mach
pub val CT_KIND_PACK_ELEM: CTKind = 5
```

## val CT_KIND_CONST_ELEM

```mach
pub val CT_KIND_CONST_ELEM: CTKind = 6
```

## val CT_KIND_CASE

```mach
pub val CT_KIND_CASE: CTKind = 7
```

a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor

## val CT_KIND_NON_INTEGER

```mach
pub val CT_KIND_NON_INTEGER: CTKind = 8
```

an integer constant whose declared type is a def chain ending outside the integers;
data.s is the interned refusal naming the chain, and reading it fails with that refusal

## val CT_KIND_NODE

```mach
pub val CT_KIND_NODE: CTKind = 9
```

a node of a module's published constant values (see comptime.deep), read at type `ty`;
`module` is the stable id of the module whose store holds it

## rec CTValue

```mach
pub rec CTValue;
```

an integer value is 128 bits in data.w; data.i is its low limb, and a signed
value sign-fills the high limb so every reader of a value that fits 64 bits
sees the same i64 it always did

## fun internal_failure

```mach
pub fun internal_failure(message: str) res[CTValue, comptime_failure.EvalFail];
```

## fun reject

```mach
pub fun reject(diag: diagnostic_kind.Kind, message: str) res[CTValue, comptime_failure.EvalFail];
```

## fun awaiting

```mach
pub fun awaiting(kind: comptime_failure.EvalFailKind, diag: diagnostic_kind.Kind, message: str) res[CTValue, comptime_failure.EvalFail];
```

## fun cast_int

```mach
pub fun cast_int(bits: wide.Wide, width: u32, signed: bool) res[CTValue, comptime_failure.EvalFail];
```

## fun nan_unspecified

```mach
pub fun nan_unspecified(itn: *intern.Interner, arch_id: u32, nans: target_facts.Nans, w: float.FloatWidth) res[CTValue, comptime_failure.EvalFail];
```

the refusal of a float operation whose result is a NaN at width `w` on the instruction
set `arch_id`, whose rule `nans` gives no defined NaN for it: no constant has that value.
a rule stated unspecified is the target model's, and one not stated is a gap of mirl's

## fun cast_scalar

```mach
pub fun cast_scalar(types: *type.TypeInterner, itn: *intern.Interner, pointer_width: u32, nans: target_facts.Nans, arch_id: u32,
value: CTValue, from_ty: type_kind.TypeId, to_ty: type_kind.TypeId, reinterpret: bool) res[CTValue, comptime_failure.EvalFail];
```

a numeric conversion folds through mirl's conversion rows. a float to int cast
whose value is NaN or truncates outside the integer's range is refused, so the
saturation mirl's row gives such a value is never folded. a NaN converted to
another float width is made by the target's rule `nans` for the format it is
converted to, and where the rule leaves it unknown it is refused, naming the
target's instruction set `arch_id`

## fun is_descriptor

```mach
pub fun is_descriptor(v: CTValue) bool;
```

## fun apply_declared_int_type

```mach
pub fun apply_declared_int_type(a: *ast.Ast, source: str, t: ast_id.TypeId, value: CTValue) CTValue;
```

## fun cast_to_int

```mach
pub fun cast_to_int(value: CTValue, width: u32, signed: bool) res[CTValue, comptime_failure.EvalFail];
```

an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign,
an untyped one as it stands, then the result is cut or extended to the destination

## fun width_held

```mach
pub fun width_held(bits: u32) u8;
```

the width a comptime integer of a `bits`-bit type is held at: its own up to the 128-bit
carrier, and untyped past it, exact to 128 bits with every overflow refused (mach#4803
widens the carrier)

## fun typed_int

```mach
pub fun typed_int(value: CTValue, unsigned: bool, width: u8) CTValue;
```

## fun float_width_unread

```mach
pub fun float_width_unread(v: CTValue) bool;
```

## fun ct_is_negative

```mach
pub fun ct_is_negative(v: CTValue) bool;
```

## fun ct_int_u64

```mach
pub fun ct_int_u64(v: CTValue) opt[u64];
```

an integer value that is neither negative nor above u64, read as a u64

## fun ct_int_text

```mach
pub fun ct_int_text(v: CTValue, buf: *u8) str;
```

the decimal text of an integer value in a wide.FORMAT_CAP buffer

## fun apply_equality

```mach
pub fun apply_equality(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, comptime_failure.EvalFail];
```

## fun apply_compare

```mach
pub fun apply_compare(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, comptime_failure.EvalFail];
```

## fun apply_arith

```mach
pub fun apply_arith(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue, nans: target_facts.Nans) res[CTValue, comptime_failure.EvalFail];
```

`nans` is the target's rule for the nan a float operation returns, and a nan result it
leaves unknown is refused as comptime.nan_unspecified

## fun non_integer

```mach
pub fun non_integer(message: intern.StrId) CTValue;
```

## fun readable

```mach
pub fun readable(v: CTValue, interner: *intern.Interner) res[CTValue, comptime_failure.EvalFail];
```

a named constant's value as a reader sees it: a non-integer constant fails with its refusal

## fun make_int_wide

```mach
pub fun make_int_wide(bits: wide.Wide, unsigned: bool) res[CTValue, comptime_failure.EvalFail];
```

## fun int_value

```mach
pub fun int_value(i: i64) res[CTValue, comptime_failure.EvalFail];
```

## fun ct_negate

```mach
pub fun ct_negate(v: CTValue) res[CTValue, comptime_failure.EvalFail];
```

-x over the untyped 128-bit range: the signed minimum negates to its own
magnitude, which only reads as unsigned; an unsigned magnitude past it has no
negation

## fun float_negated

```mach
pub fun float_negated(x: f64, w: float.FloatWidth) f64;
```

the negation of a float at width `w`, folded

## fun complement

```mach
pub fun complement(x: wide.Wide) wide.Wide;
```

the bitwise complement of an integer, folded at comptime's width

## fun float_value

```mach
pub fun float_value(f: f64, w: float.FloatWidth) res[CTValue, comptime_failure.EvalFail];
```

## fun ct_float

```mach
pub fun ct_float(f: f64, w: float.FloatWidth) CTValue;
```

## fun u1_value

```mach
pub fun u1_value(held: bool) res[CTValue, comptime_failure.EvalFail];
```

## fun str_value

```mach
pub fun str_value(s: intern.StrId) res[CTValue, comptime_failure.EvalFail];
```

## fun type_value

```mach
pub fun type_value(t: u32) res[CTValue, comptime_failure.EvalFail];
```

## fun field_value

```mach
pub fun field_value(owner: u32, index: u32) CTValue;
```

## fun case_value

```mach
pub fun case_value(owner: u32, index: u32) CTValue;
```

## fun pack_elem_value

```mach
pub fun pack_elem_value(index: u32, ty: u32) CTValue;
```

## fun const_elem_untyped

```mach
pub fun const_elem_untyped(elem: u32) CTValue;
```

a constant element whose checked type is not yet known: name resolution and loading bind case
literals this way, and type checking rebinds them with the declared type

## fun const_elem_value

```mach
pub fun const_elem_value(elem: u32, ty: u32) CTValue;
```

## fun node_ref

```mach
pub fun node_ref(module: u32, node: u32, ty: u32) CTValue;
```

## fun ct_str

```mach
pub fun ct_str(s: intern.StrId) CTValue;
```

## fun ct_u1

```mach
pub fun ct_u1(held: bool) CTValue;
```

the truth value `held`, a u1: a comparison's answer, and the type a condition requires

## fun is_u1

```mach
pub fun is_u1(v: CTValue) bool;
```

## fun truth

```mach
pub fun truth(v: CTValue) bool;
```

## fun ct_type

```mach
pub fun ct_type(t: u32) CTValue;
```

## fun ct_uint

```mach
pub fun ct_uint(n: u64) CTValue;
```

## fun ct_zero_int

```mach
pub fun ct_zero_int(unsigned: bool, width: u8) CTValue;
```

