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

## rec FieldRef

```mach
pub rec FieldRef;
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

## fun cast_scalar

```mach
pub fun cast_scalar(types: *type.TypeInterner, pointer_width: u32, nan_rule: float.NanRule, value: CTValue,
from_ty: type.TypeId, to_ty: type.TypeId, reinterpret: bool) res[CTValue, comptime_failure.EvalFail];
```

a float converted to another float width folds by `nan_rule`, the target's, so the
folded conversion is the one the target makes at run time

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
pub fun apply_arith(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, comptime_failure.EvalFail];
```

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

## fun float_value

```mach
pub fun float_value(f: f64, w: float.FloatWidth) res[CTValue, comptime_failure.EvalFail];
```

## fun ct_float

```mach
pub fun ct_float(f: f64, w: float.FloatWidth) CTValue;
```

## fun u8_value

```mach
pub fun u8_value(n: u8) res[CTValue, comptime_failure.EvalFail];
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

## fun ct_str

```mach
pub fun ct_str(s: intern.StrId) CTValue;
```

## fun ct_u8

```mach
pub fun ct_u8(n: u8) CTValue;
```

## fun is_u8

```mach
pub fun is_u8(v: CTValue) bool;
```

## fun truth

```mach
pub fun truth(v: CTValue) bool;
```

## fun if_u8

```mach
pub fun if_u8(v: bool) u8;
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

