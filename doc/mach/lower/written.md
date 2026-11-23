[mach](../README.md) / [lower](README.md) / **written**

# mach.lower.written

the language's own spellings of target instructions, which decide the `intrinsic` calls it refuses

mirl states what the ir knows each op of an own set as, in its own terms and with no word of
a language (`mirl.target.capability.Known`): the ir forms the target writes as the op, over
which values, or the part of a module's structure it is. this module holds the other half,
which only the language knows: which constructs write the operation of an ir form, in the
words a diagnostic says, and why an op no form writes is still not available. an `intrinsic`
of an op is refused where a construct writes its operation by emitting the op's form, so each
operation has one spelling, where the op is structure, which the language states by
declaration, and where the op is listed here as not available. the facts stay apart because
mirl serves front ends that are not mach, and mach's constructs are no fact of the ir

Source: [`src/lower/written.mach`](../../../src/lower/written.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Written`](#written) | `rec` | a construct of the language that writes the operation of an ir form over some values |
| [`Unavailable`](#unavailable) | `rec` | an op of a set no ir form writes that the language still has no use of, as one over a type it does not declare |
| [`Refusal`](#refusal) | `tag` | why the language refuses an `intrinsic` of an op |

| On `Refusal` | |
| :--- | :--- |
| [`refused`](#refused) | why an `intrinsic` of the op `op` of the set `set` is refused, none where it is not: `s` is the set in the target's catalog, which states what the ir knows the op as |

| Functions | |
| :--- | :--- |
| [`structure_words`](#structure_words) | how the language states a part of a module's structure, which a refusal of an `intrinsic` of it says |

| Constants | |
| :--- | :--- |
| [`WRITTEN_COUNT`](#constants) | the constructs that write an ir form's operation, the form a target op could also name |
| [`WRITTEN`](#constants) | |
| [`UNAVAILABLE_COUNT`](#constants) | the ops not available to an `intrinsic`, each with its reason |
| [`UNAVAILABLE`](#constants) | |

---

## Written

```mach
pub rec Written {
    form:  Opcode;
    over:  Over;
    words: str;
    emits: bool;
}
```

a construct of the language that writes the operation of an ir form over some values

| Field | Type | |
| :--- | :--- | :--- |
| `form` | `Opcode` | the ir form |
| `over` | `Over` | the values it is over, as mirl's facts name them |
| `words` | `str` | the construct, as a diagnostic names it |
| `emits` | `bool` | whether the construct emits the form itself. one that writes the operation by another instruction, as `>` by a less-than with its operands exchanged, leaves an `intrinsic` of the form the only spelling of the instruction, so it is not refused |

## Unavailable

```mach
pub rec Unavailable {
    set: str;
    op:  str;
    why: str;
}
```

an op of a set no ir form writes that the language still has no use of, as one over a type
it does not declare

| Field | Type | |
| :--- | :--- | :--- |
| `set` | `str` | the instruction set, as a target op names it |
| `op` | `str` | the op |
| `why` | `str` | why it is not available, as a diagnostic says it |

## Refusal

```mach
pub tag Refusal: u8 {
    construct:   str;
    structure:   str;
    unavailable: str;
}
```

why the language refuses an `intrinsic` of an op

| Case | Payload | |
| :--- | :--- | :--- |
| `construct` | `str` | a construct writes the op's operation by its form, in these words |
| `structure` | `str` | the op is a part of a module's structure, which the language states as these words say |
| `unavailable` | `str` | the op is not available, for this reason |

### refused

```mach
pub fun refused(s: *mirl.target.capability.OpSet, set: View, op: View) opt[Refusal]
```

why an `intrinsic` of the op `op` of the set `set` is refused, none where it is not: `s` is
the set in the target's catalog, which states what the ir knows the op as

## Functions

### structure_words

```mach
pub fun structure_words(k: mirl.target.capability.Structure) str
```

how the language states a part of a module's structure, which a refusal of an `intrinsic`
of it says

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `WRITTEN_COUNT` | `usize` | `65` | the constructs that write an ir form's operation, the form a target op could also name |
| `WRITTEN` | `[65]Written` | `` [65]Written{ Written{form: opcodes.ADD, over: OVER_INTEGER, words: "the `+` operator on integers", emits: true}, Written{form: opcodes.SUB, over: OVER_INTEGER, words: "the `-` operator on integers", emits: true}, Written{form: opcodes.MUL, over: OVER_INTEGER, words: "the `*` operator on integers", emits: true}, Written{form: opcodes.DIV_S, over: OVER_INTEGER, words: "the `/` operator on signed integers", emits: true}, Written{form: opcodes.DIV_U, over: OVER_INTEGER, words: "the `/` operator on unsigned integers", emits: true}, Written{form: opcodes.REM_S, over: OVER_INTEGER, words: "the `%` operator on signed integers", emits: true}, Written{form: opcodes.REM_U, over: OVER_INTEGER, words: "the `%` operator on unsigned integers", emits: true}, Written{form: opcodes.AND, over: OVER_INTEGER, words: "the `&` operator", emits: true}, Written{form: opcodes.OR, over: OVER_INTEGER, words: "the `\|` operator", emits: true}, Written{form: opcodes.XOR, over: OVER_INTEGER, words: "the `^` operator", emits: true}, Written{form: opcodes.SHL, over: OVER_INTEGER, words: "the `<<` operator", emits: true}, Written{form: opcodes.SHR_U, over: OVER_INTEGER, words: "the `>>` operator on unsigned integers", emits: true}, Written{form: opcodes.SHR_S, over: OVER_INTEGER, words: "the `>>` operator on signed integers", emits: true}, Written{form: opcodes.NEG, over: OVER_INTEGER, words: "unary `-` on an integer", emits: true}, Written{form: opcodes.NOT, over: OVER_INTEGER, words: "the `~` operator", emits: true}, Written{form: opcodes.EQ, over: OVER_INTEGER, words: "the `==` operator on integers", emits: true}, Written{form: opcodes.NE, over: OVER_INTEGER, words: "the `!=` operator on integers", emits: true}, Written{form: opcodes.LT_S, over: OVER_INTEGER, words: "the `<` operator on signed integers, or `>` with its operands exchanged", emits: true}, Written{form: opcodes.LE_S, over: OVER_INTEGER, words: "the `<=` operator on signed integers, or `>=` with its operands exchanged", emits: true}, Written{form: opcodes.LT_U, over: OVER_INTEGER, words: "the `<` operator on unsigned integers, or `>` with its operands exchanged", emits: true}, Written{form: opcodes.LE_U, over: OVER_INTEGER, words: "the `<=` operator on unsigned integers, or `>=` with its operands exchanged", emits: true}, Written{form: opcodes.GT_S, over: OVER_INTEGER, words: "the `>` operator on signed integers, which is a `<` with its operands exchanged", emits: false}, Written{form: opcodes.GE_S, over: OVER_INTEGER, words: "the `>=` operator on signed integers, which is a `<=` with its operands exchanged", emits: false}, Written{form: opcodes.GT_U, over: OVER_INTEGER, words: "the `>` operator on unsigned integers, which is a `<` with its operands exchanged", emits: false}, Written{form: opcodes.GE_U, over: OVER_INTEGER, words: "the `>=` operator on unsigned integers, which is a `<=` with its operands exchanged", emits: false}, Written{form: opcodes.AND, over: OVER_TRUTH, words: "the `&&` operator, which branches", emits: false}, Written{form: opcodes.OR, over: OVER_TRUTH, words: "the `\|\|` operator, which branches", emits: false}, Written{form: opcodes.NOT, over: OVER_TRUTH, words: "the `!` operator, which compares with zero", emits: false}, Written{form: opcodes.EQ, over: OVER_TRUTH, words: "the `==` operator on `bool` values, which compares them as integers", emits: false}, Written{form: opcodes.NE, over: OVER_TRUTH, words: "the `!=` operator on `bool` values, which compares them as integers", emits: false}, Written{form: opcodes.FADD, over: OVER_FLOAT, words: "the `+` operator on floats", emits: true}, Written{form: opcodes.FSUB, over: OVER_FLOAT, words: "the `-` operator on floats", emits: true}, Written{form: opcodes.FMUL, over: OVER_FLOAT, words: "the `*` operator on floats", emits: true}, Written{form: opcodes.FDIV, over: OVER_FLOAT, words: "the `/` operator on floats", emits: true}, Written{form: opcodes.FREM, over: OVER_FLOAT, words: "the `%` operator on floats, which a helper computes exactly", emits: false}, Written{form: opcodes.FNEG, over: OVER_FLOAT, words: "unary `-` on a float", emits: true}, Written{form: opcodes.FEQ_O, over: OVER_FLOAT, words: "the `==` operator on floats", emits: true}, Written{form: opcodes.FNE_U, over: OVER_FLOAT, words: "the `!=` operator on floats", emits: true}, Written{form: opcodes.FLT_O, over: OVER_FLOAT, words: "the `<` operator on floats, or `>` with its operands exchanged", emits: true}, Written{form: opcodes.FLE_O, over: OVER_FLOAT, words: "the `<=` operator on floats, or `>=` with its operands exchanged", emits: true}, Written{form: opcodes.FGT_O, over: OVER_FLOAT, words: "the `>` operator on floats, which is a `<` with its operands exchanged", emits: false}, Written{form: opcodes.FGE_O, over: OVER_FLOAT, words: "the `>=` operator on floats, which is a `<=` with its operands exchanged", emits: false}, Written{form: opcodes.EXT_S, over: OVER_INTEGER, words: "a converting cast between signed integer widths, `x::T`", emits: true}, Written{form: opcodes.EXT_U, over: OVER_INTEGER, words: "a converting cast between integer widths, `x::T`", emits: true}, Written{form: opcodes.TRUNC, over: OVER_INTEGER, words: "a converting cast between integer widths, `x::T`", emits: true}, Written{form: opcodes.FPROMOTE, over: OVER_FLOAT, words: "a converting cast between float widths, `x::T`", emits: true}, Written{form: opcodes.FDEMOTE, over: OVER_FLOAT, words: "a converting cast between float widths, `x::T`", emits: true}, Written{form: opcodes.FTOI_S, over: OVER_FLOAT, words: "a converting cast from a float to a signed integer, `x::T`", emits: true}, Written{form: opcodes.FTOI_U, over: OVER_FLOAT, words: "a converting cast from a float to an unsigned integer, `x::T`", emits: true}, Written{form: opcodes.ITOF_S, over: OVER_INTEGER, words: "a converting cast from a signed integer to a float, `x::T`", emits: true}, Written{form: opcodes.ITOF_U, over: OVER_INTEGER, words: "a converting cast from an unsigned integer to a float, `x::T`", emits: true}, Written{form: opcodes.BITCAST, over: OVER_ANY, words: "a reinterpreting cast, `x:~T`", emits: true}, Written{form: opcodes.MUL_HIGH_S, over: OVER_INTEGER, words: "a widening multiply of signed integers, `(a::i64) * (b::i64)` of two `i32`", emits: true}, Written{form: opcodes.MUL_HIGH_U, over: OVER_INTEGER, words: "a widening multiply of unsigned integers, `(a::u64) * (b::u64)` of two `u32`", emits: true}, Written{form: opcodes.VEXTRACT, over: OVER_ANY, words: "a lane or member of a value, as `v[0]`", emits: true}, Written{form: opcodes.EXTRACT, over: OVER_ANY, words: "a lane or member of a value, as `v[0]`", emits: true}, Written{form: opcodes.VINSERT, over: OVER_ANY, words: "an assignment to a lane or member of a value, as `v[0] = x`", emits: true}, Written{form: opcodes.INSERT, over: OVER_ANY, words: "an assignment to a lane or member of a value, as `v[0] = x`", emits: true}, Written{form: opcodes.VBUILD, over: OVER_ANY, words: "a vector or record literal, as `f32x4{a, b, c, d}`", emits: true}, Written{form: opcodes.LOAD, over: OVER_ANY, words: "reading a variable or through a pointer, as `x` or `@p`", emits: true}, Written{form: opcodes.STORE, over: OVER_ANY, words: "an assignment to a variable or through a pointer, as `@p = x`", emits: true}, Written{form: opcodes.CALL, over: OVER_ANY, words: "a call of a function", emits: true}, Written{form: opcodes.PTR_MEMBER, over: OVER_LOGICAL, words: "a member access or an index, as `r.f` or `a[i]`", emits: true}, Written{form: opcodes.PTR_INDEX, over: OVER_LOGICAL, words: "a member access or an index, as `r.f` or `a[i]`", emits: true}, Written{form: opcodes.PTR_INDEX, over: OVER_PHYSICAL, words: "an index on a physical pointer, `p[i]`", emits: true}, } `` |  |
| `UNAVAILABLE_COUNT` | `usize` | `21` | the ops not available to an `intrinsic`, each with its reason |
| `UNAVAILABLE` | `[21]Unavailable` | `` [21]Unavailable{ Unavailable{set: "core", op: "OpCopyLogical", why: "it copies between two distinct types alike member for member, and records of the same members are one type. assign the value instead"}, Unavailable{set: "core", op: "OpUndef", why: "an undefined value is never written: a `var` is zeroed and a `val` is initialized"}, Unavailable{set: "core", op: "OpUntypedAccessChainKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedInBoundsAccessChainKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedInBoundsPtrAccessChainKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedPtrAccessChainKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedVariableKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedArrayLengthKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedGroupAsyncCopyKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedImageTexelPointerEXT", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedPrefetchKHR", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpUntypedVariableLengthArrayINTEL", why: "it operates on a SPIR-V untyped pointer, which no declaration can state. write the access through a typed pointer"}, Unavailable{set: "core", op: "OpInBoundsPtrAccessChain", why: "its element is the pointee's stride, which a physical pointer, an address, does not carry, and a logical pointer takes only under variable pointers into an array with an explicit stride, which no declaration can state. index the array instead"}, Unavailable{set: "core", op: "OpTranspose", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "core", op: "OpOuterProduct", why: "its result is a matrix, which no declaration can state. write the operation over vectors"}, Unavailable{set: "core", op: "OpMatrixTimesScalar", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "core", op: "OpMatrixTimesVector", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "core", op: "OpVectorTimesMatrix", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "core", op: "OpMatrixTimesMatrix", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "GLSL.std.450", op: "Determinant", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, Unavailable{set: "GLSL.std.450", op: "MatrixInverse", why: "it is over matrices, which no declaration can state. write the operation over vectors"}, } `` |  |

