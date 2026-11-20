[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **value**

# mach.front.comptime.value

the comptime value model: its kinds, constructors, integer and float arithmetic and
casts between scalars

Source: [`src/front/comptime/value.mach`](../../../../src/front/comptime/value.mach)

Uses types from [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.float`](../../base/float.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.wide`](../../base/wide.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.expr`](../ast/expr.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.failure`](failure.md), [`mach.target.facts`](../../target/facts.md), [`mach.type`](../../type.md), [`mach.type.kind`](../../type/kind.md).

| Type | | |
| :--- | :--- | :--- |
| [`CTKind`](#ctkind) | `def` |  |
| [`CTValue`](#ctvalue) | `rec` | an integer value is 128 bits in data.w; data.i is its low limb, and a signed value sign-fills the high limb so every reader of a value that fits 64 bits sees the same i64 it always did |

| On `CTKind` | |
| :--- | :--- |
| [`CT_KIND_INT`](#ctkind), [`CT_KIND_FLOAT`](#ctkind), [`CT_KIND_STR`](#ctkind), [`CT_KIND_TYPE`](#ctkind), [`CT_KIND_FIELD`](#ctkind), [`CT_KIND_PACK_ELEM`](#ctkind), [`CT_KIND_CONST_ELEM`](#ctkind) | |
| [`CT_KIND_CASE`](#ctkind) | a tag case descriptor from `$cases(T)`: the same owner/index shape as a field descriptor |
| [`CT_KIND_NON_INTEGER`](#ctkind) | an integer constant whose declared type is a def chain ending outside the integers; data.s is the interned refusal naming the chain, and reading it fails with that refusal |
| [`CT_KIND_NODE`](#ctkind) | a node of a module's published constant values (see comptime.deep), read at type `ty`; `module` is the stable id of the module whose store holds it |

| On `CTValue` | |
| :--- | :--- |
| [`internal_failure`](#internal_failure) | an internal failure of the evaluator as an evaluation result |
| [`reject`](#reject) | a refusal of the input as an evaluation result |
| [`reject_formatted`](#reject_formatted) | the refusal `diag` with a message formatted on the allocator of `itn`, interned so it outlives the evaluation while the formatted copy is freed |
| [`awaiting`](#awaiting) | an evaluation failure of any kind as an evaluation result |
| [`cast_int`](#cast_int) | an integer value of a given type: the bits cut to `width` and extended back by its sign |
| [`nan_unspecified`](#nan_unspecified) | the refusal of a float operation whose result is a NaN at width `w` on the instruction set `arch_id`, whose rule `nans` gives no defined NaN for it: no constant has that value |
| [`cast_scalar`](#cast_scalar) | a numeric conversion folds through mirl's conversion rows |
| [`is_descriptor`](#is_descriptor) | whether a value is a field or case descriptor |
| [`apply_declared_int_type`](#apply_declared_int_type) | an integer value typed by the integer type its declaration spells a named primitive integer types it at that width and sign, and any other name leaves it untyped |
| [`cast_to_int`](#cast_to_int) | an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign, an untyped one as it stands, then the result is cut or extended to the destination |
| [`typed_int`](#typed_int) | a value with its integer sign and width replaced |
| [`float_width_unread`](#float_width_unread) | whether a float value's width is not read yet |
| [`ct_is_negative`](#ct_is_negative) | whether a value is a negative integer |
| [`ct_int_u64`](#ct_int_u64) | an integer value that is neither negative nor above u64, read as a u64 |
| [`ct_int_text`](#ct_int_text) | the decimal text of an integer value in a wide.FORMAT_CAP buffer |
| [`apply_equality`](#apply_equality) | the u1 answer of `==` or `!=` over two values of one kind |
| [`apply_compare`](#apply_compare) | the u1 answer of `<`, `<=`, `>` or `>=` over two numbers of one kind |
| [`apply_arith`](#apply_arith) | `nans` is the target's rule for the nan a float operation returns, and a nan result it leaves unknown is refused as comptime.nan_unspecified |
| [`non_integer`](#non_integer) | an integer constant whose declared type is no integer, which fails when read |
| [`readable`](#readable) | a named constant's value as a reader sees it: a non-integer constant fails with its refusal |
| [`int_wide_make`](#int_wide_make) | an untyped integer value of 128 bits |
| [`int_value`](#int_value) | an untyped signed integer value |
| [`ct_negate`](#ct_negate) | -x over the untyped 128-bit range: the signed minimum negates to its own magnitude, which only reads as unsigned; an unsigned magnitude past it has no negation |
| [`float_value`](#float_value) | a float value as an evaluation result, rounded to its width |
| [`ct_float`](#ct_float) | a float value rounded to its width |
| [`u1_value`](#u1_value) | a u1 truth value as an evaluation result |
| [`str_value`](#str_value) | a string value as an evaluation result |
| [`type_value`](#type_value) | a type value as an evaluation result |
| [`field_value`](#field_value) | a field descriptor of `$fields(T)` |
| [`case_value`](#case_value) | a case descriptor of `$cases(T)` |
| [`pack_elem_value`](#pack_elem_value) | an element of a variadic pack |
| [`const_elem_untyped`](#const_elem_untyped) | a constant element whose checked type is not yet known: name resolution and loading bind case literals this way, and type checking rebinds them with the declared type |
| [`const_elem_value`](#const_elem_value) | a constant element read at type `ty` |
| [`node_ref`](#node_ref) | a node of a module's published constant values |
| [`ct_str`](#ct_str) | a string value |
| [`ct_u1`](#ct_u1) | the truth value `held`, a u1: a comparison's answer, and the type a condition requires |
| [`is_u1`](#is_u1) | whether a value is a u1 |
| [`truth`](#truth) | whether a value holds the truth |
| [`ct_type`](#ct_type) | a type value |
| [`ct_uint`](#ct_uint) | a u64 value |
| [`ct_zero_int`](#ct_zero_int) | an integer zero of a given type |

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
    float_width:  mach.base.float.FloatWidth;
    # the constant's declared type holds a secret: it has no compile-time value but under `:>`
    secret: bool;
    data:   uni {
        i: i64;
        w: mach.base.wide.Wide;
        f: f64;
        s: mach.base.intern.StrId;
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
pub fun internal_failure(message: str) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an internal failure of the evaluator as an evaluation result

| Parameter | |
| :--- | :--- |
| `message` | the failure's text, borrowed |
| **returns** | the failure |

### reject

```mach
pub fun reject(diag: mach.base.diagnostic.kind.Kind, message: str) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a refusal of the input as an evaluation result

| Parameter | |
| :--- | :--- |
| `diag` | the diagnostic kind it is reported as |
| `message` | the refusal's text, borrowed |
| **returns** | the failure, of kind EVAL_FAIL_REJECTED |

### reject_formatted

```mach
pub fun reject_formatted(itn: *mach.base.intern.Interner, diag: mach.base.diagnostic.kind.Kind, res_text: res[str, mach.base.fail.Fail]) res[CTValue, mach.front.comptime.failure.EvalFail]
```

the refusal `diag` with a message formatted on the allocator of `itn`, interned so it
outlives the evaluation while the formatted copy is freed

### awaiting

```mach
pub fun awaiting(kind: mach.front.comptime.failure.EvalFailKind, diag: mach.base.diagnostic.kind.Kind, message: str) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an evaluation failure of any kind as an evaluation result

| Parameter | |
| :--- | :--- |
| `kind` | why there is no value, such as a phase it waits on |
| `diag` | the diagnostic kind it is reported as |
| `message` | the text, borrowed |
| **returns** | the failure |

### cast_int

```mach
pub fun cast_int(bits: mach.base.wide.Wide, width: u32, signed: bool) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an integer value of a given type: the bits cut to `width` and extended back by its sign

| Parameter | |
| :--- | :--- |
| `bits` | the value's bits |
| `width` | the type's width in bits, more than zero |
| `signed` | whether the type is signed |
| **returns** | the typed value, held untyped past 128 bits, or an internal failure for a zero width |

### nan_unspecified

```mach
pub fun nan_unspecified(itn: *mach.base.intern.Interner, arch_id: u32, nans: mach.target.facts.Nans, w: mach.base.float.FloatWidth) res[CTValue, mach.front.comptime.failure.EvalFail]
```

the refusal of a float operation whose result is a NaN at width `w` on the instruction
set `arch_id`, whose rule `nans` gives no defined NaN for it: no constant has that value.
a rule stated unspecified is the target model's, and one not stated is a gap of mirl's

### cast_scalar

```mach
pub fun cast_scalar(types: *mach.type.TypeInterner, itn: *mach.base.intern.Interner, pointer_width: u32, nans: mach.target.facts.Nans, arch_id: u32,
    value: CTValue, from_ty: mach.type.kind.TypeId, to_ty: mach.type.kind.TypeId, reinterpret: bool) res[CTValue, mach.front.comptime.failure.EvalFail]
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

whether a value is a field or case descriptor

| Parameter | |
| :--- | :--- |
| `v` | the value |
| **returns** | true for CT_KIND_FIELD and CT_KIND_CASE |

### apply_declared_int_type

```mach
pub fun apply_declared_int_type(a: *mach.front.ast.Ast, source: str, t: mach.front.ast.id.TypeId, value: CTValue) CTValue
```

an integer value typed by the integer type its declaration spells
a named primitive integer types it at that width and sign, and any other name leaves it untyped.
a secret type is read through, and a non-integer value, a nil type, a generic name or a type that
is no name keep the value as it is

| Parameter | |
| :--- | :--- |
| `a` | the tree |
| `source` | the tree's source text |
| `t` | the declared type syntax |
| `value` | the value |
| **returns** | the value with its integer type applied |

### cast_to_int

```mach
pub fun cast_to_int(value: CTValue, width: u32, signed: bool) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an integer cast as a scalar cast computes it: a typed operand is read at its own width and sign,
an untyped one as it stands, then the result is cut or extended to the destination

### typed_int

```mach
pub fun typed_int(value: CTValue, unsigned: bool, width: u8) CTValue
```

a value with its integer sign and width replaced

| Parameter | |
| :--- | :--- |
| `value` | the value |
| `unsigned` | whether it reads as unsigned |
| `width` | its width in bits, 0 for untyped |
| **returns** | the retyped value, its bits untouched |

### float_width_unread

```mach
pub fun float_width_unread(v: CTValue) bool
```

whether a float value's width is not read yet

| Parameter | |
| :--- | :--- |
| `v` | the value |
| **returns** | true for a float of FLOAT_W_NONE |

### ct_is_negative

```mach
pub fun ct_is_negative(v: CTValue) bool
```

whether a value is a negative integer

| Parameter | |
| :--- | :--- |
| `v` | the value |
| **returns** | true for a signed integer below zero, false for an unsigned one or any other kind |

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
pub fun apply_equality(op: mach.front.ast.expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, mach.front.comptime.failure.EvalFail]
```

the u1 answer of `==` or `!=` over two values of one kind

| Parameter | |
| :--- | :--- |
| `op` | BIN_EQ or BIN_NEQ, any other read as `!=` |
| `lhs` | the left operand |
| `rhs` | the right operand |
| **returns** | the answer, or a refusal for operands of different kinds, floats of different widths or a kind with no equality |

### apply_compare

```mach
pub fun apply_compare(op: mach.front.ast.expr.BinOp, lhs: CTValue, rhs: CTValue) res[CTValue, mach.front.comptime.failure.EvalFail]
```

the u1 answer of `<`, `<=`, `>` or `>=` over two numbers of one kind

| Parameter | |
| :--- | :--- |
| `op` | the comparison, any other read as `>=` |
| `lhs` | the left operand |
| `rhs` | the right operand |
| **returns** | the answer, or a refusal for operands of different kinds, floats of different widths or non-numeric operands |

### apply_arith

```mach
pub fun apply_arith(op: mach.front.ast.expr.BinOp, lhs: CTValue, rhs: CTValue, nans: mach.target.facts.Nans) res[CTValue, mach.front.comptime.failure.EvalFail]
```

`nans` is the target's rule for the nan a float operation returns, and a nan result it
leaves unknown is refused as comptime.nan_unspecified

### non_integer

```mach
pub fun non_integer(message: mach.base.intern.StrId) CTValue
```

an integer constant whose declared type is no integer, which fails when read

| Parameter | |
| :--- | :--- |
| `message` | the interned refusal naming the def chain |
| **returns** | the value, of kind CT_KIND_NON_INTEGER |

### readable

```mach
pub fun readable(v: CTValue, interner: *mach.base.intern.Interner) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a named constant's value as a reader sees it: a non-integer constant fails with its refusal

### int_wide_make

```mach
pub fun int_wide_make(bits: mach.base.wide.Wide, unsigned: bool) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an untyped integer value of 128 bits

| Parameter | |
| :--- | :--- |
| `bits` | the value's bits |
| `unsigned` | whether it reads as unsigned |
| **returns** | the value, never a failure |

### int_value

```mach
pub fun int_value(i: i64) res[CTValue, mach.front.comptime.failure.EvalFail]
```

an untyped signed integer value

| Parameter | |
| :--- | :--- |
| `i` | the value |
| **returns** | the value, never a failure |

### ct_negate

```mach
pub fun ct_negate(v: CTValue) res[CTValue, mach.front.comptime.failure.EvalFail]
```

-x over the untyped 128-bit range: the signed minimum negates to its own
magnitude, which only reads as unsigned; an unsigned magnitude past it has no
negation

### float_value

```mach
pub fun float_value(f: f64, w: mach.base.float.FloatWidth) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a float value as an evaluation result, rounded to its width

| Parameter | |
| :--- | :--- |
| `f` | the value |
| `w` | its width, FLOAT_W_NONE when not yet read |
| **returns** | the value, never a failure |

### ct_float

```mach
pub fun ct_float(f: f64, w: mach.base.float.FloatWidth) CTValue
```

a float value rounded to its width

| Parameter | |
| :--- | :--- |
| `f` | the value |
| `w` | its width, FLOAT_W_NONE when not yet read |
| **returns** | the value |

### u1_value

```mach
pub fun u1_value(held: bool) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a u1 truth value as an evaluation result

| Parameter | |
| :--- | :--- |
| `held` | the truth |
| **returns** | the value, never a failure |

### str_value

```mach
pub fun str_value(s: mach.base.intern.StrId) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a string value as an evaluation result

| Parameter | |
| :--- | :--- |
| `s` | the interned string |
| **returns** | the value, never a failure |

### type_value

```mach
pub fun type_value(t: u32) res[CTValue, mach.front.comptime.failure.EvalFail]
```

a type value as an evaluation result

| Parameter | |
| :--- | :--- |
| `t` | the type id |
| **returns** | the value, never a failure |

### field_value

```mach
pub fun field_value(owner: u32, index: u32) CTValue
```

a field descriptor of `$fields(T)`

| Parameter | |
| :--- | :--- |
| `owner` | the type id that declares the field |
| `index` | the field's position in it |
| **returns** | the descriptor |

### case_value

```mach
pub fun case_value(owner: u32, index: u32) CTValue
```

a case descriptor of `$cases(T)`

| Parameter | |
| :--- | :--- |
| `owner` | the tag's type id |
| `index` | the case's position in it |
| **returns** | the descriptor |

### pack_elem_value

```mach
pub fun pack_elem_value(index: u32, ty: u32) CTValue
```

an element of a variadic pack

| Parameter | |
| :--- | :--- |
| `index` | the element's position in the pack |
| `ty` | its type id |
| **returns** | the value |

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

a constant element read at type `ty`

| Parameter | |
| :--- | :--- |
| `elem` | the index of the element's expression id |
| `ty` | its type id, TYPE_NIL's index while it is not yet known |
| **returns** | the value |

### node_ref

```mach
pub fun node_ref(module: u32, shape: u32, at: u32, ty: u32) CTValue
```

a node of a module's published constant values

| Parameter | |
| :--- | :--- |
| `module` | the stable id of the module whose store holds it |
| `shape` | the node's shape in that store |
| `at` | the offset of its payload |
| `ty` | the type id it is read at |
| **returns** | the value |

### ct_str

```mach
pub fun ct_str(s: mach.base.intern.StrId) CTValue
```

a string value

| Parameter | |
| :--- | :--- |
| `s` | the interned string |
| **returns** | the value |

### ct_u1

```mach
pub fun ct_u1(held: bool) CTValue
```

the truth value `held`, a u1: a comparison's answer, and the type a condition requires

### is_u1

```mach
pub fun is_u1(v: CTValue) bool
```

whether a value is a u1

| Parameter | |
| :--- | :--- |
| `v` | the value |
| **returns** | true for an unsigned integer one bit wide |

### truth

```mach
pub fun truth(v: CTValue) bool
```

whether a value holds the truth

| Parameter | |
| :--- | :--- |
| `v` | the value |
| **returns** | true for an integer equal to 1 of any width, false for anything else |

### ct_type

```mach
pub fun ct_type(t: u32) CTValue
```

a type value

| Parameter | |
| :--- | :--- |
| `t` | the type id |
| **returns** | the value |

### ct_uint

```mach
pub fun ct_uint(n: u64) CTValue
```

a u64 value

| Parameter | |
| :--- | :--- |
| `n` | the value |
| **returns** | an unsigned 64-bit integer value |

### ct_zero_int

```mach
pub fun ct_zero_int(unsigned: bool, width: u8) CTValue
```

an integer zero of a given type

| Parameter | |
| :--- | :--- |
| `unsigned` | whether the type is unsigned |
| `width` | its width in bits, 0 for untyped |
| **returns** | the value |

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
pub fun float_negated(x: f64, w: mach.base.float.FloatWidth) f64
```

the negation of a float at width `w`, folded

### complement

```mach
pub fun complement(x: mach.base.wide.Wide) mach.base.wide.Wide
```

the bitwise complement of an integer, folded at comptime's width

