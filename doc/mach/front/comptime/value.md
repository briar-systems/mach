[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **value**

# mach.front.comptime.value

the comptime value model: its kinds, constructors, integer and float arithmetic and
casts between scalars

<sub>source: [`src/front/comptime/value.mach`](../../../../src/front/comptime/value.mach) · imported by 32 modules</sub>

<sub>`float` is [`mach.base.float`](../../base/float.md) · `wide` is [`mach.base.wide`](../../base/wide.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `FieldRef` is private · `PackElemRef` is private · `ConstElemRef` is private · `NodeRef` is private · `comptime_failure` is [`mach.front.comptime.failure`](failure.md) · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md) · `target_facts` is [`mach.target.facts`](../../target/facts.md) · `type` is [`mach.type`](../../type.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `ast` is [`mach.front.ast`](../ast.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `ast_expr` is [`mach.front.ast.expr`](../ast/expr.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`CTKind`](#ctkind) | `def` |  |
| [`CTValue`](#ctvalue) | `rec` | an integer value is 128 bits in data.w; data.i is its low limb, and a signed value sign-fills the high limb so every reader of a value that fits 64 bits sees the same i64 it always did |

| On `CTKind` | |
| :--- | :--- |
| [`CT_KIND_INT`](#ctkind) · [`CT_KIND_FLOAT`](#ctkind) · [`CT_KIND_STR`](#ctkind) · [`CT_KIND_TYPE`](#ctkind) · [`CT_KIND_FIELD`](#ctkind) · [`CT_KIND_PACK_ELEM`](#ctkind) · [`CT_KIND_CONST_ELEM`](#ctkind) | |
| [`CT_KIND_CASE`](#ctkind) | a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor |
| [`CT_KIND_NON_INTEGER`](#ctkind) | an integer constant whose declared type is a def chain ending outside the integers; data.s is the interned refusal naming the chain, and reading it fails with that refusal |
| [`CT_KIND_NODE`](#ctkind) | a node of a module's published constant values (see comptime.deep), read at type `ty`; `module` is the stable id of the module whose store holds it |

| On `CTValue` | |
| :--- | :--- |
| [`internal_failure`](#internal_failure) · [`reject`](#reject) · [`awaiting`](#awaiting) · [`cast_int`](#cast_int) | |
| [`nan_unspecified`](#nan_unspecified) | the refusal of a float operation whose result is a NaN at width `w` on the instruction set `arch_id`, whose rule `nans` gives no defined NaN for it: no constant has that value |
| [`cast_scalar`](#cast_scalar) | a numeric conversion folds through mirl's conversion rows |
| [`is_descriptor`](#is_descriptor) · [`apply_declared_int_type`](#apply_declared_int_type) | |
| [`cast_to_int`](#cast_to_int) | an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign, an untyped one as it stands, then the result is cut or extended to the destination |
| [`typed_int`](#typed_int) · [`float_width_unread`](#float_width_unread) · [`ct_is_negative`](#ct_is_negative) | |
| [`ct_int_u64`](#ct_int_u64) | an integer value that is neither negative nor above u64, read as a u64 |
| [`ct_int_text`](#ct_int_text) | the decimal text of an integer value in a wide.FORMAT_CAP buffer |
| [`apply_equality`](#apply_equality) · [`apply_compare`](#apply_compare) | |
| [`apply_arith`](#apply_arith) | `nans` is the target's rule for the nan a float operation returns, and a nan result it leaves unknown is refused as comptime.nan_unspecified |
| [`non_integer`](#non_integer) | |
| [`readable`](#readable) | a named constant's value as a reader sees it: a non-integer constant fails with its refusal |
| [`int_wide_make`](#int_wide_make) · [`int_value`](#int_value) | |
| [`ct_negate`](#ct_negate) | -x over the untyped 128-bit range: the signed minimum negates to its own magnitude, which only reads as unsigned; an unsigned magnitude past it has no negation |
| [`float_value`](#float_value) · [`ct_float`](#ct_float) · [`u1_value`](#u1_value) · [`str_value`](#str_value) · [`type_value`](#type_value) · [`field_value`](#field_value) · [`case_value`](#case_value) · [`pack_elem_value`](#pack_elem_value) | |
| [`const_elem_untyped`](#const_elem_untyped) | a constant element whose checked type is not yet known: name resolution and loading bind case literals this way, and type checking rebinds them with the declared type |
| [`const_elem_value`](#const_elem_value) · [`node_ref`](#node_ref) · [`ct_str`](#ct_str) | |
| [`ct_u1`](#ct_u1) | the truth value `held`, a u1: a comparison's answer, and the type a condition requires |
| [`is_u1`](#is_u1) · [`truth`](#truth) · [`ct_type`](#ct_type) · [`ct_uint`](#ct_uint) · [`ct_zero_int`](#ct_zero_int) | |

| Functions | |
| :--- | :--- |
| [`width_held`](#width_held) | the width a comptime integer of a `bits`-bit type is held at: its own up to the 128-bit carrier, and untyped past it, exact to 128 bits with every overflow refused (mach#4803 widens the carrier) |
| [`float_negated`](#float_negated) | the negation of a float at width `w`, folded |
| [`complement`](#complement) | the bitwise complement of an integer, folded at comptime's width |

---

## CTKind

```mach
pub def CTKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CT_KIND_INT` | `0` |  |
| `CT_KIND_FLOAT` | `1` |  |
| `CT_KIND_STR` | `2` |  |
| `CT_KIND_TYPE` | `3` |  |
| `CT_KIND_FIELD` | `4` |  |
| `CT_KIND_PACK_ELEM` | `5` |  |
| `CT_KIND_CONST_ELEM` | `6` |  |
| `CT_KIND_CASE` | `7` | a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor |
| `CT_KIND_NON_INTEGER` | `8` | an integer constant whose declared type is a def chain ending outside the integers; data.s is the interned refusal naming the chain, and reading it fails with that refusal |
| `CT_KIND_NODE` | `9` | a node of a module's published constant values (see comptime.deep), read at type `ty`; `module` is the stable id of the module whose store holds it |

## CTValue

```mach
pub rec CTValue {
    kind:         CTKind;
    int_unsigned: bool;
    int_width:    u8;
    float_width:  float.FloatWidth;
    # the constant's declared type holds a secret: it has no compile-time value but under `:>`
    secret: bool;
    data:   uni {
        i: i64;
        w: wide.Wide;
        f: f64;
        s: intern.StrId;
        t: u32;
        fld: FieldRef;
        pelem: PackElemRef;
        celem: ConstElemRef;
        node: NodeRef;
    };
}
```

an integer value is 128 bits in data.w; data.i is its low limb, and a signed
value sign-fills the high limb so every reader of a value that fits 64 bits
sees the same i64 it always did

### internal_failure

```mach
pub fun internal_failure(message: str) res[CTValue, comptime_failure.EvalFail]
```

### reject

```mach
pub fun reject(diag: diagnostic_kind.Kind, message: str) res[CTValue, comptime_failure.EvalFail]
```

### awaiting

```mach
pub fun awaiting(kind: comptime_failure.EvalFailKind, diag: diagnostic_kind.Kind, message: str) res[CTValue, comptime_failure.EvalFail]
```

### cast_int

```mach
pub fun cast_int(bits: wide.Wide, width: u32, signed: bool) res[CTValue, comptime_failure.EvalFail]
```

### nan_unspecified

```mach
pub fun nan_unspecified(itn: *intern.Interner, arch_id: u32, nans: target_facts.Nans, w: float.FloatWidth) res[CTValue, comptime_failure.EvalFail]
```

the refusal of a float operation whose result is a NaN at width `w` on the instruction
set `arch_id`, whose rule `nans` gives no defined NaN for it: no constant has that value.
a rule stated unspecified is the target model's, and one not stated is a gap of mirl's

### cast_scalar

```mach
pub fun cast_scalar(types: *type.TypeInterner, itn: *intern.Interner, pointer_width: u32, nans: target_facts.Nans, arch_id: u32,
    value: CTValue, from_ty: type_kind.TypeId, to_ty: type_kind.TypeId, reinterpret: bool) res[CTValue, comptime_failure.EvalFail]
```

a numeric conversion folds through mirl's conversion rows. a float to int cast
whose value is NaN or truncates outside the integer's range is refused, so the
saturation mirl's row gives such a value is never folded. a NaN converted to
another float width is made by the target's rule `nans` for the format it is
converted to, and where the rule leaves it unknown it is refused, naming the
target's instruction set `arch_id`

### is_descriptor

```mach
pub fun is_descriptor(v: CTValue) bool
```

### apply_declared_int_type

```mach
pub fun apply_declared_int_type(a: *ast.Ast, source: str, t: ast_id.TypeId, value: CTValue) CTValue
```

### cast_to_int

```mach
pub fun cast_to_int(value: CTValue, width: u32, signed: bool) res[CTValue, comptime_failure.EvalFail]
```

an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign,
an untyped one as it stands, then the result is cut or extended to the destination

### typed_int

```mach
pub fun typed_int(value: CTValue, unsigned: bool, width: u8) CTValue
```

### float_width_unread

```mach
pub fun float_width_unread(v: CTValue) bool
```

### ct_is_negative

```mach
pub fun ct_is_negative(v: CTValue) bool
```

### ct_int_u64

```mach
pub fun ct_int_u64(v: CTValue) opt[u64]
```

an integer value that is neither negative nor above u64, read as a u64

### ct_int_text

```mach
pub fun ct_int_text(v: CTValue, buf: *u8) str
```

the decimal text of an integer value in a wide.FORMAT_CAP buffer

### apply_equality

```mach
pub fun apply_equality(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, comptime_failure.EvalFail]
```

### apply_compare

```mach
pub fun apply_compare(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, comptime_failure.EvalFail]
```

### apply_arith

```mach
pub fun apply_arith(op: ast_expr.BinOp, lhs: CTValue, rhs: CTValue, nans: target_facts.Nans) res[CTValue, comptime_failure.EvalFail]
```

`nans` is the target's rule for the nan a float operation returns, and a nan result it
leaves unknown is refused as comptime.nan_unspecified

### non_integer

```mach
pub fun non_integer(message: intern.StrId) CTValue
```

### readable

```mach
pub fun readable(v: CTValue, interner: *intern.Interner) res[CTValue, comptime_failure.EvalFail]
```

a named constant's value as a reader sees it: a non-integer constant fails with its refusal

### int_wide_make

```mach
pub fun int_wide_make(bits: wide.Wide, unsigned: bool) res[CTValue, comptime_failure.EvalFail]
```

### int_value

```mach
pub fun int_value(i: i64) res[CTValue, comptime_failure.EvalFail]
```

### ct_negate

```mach
pub fun ct_negate(v: CTValue) res[CTValue, comptime_failure.EvalFail]
```

-x over the untyped 128-bit range: the signed minimum negates to its own
magnitude, which only reads as unsigned; an unsigned magnitude past it has no
negation

### float_value

```mach
pub fun float_value(f: f64, w: float.FloatWidth) res[CTValue, comptime_failure.EvalFail]
```

### ct_float

```mach
pub fun ct_float(f: f64, w: float.FloatWidth) CTValue
```

### u1_value

```mach
pub fun u1_value(held: bool) res[CTValue, comptime_failure.EvalFail]
```

### str_value

```mach
pub fun str_value(s: intern.StrId) res[CTValue, comptime_failure.EvalFail]
```

### type_value

```mach
pub fun type_value(t: u32) res[CTValue, comptime_failure.EvalFail]
```

### field_value

```mach
pub fun field_value(owner: u32, index: u32) CTValue
```

### case_value

```mach
pub fun case_value(owner: u32, index: u32) CTValue
```

### pack_elem_value

```mach
pub fun pack_elem_value(index: u32, ty: u32) CTValue
```

### const_elem_untyped

```mach
pub fun const_elem_untyped(elem: u32) CTValue
```

a constant element whose checked type is not yet known: name resolution and loading bind case
literals this way, and type checking rebinds them with the declared type

### const_elem_value

```mach
pub fun const_elem_value(elem: u32, ty: u32) CTValue
```

### node_ref

```mach
pub fun node_ref(module: u32, node: u32, ty: u32) CTValue
```

### ct_str

```mach
pub fun ct_str(s: intern.StrId) CTValue
```

### ct_u1

```mach
pub fun ct_u1(held: bool) CTValue
```

the truth value `held`, a u1: a comparison's answer, and the type a condition requires

### is_u1

```mach
pub fun is_u1(v: CTValue) bool
```

### truth

```mach
pub fun truth(v: CTValue) bool
```

### ct_type

```mach
pub fun ct_type(t: u32) CTValue
```

### ct_uint

```mach
pub fun ct_uint(n: u64) CTValue
```

### ct_zero_int

```mach
pub fun ct_zero_int(unsigned: bool, width: u8) CTValue
```

## Functions

### width_held

```mach
pub fun width_held(bits: u32) u8
```

the width a comptime integer of a `bits`-bit type is held at: its own up to the 128-bit
carrier, and untyped past it, exact to 128 bits with every overflow refused (mach#4803
widens the carrier)

### float_negated

```mach
pub fun float_negated(x: f64, w: float.FloatWidth) f64
```

the negation of a float at width `w`, folded

### complement

```mach
pub fun complement(x: wide.Wide) wide.Wide
```

the bitwise complement of an integer, folded at comptime's width

