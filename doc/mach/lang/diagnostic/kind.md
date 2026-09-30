# mach.lang.diagnostic.kind

the diagnostic code registry

one closed table, and the only place a kind is declared. every diagnostic
names its row when it is raised, and each row carries a dotted key named by
the subject it concerns: the key is the diagnostic's stable code, which the
renderer prints and a profile's `allow` and a declaration's `#[expect]`
select by. a key's leading components name a family, so `vector` selects
every key under `vector.`. the table is append-only: a row is never removed
or reordered and its key never changes meaning, and a retired kind keeps its
row so its key is never reused. only a warning row can be silenced or
expected; a selection naming an error row is refused rather than read as
unknown

## def Kind

```mach
pub def Kind: u16
```

## def Level

```mach
pub def Level: u8
```

## val LEVEL_WARNING

```mach
pub val LEVEL_WARNING: Level = 0
```

## val LEVEL_ERROR

```mach
pub val LEVEL_ERROR:   Level = 1
```

## val NONE

```mach
pub val NONE: Kind = 0
```

the zero kind names no row, and no diagnostic may be raised with it

## val IMPORT_UNUSED

```mach
pub val IMPORT_UNUSED:                    Kind = 1
```

## val DECL_DEPRECATED

```mach
pub val DECL_DEPRECATED:                  Kind = 2
```

## val DOC_LINT

```mach
pub val DOC_LINT:                         Kind = 3
```

## val FWD_INSTANCES

```mach
pub val FWD_INSTANCES:                    Kind = 4
```

## val DEBUG_DROPPED

```mach
pub val DEBUG_DROPPED:                    Kind = 5
```

## val TARGET_SKIPPED

```mach
pub val TARGET_SKIPPED:                   Kind = 6
```

## val TARGET_NATIVE_FALLBACK

```mach
pub val TARGET_NATIVE_FALLBACK:           Kind = 7
```

## val SECRET_NOT_OBLIVIOUS

```mach
pub val SECRET_NOT_OBLIVIOUS:             Kind = 8
```

## val FLOAT_INEXACT

```mach
pub val FLOAT_INEXACT:                    Kind = 9
```

## val VECTOR_SCALARIZE

```mach
pub val VECTOR_SCALARIZE:                 Kind = 10
```

## val EXPECT_UNFULFILLED

```mach
pub val EXPECT_UNFULFILLED:               Kind = 11
```

## val ABI_TYPE_HAS_BODY

```mach
pub val ABI_TYPE_HAS_BODY:                Kind = 12
```

## val ABI_TYPE_OPAQUE

```mach
pub val ABI_TYPE_OPAQUE:                  Kind = 13
```

## val ABI_TYPE_UNAVAILABLE

```mach
pub val ABI_TYPE_UNAVAILABLE:             Kind = 14
```

## val ABI_TYPE_UNKNOWN

```mach
pub val ABI_TYPE_UNKNOWN:                 Kind = 15
```

## val ADDRESS_COMPTIME_PARAM

```mach
pub val ADDRESS_COMPTIME_PARAM:           Kind = 16
```

## val ADDRESS_GENERIC_FUNCTION

```mach
pub val ADDRESS_GENERIC_FUNCTION:         Kind = 17
```

## val ADDRESS_PACKED_MEMBER

```mach
pub val ADDRESS_PACKED_MEMBER:            Kind = 18
```

## val ADDRESS_TEMPORARY

```mach
pub val ADDRESS_TEMPORARY:                Kind = 19
```

## val ALLOCA_VARIABLE_SIZE

```mach
pub val ALLOCA_VARIABLE_SIZE:             Kind = 20
```

## val ARRAY_INFERRED_LENGTH

```mach
pub val ARRAY_INFERRED_LENGTH:            Kind = 21
```

## val ARRAY_LENGTH_NOT_CONST

```mach
pub val ARRAY_LENGTH_NOT_CONST:           Kind = 22
```

## val ARRAY_LENGTH_VALUE

```mach
pub val ARRAY_LENGTH_VALUE:               Kind = 23
```

## val ARRAY_LITERAL_LENGTH

```mach
pub val ARRAY_LITERAL_LENGTH:             Kind = 24
```

## val ASM_DIRECTIVE

```mach
pub val ASM_DIRECTIVE:                    Kind = 25
```

## val ASM_EFFECTS

```mach
pub val ASM_EFFECTS:                      Kind = 26
```

## val ASM_EXTENSION

```mach
pub val ASM_EXTENSION:                    Kind = 27
```

## val ASM_IMMEDIATE_RANGE

```mach
pub val ASM_IMMEDIATE_RANGE:              Kind = 28
```

## val ASM_INSTRUCTION_UNAVAILABLE

```mach
pub val ASM_INSTRUCTION_UNAVAILABLE:      Kind = 29
```

## val ASM_ISA_MISSING

```mach
pub val ASM_ISA_MISSING:                  Kind = 30
```

## val ASM_LABEL

```mach
pub val ASM_LABEL:                        Kind = 31
```

## val ASM_LOCAL

```mach
pub val ASM_LOCAL:                        Kind = 32
```

## val ASM_OPERAND

```mach
pub val ASM_OPERAND:                      Kind = 33
```

## val ASM_SYNTAX

```mach
pub val ASM_SYNTAX:                       Kind = 34
```

## val ASM_TARGET_MISMATCH

```mach
pub val ASM_TARGET_MISMATCH:              Kind = 35
```

## val ASM_UNAVAILABLE

```mach
pub val ASM_UNAVAILABLE:                  Kind = 36
```

## val ASM_UNKNOWN_INSTRUCTION

```mach
pub val ASM_UNKNOWN_INSTRUCTION:          Kind = 37
```

## val ASM_UNSUPPORTED

```mach
pub val ASM_UNSUPPORTED:                  Kind = 38
```

## val ASM_UNTERMINATED

```mach
pub val ASM_UNTERMINATED:                 Kind = 39
```

## val ASSIGN_IMMUTABLE

```mach
pub val ASSIGN_IMMUTABLE:                 Kind = 40
```

## val ASSIGN_NOT_PLACE

```mach
pub val ASSIGN_NOT_PLACE:                 Kind = 41
```

## val ASSIGN_READONLY_STORAGE

```mach
pub val ASSIGN_READONLY_STORAGE:          Kind = 42
```

## val BINDING_UNTYPED

```mach
pub val BINDING_UNTYPED:                  Kind = 43
```

## val BINDING_VAL_UNINITIALIZED

```mach
pub val BINDING_VAL_UNINITIALIZED:        Kind = 44
```

## val CALL_ARITY

```mach
pub val CALL_ARITY:                       Kind = 45
```

## val CALL_C_VARIADIC_PROMOTION

```mach
pub val CALL_C_VARIADIC_PROMOTION:        Kind = 46
```

## val CALL_NOT_CALLABLE

```mach
pub val CALL_NOT_CALLABLE:                Kind = 47
```

## val CALL_SPREAD

```mach
pub val CALL_SPREAD:                      Kind = 48
```

## val CAST_INVALID

```mach
pub val CAST_INVALID:                     Kind = 49
```

## val CAST_REINTERPRET_SIZE

```mach
pub val CAST_REINTERPRET_SIZE:            Kind = 50
```

## val CAST_TAG_REPRESENTATION

```mach
pub val CAST_TAG_REPRESENTATION:          Kind = 51
```

## val CAST_VECTOR_ELEMENT

```mach
pub val CAST_VECTOR_ELEMENT:              Kind = 52
```

## val CAST_VECTOR_SHAPE

```mach
pub val CAST_VECTOR_SHAPE:                Kind = 53
```

## val COMPILER_INTERNAL

```mach
pub val COMPILER_INTERNAL:                Kind = 54
```

## val COMPTIME_ARGUMENT_NOT_CONSTANT

```mach
pub val COMPTIME_ARGUMENT_NOT_CONSTANT:   Kind = 55
```

## val COMPTIME_CASE_DESCRIPTOR

```mach
pub val COMPTIME_CASE_DESCRIPTOR:         Kind = 56
```

## val COMPTIME_CASE_NO_PAYLOAD

```mach
pub val COMPTIME_CASE_NO_PAYLOAD:         Kind = 57
```

## val COMPTIME_CASE_PAYLOAD

```mach
pub val COMPTIME_CASE_PAYLOAD:            Kind = 58
```

## val COMPTIME_CASES_NOT_TAG

```mach
pub val COMPTIME_CASES_NOT_TAG:           Kind = 59
```

## val COMPTIME_CAST

```mach
pub val COMPTIME_CAST:                    Kind = 60
```

## val COMPTIME_CAST_UNTYPED

```mach
pub val COMPTIME_CAST_UNTYPED:            Kind = 61
```

## val COMPTIME_CT_MUL

```mach
pub val COMPTIME_CT_MUL:                  Kind = 62
```

## val COMPTIME_DESCRIPTOR_MEMBER

```mach
pub val COMPTIME_DESCRIPTOR_MEMBER:       Kind = 63
```

## val COMPTIME_DISCRIMINANT_OF

```mach
pub val COMPTIME_DISCRIMINANT_OF:         Kind = 64
```

## val COMPTIME_DIVISION_BY_ZERO

```mach
pub val COMPTIME_DIVISION_BY_ZERO:        Kind = 65
```

## val COMPTIME_ERROR_DIRECTIVE

```mach
pub val COMPTIME_ERROR_DIRECTIVE:         Kind = 66
```

## val COMPTIME_EXTENSION_UNKNOWN

```mach
pub val COMPTIME_EXTENSION_UNKNOWN:       Kind = 67
```

## val COMPTIME_FIELD_DESCRIPTOR

```mach
pub val COMPTIME_FIELD_DESCRIPTOR:        Kind = 68
```

## val COMPTIME_FIELD_TYPE_BINDING

```mach
pub val COMPTIME_FIELD_TYPE_BINDING:      Kind = 69
```

## val COMPTIME_FIELDS_NOT_RECORD

```mach
pub val COMPTIME_FIELDS_NOT_RECORD:       Kind = 70
```

## val COMPTIME_FLOAT_WIDTH_MISMATCH

```mach
pub val COMPTIME_FLOAT_WIDTH_MISMATCH:    Kind = 71
```

## val COMPTIME_GATE_ONLY

```mach
pub val COMPTIME_GATE_ONLY:               Kind = 72
```

## val COMPTIME_INTRINSIC_ARITY

```mach
pub val COMPTIME_INTRINSIC_ARITY:         Kind = 73
```

## val COMPTIME_LENGTH_OF

```mach
pub val COMPTIME_LENGTH_OF:               Kind = 74
```

## val COMPTIME_NEEDS_INSTANCE

```mach
pub val COMPTIME_NEEDS_INSTANCE:          Kind = 75
```

## val COMPTIME_NOT_CONSTANT

```mach
pub val COMPTIME_NOT_CONSTANT:            Kind = 76
```

## val COMPTIME_NOT_EVALUABLE

```mach
pub val COMPTIME_NOT_EVALUABLE:           Kind = 77
```

## val COMPTIME_NOT_INTEGER

```mach
pub val COMPTIME_NOT_INTEGER:             Kind = 78
```

## val COMPTIME_OFFSET_OF

```mach
pub val COMPTIME_OFFSET_OF:               Kind = 79
```

## val COMPTIME_OPERAND_TYPE

```mach
pub val COMPTIME_OPERAND_TYPE:            Kind = 80
```

## val COMPTIME_OVERFLOW

```mach
pub val COMPTIME_OVERFLOW:                Kind = 81
```

## val COMPTIME_PARAM_FN_VALUE

```mach
pub val COMPTIME_PARAM_FN_VALUE:          Kind = 82
```

## val COMPTIME_PATH_RESERVED

```mach
pub val COMPTIME_PATH_RESERVED:           Kind = 83
```

## val COMPTIME_PATH_ROOT

```mach
pub val COMPTIME_PATH_ROOT:               Kind = 84
```

## val COMPTIME_PATH_UNAVAILABLE

```mach
pub val COMPTIME_PATH_UNAVAILABLE:        Kind = 85
```

## val COMPTIME_PATH_UNEVALUABLE

```mach
pub val COMPTIME_PATH_UNEVALUABLE:        Kind = 86
```

## val COMPTIME_PATH_UNKNOWN

```mach
pub val COMPTIME_PATH_UNKNOWN:            Kind = 87
```

## val COMPTIME_POINTEE_OF

```mach
pub val COMPTIME_POINTEE_OF:              Kind = 88
```

## val COMPTIME_SHIFT_COUNT

```mach
pub val COMPTIME_SHIFT_COUNT:             Kind = 89
```

## val COMPTIME_TOO_EARLY

```mach
pub val COMPTIME_TOO_EARLY:               Kind = 90
```

## val COMPTIME_TYPE_ARGUMENT

```mach
pub val COMPTIME_TYPE_ARGUMENT:           Kind = 91
```

## val COMPTIME_TYPE_CONSTRUCTOR

```mach
pub val COMPTIME_TYPE_CONSTRUCTOR:        Kind = 92
```

## val COMPTIME_TYPE_ID

```mach
pub val COMPTIME_TYPE_ID:                 Kind = 93
```

## val COMPTIME_TYPE_NOT_VALUE

```mach
pub val COMPTIME_TYPE_NOT_VALUE:          Kind = 94
```

## val COMPTIME_TYPE_OPERAND

```mach
pub val COMPTIME_TYPE_OPERAND:            Kind = 95
```

## val COMPTIME_TYPE_UNRESOLVED

```mach
pub val COMPTIME_TYPE_UNRESOLVED:         Kind = 96
```

## val COMPTIME_UNMEASURABLE

```mach
pub val COMPTIME_UNMEASURABLE:            Kind = 97
```

## val COMPTIME_USER_ERROR

```mach
pub val COMPTIME_USER_ERROR:              Kind = 98
```

## val CONDITION_TYPE

```mach
pub val CONDITION_TYPE:                   Kind = 99
```

## val CONST_OUT_OF_RANGE

```mach
pub val CONST_OUT_OF_RANGE:               Kind = 100
```

## val CONST_TUPLE_DEPENDENT

```mach
pub val CONST_TUPLE_DEPENDENT:            Kind = 101
```

## val DECL_VECTOR_SPELLED_NAME

```mach
pub val DECL_VECTOR_SPELLED_NAME:         Kind = 102
```

## val DECORATOR_ALIGN

```mach
pub val DECORATOR_ALIGN:                  Kind = 103
```

## val DECORATOR_ARGUMENT

```mach
pub val DECORATOR_ARGUMENT:               Kind = 104
```

## val DECORATOR_ARITY

```mach
pub val DECORATOR_ARITY:                  Kind = 105
```

## val DECORATOR_CONFLICT

```mach
pub val DECORATOR_CONFLICT:               Kind = 106
```

## val DECORATOR_DUPLICATE

```mach
pub val DECORATOR_DUPLICATE:              Kind = 107
```

## val DECORATOR_MISPLACED

```mach
pub val DECORATOR_MISPLACED:              Kind = 108
```

## val DECORATOR_REQUIRES

```mach
pub val DECORATOR_REQUIRES:               Kind = 109
```

## val DECORATOR_UNKNOWN

```mach
pub val DECORATOR_UNKNOWN:                Kind = 110
```

## val EACH_ELEMENT_TYPE

```mach
pub val EACH_ELEMENT_TYPE:                Kind = 111
```

## val EACH_INITIALIZER

```mach
pub val EACH_INITIALIZER:                 Kind = 112
```

## val EACH_MUTABLE

```mach
pub val EACH_MUTABLE:                     Kind = 113
```

## val EACH_SEQUENCE

```mach
pub val EACH_SEQUENCE:                    Kind = 114
```

## val EMBED_ARGUMENT

```mach
pub val EMBED_ARGUMENT:                   Kind = 115
```

## val EMBED_ESCAPES_ROOT

```mach
pub val EMBED_ESCAPES_ROOT:               Kind = 116
```

## val EMBED_INITIALIZER

```mach
pub val EMBED_INITIALIZER:                Kind = 117
```

## val EMBED_NO_LOCATION

```mach
pub val EMBED_NO_LOCATION:                Kind = 118
```

## val EMBED_PATH_TEMPLATE

```mach
pub val EMBED_PATH_TEMPLATE:              Kind = 119
```

## val EMBED_SIZE

```mach
pub val EMBED_SIZE:                       Kind = 120
```

## val EMBED_TARGET

```mach
pub val EMBED_TARGET:                     Kind = 121
```

## val EMBED_TYPE

```mach
pub val EMBED_TYPE:                       Kind = 122
```

## val EMBED_UNREADABLE

```mach
pub val EMBED_UNREADABLE:                 Kind = 123
```

## val EXPECT_DUPLICATE_KEY

```mach
pub val EXPECT_DUPLICATE_KEY:             Kind = 124
```

## val EXPECT_ERROR_KEY

```mach
pub val EXPECT_ERROR_KEY:                 Kind = 125
```

## val EXPECT_UNKNOWN_KEY

```mach
pub val EXPECT_UNKNOWN_KEY:               Kind = 126
```

## val EXT_DATA_INITIALIZER

```mach
pub val EXT_DATA_INITIALIZER:             Kind = 127
```

## val EXT_VARIADIC_BODY

```mach
pub val EXT_VARIADIC_BODY:                Kind = 128
```

## val EXTENSION_DUPLICATE

```mach
pub val EXTENSION_DUPLICATE:              Kind = 129
```

## val EXTENSION_TARGET_ONLY

```mach
pub val EXTENSION_TARGET_ONLY:            Kind = 130
```

## val EXTENSION_UNKNOWN

```mach
pub val EXTENSION_UNKNOWN:                Kind = 131
```

## val FIELD_DUPLICATE_INIT

```mach
pub val FIELD_DUPLICATE_INIT:             Kind = 132
```

## val FIELD_MISSING_VALUE

```mach
pub val FIELD_MISSING_VALUE:              Kind = 133
```

## val FIELD_UNKNOWN

```mach
pub val FIELD_UNKNOWN:                    Kind = 134
```

## val FIELD_VOID_RECEIVER

```mach
pub val FIELD_VOID_RECEIVER:              Kind = 135
```

## val FIN_JUMP

```mach
pub val FIN_JUMP:                         Kind = 136
```

## val FIN_RET

```mach
pub val FIN_RET:                          Kind = 137
```

## val FWD_NOT_SYMBOL

```mach
pub val FWD_NOT_SYMBOL:                   Kind = 138
```

## val FWD_PUB

```mach
pub val FWD_PUB:                          Kind = 139
```

## val GATE_CONST_NOT_INTEGER

```mach
pub val GATE_CONST_NOT_INTEGER:           Kind = 140
```

## val GATE_CYCLE

```mach
pub val GATE_CYCLE:                       Kind = 141
```

## val GATE_NOT_CONSTANT

```mach
pub val GATE_NOT_CONSTANT:                Kind = 142
```

## val GATE_NOT_U8

```mach
pub val GATE_NOT_U8:                      Kind = 143
```

## val GATE_RUNTIME_VALUE

```mach
pub val GATE_RUNTIME_VALUE:               Kind = 144
```

## val GATE_TYPE_QUESTION

```mach
pub val GATE_TYPE_QUESTION:               Kind = 145
```

## val GENERIC_ARGUMENT

```mach
pub val GENERIC_ARGUMENT:                 Kind = 146
```

## val GENERIC_ARITY

```mach
pub val GENERIC_ARITY:                    Kind = 147
```

## val GENERIC_COMPTIME_VALUE_PARAM

```mach
pub val GENERIC_COMPTIME_VALUE_PARAM:     Kind = 148
```

## val GENERIC_INSTANTIATION_LIMIT

```mach
pub val GENERIC_INSTANTIATION_LIMIT:      Kind = 149
```

## val GENERIC_MISSING_TYPE_ARGS

```mach
pub val GENERIC_MISSING_TYPE_ARGS:        Kind = 150
```

## val GENERIC_NOT_GENERIC

```mach
pub val GENERIC_NOT_GENERIC:              Kind = 151
```

## val GLOBAL_INIT_NOT_CONSTANT

```mach
pub val GLOBAL_INIT_NOT_CONSTANT:         Kind = 152
```

## val GUARD_ASSIGN

```mach
pub val GUARD_ASSIGN:                     Kind = 153
```

## val GUARD_UNSTABLE_PLACE

```mach
pub val GUARD_UNSTABLE_PLACE:             Kind = 154
```

## val HANDLE_ARGUMENTS

```mach
pub val HANDLE_ARGUMENTS:                 Kind = 155
```

## val HANDLE_HAS_BODY

```mach
pub val HANDLE_HAS_BODY:                  Kind = 156
```

## val HANDLE_NOT_DEF

```mach
pub val HANDLE_NOT_DEF:                   Kind = 157
```

## val HANDLE_OPERAND_COUNT

```mach
pub val HANDLE_OPERAND_COUNT:             Kind = 158
```

## val HANDLE_OPERAND_REFUSED

```mach
pub val HANDLE_OPERAND_REFUSED:           Kind = 159
```

## val HANDLE_OPERAND_TYPE

```mach
pub val HANDLE_OPERAND_TYPE:              Kind = 160
```

## val HANDLE_OPERAND_WORD

```mach
pub val HANDLE_OPERAND_WORD:              Kind = 161
```

## val HANDLE_ROLE

```mach
pub val HANDLE_ROLE:                      Kind = 162
```

## val HANDLE_STORAGE

```mach
pub val HANDLE_STORAGE:                   Kind = 163
```

## val HANDLE_UNBOUND

```mach
pub val HANDLE_UNBOUND:                   Kind = 164
```

## val HANDLE_UNKNOWN_CONSTRUCTOR

```mach
pub val HANDLE_UNKNOWN_CONSTRUCTOR:       Kind = 165
```

## val INDEX_NOT_INDEXABLE

```mach
pub val INDEX_NOT_INDEXABLE:              Kind = 166
```

## val INDEX_OUT_OF_BOUNDS

```mach
pub val INDEX_OUT_OF_BOUNDS:              Kind = 167
```

## val INDEX_TYPE

```mach
pub val INDEX_TYPE:                       Kind = 168
```

## val LAYOUT_CYCLE

```mach
pub val LAYOUT_CYCLE:                     Kind = 169
```

## val LAYOUT_MEASUREMENT_OVERFLOW

```mach
pub val LAYOUT_MEASUREMENT_OVERFLOW:      Kind = 170
```

## val LAYOUT_UNAVAILABLE

```mach
pub val LAYOUT_UNAVAILABLE:               Kind = 171
```

## val LITERAL_INVALID_ESCAPE

```mach
pub val LITERAL_INVALID_ESCAPE:           Kind = 172
```

## val LITERAL_MALFORMED

```mach
pub val LITERAL_MALFORMED:                Kind = 173
```

## val LITERAL_OUT_OF_RANGE

```mach
pub val LITERAL_OUT_OF_RANGE:             Kind = 174
```

## val LITERAL_UNTERMINATED_CHAR

```mach
pub val LITERAL_UNTERMINATED_CHAR:        Kind = 175
```

## val LITERAL_UNTERMINATED_STRING

```mach
pub val LITERAL_UNTERMINATED_STRING:      Kind = 176
```

## val MODULE_ALIAS_EXPECTED

```mach
pub val MODULE_ALIAS_EXPECTED:            Kind = 177
```

## val MODULE_NOT_IN_DEPS

```mach
pub val MODULE_NOT_IN_DEPS:               Kind = 178
```

## val NAKED_BODY

```mach
pub val NAKED_BODY:                       Kind = 179
```

## val NAME_DUPLICATE

```mach
pub val NAME_DUPLICATE:                   Kind = 180
```

## val NAME_NOT_TYPE

```mach
pub val NAME_NOT_TYPE:                    Kind = 181
```

## val NAME_NOT_VALUE

```mach
pub val NAME_NOT_VALUE:                   Kind = 182
```

## val NAME_UNRESOLVED

```mach
pub val NAME_UNRESOLVED:                  Kind = 183
```

## val OP_OPERAND_COUNT

```mach
pub val OP_OPERAND_COUNT:                 Kind = 184
```

## val OP_UNKNOWN

```mach
pub val OP_UNKNOWN:                       Kind = 185
```

## val OPERATOR_AGGREGATE_EQUALITY

```mach
pub val OPERATOR_AGGREGATE_EQUALITY:      Kind = 186
```

## val OPERATOR_OPERAND_MISMATCH

```mach
pub val OPERATOR_OPERAND_MISMATCH:        Kind = 187
```

## val OPERATOR_OPERAND_TYPE

```mach
pub val OPERATOR_OPERAND_TYPE:            Kind = 188
```

## val PACK_BODYLESS

```mach
pub val PACK_BODYLESS:                    Kind = 189
```

## val PACK_MEMBER

```mach
pub val PACK_MEMBER:                      Kind = 190
```

## val PACK_SPREAD

```mach
pub val PACK_SPREAD:                      Kind = 191
```

## val PTR_DEREF_NON_POINTER

```mach
pub val PTR_DEREF_NON_POINTER:            Kind = 192
```

## val PTR_UNTYPED_ACCESS

```mach
pub val PTR_UNTYPED_ACCESS:               Kind = 193
```

## val RANGE_COUNT_INVALID

```mach
pub val RANGE_COUNT_INVALID:              Kind = 194
```

## val RANGE_COUNT_NOT_CONSTANT

```mach
pub val RANGE_COUNT_NOT_CONSTANT:         Kind = 195
```

## val RANGE_OUT_OF_BOUNDS

```mach
pub val RANGE_OUT_OF_BOUNDS:              Kind = 196
```

## val RET_VALUE

```mach
pub val RET_VALUE:                        Kind = 197
```

## val SECRET_ADDRESS

```mach
pub val SECRET_ADDRESS:                   Kind = 198
```

## val SECRET_BRANCH

```mach
pub val SECRET_BRANCH:                    Kind = 199
```

## val SECRET_CAST_QUALIFIER

```mach
pub val SECRET_CAST_QUALIFIER:            Kind = 200
```

## val SECRET_INDEX

```mach
pub val SECRET_INDEX:                     Kind = 201
```

## val SECRET_PTR_ERASE

```mach
pub val SECRET_PTR_ERASE:                 Kind = 202
```

## val SECRET_STRIP_TARGET

```mach
pub val SECRET_STRIP_TARGET:              Kind = 203
```

## val SECRET_TAG_CASE

```mach
pub val SECRET_TAG_CASE:                  Kind = 204
```

## val SECRET_TARGET_UNVERIFIABLE

```mach
pub val SECRET_TARGET_UNVERIFIABLE:       Kind = 205
```

## val SECRET_UNVERIFIABLE

```mach
pub val SECRET_UNVERIFIABLE:              Kind = 206
```

## val SECRET_VARIABLE_LATENCY

```mach
pub val SECRET_VARIABLE_LATENCY:          Kind = 207
```

## val SECRET_VARIADIC

```mach
pub val SECRET_VARIADIC:                  Kind = 208
```

## val SECRET_VARIANT_MISMATCH

```mach
pub val SECRET_VARIANT_MISMATCH:          Kind = 209
```

## val SECRET_VARIANT_UNPROVABLE

```mach
pub val SECRET_VARIANT_UNPROVABLE:        Kind = 210
```

## val SEL_NOT_A_CASE

```mach
pub val SEL_NOT_A_CASE:                   Kind = 211
```

## val SEL_NOT_PLACE

```mach
pub val SEL_NOT_PLACE:                    Kind = 212
```

## val SEL_NOT_TAG

```mach
pub val SEL_NOT_TAG:                      Kind = 213
```

## val SHIFT_COUNT

```mach
pub val SHIFT_COUNT:                      Kind = 214
```

## val SOURCE_CONTROL_CHAR

```mach
pub val SOURCE_CONTROL_CHAR:              Kind = 215
```

## val SOURCE_UNEXPECTED_CHAR

```mach
pub val SOURCE_UNEXPECTED_CHAR:           Kind = 216
```

## val SPIRV_CAPABILITY

```mach
pub val SPIRV_CAPABILITY:                 Kind = 217
```

## val SPIRV_CONTROL_FLOW

```mach
pub val SPIRV_CONTROL_FLOW:               Kind = 218
```

## val SPIRV_INTERFACE

```mach
pub val SPIRV_INTERFACE:                  Kind = 219
```

## val SPIRV_READONLY_STORE

```mach
pub val SPIRV_READONLY_STORE:             Kind = 220
```

## val SPIRV_RECURSION

```mach
pub val SPIRV_RECURSION:                  Kind = 221
```

## val SPIRV_RECURSIVE_TYPE

```mach
pub val SPIRV_RECURSIVE_TYPE:             Kind = 222
```

## val SPIRV_STAGE_SIGNATURE

```mach
pub val SPIRV_STAGE_SIGNATURE:            Kind = 223
```

## val SPIRV_UNSUPPORTED

```mach
pub val SPIRV_UNSUPPORTED:                Kind = 224
```

## val STACK_RESERVE_EXCEEDED

```mach
pub val STACK_RESERVE_EXCEEDED:           Kind = 225
```

## val SYNTAX_EXPECTED_DECL

```mach
pub val SYNTAX_EXPECTED_DECL:             Kind = 226
```

## val SYNTAX_EXPECTED_EXPR

```mach
pub val SYNTAX_EXPECTED_EXPR:             Kind = 227
```

## val SYNTAX_EXPECTED_NAME

```mach
pub val SYNTAX_EXPECTED_NAME:             Kind = 228
```

## val SYNTAX_EXPECTED_PATH

```mach
pub val SYNTAX_EXPECTED_PATH:             Kind = 229
```

## val SYNTAX_EXPECTED_TOKEN

```mach
pub val SYNTAX_EXPECTED_TOKEN:            Kind = 230
```

## val SYNTAX_EXPECTED_TYPE

```mach
pub val SYNTAX_EXPECTED_TYPE:             Kind = 231
```

## val SYNTAX_NESTING_DEPTH

```mach
pub val SYNTAX_NESTING_DEPTH:             Kind = 232
```

## val TAG_CONSTRUCTOR_FORM

```mach
pub val TAG_CONSTRUCTOR_FORM:             Kind = 233
```

## val TAG_DISCRIMINATOR_NARROW

```mach
pub val TAG_DISCRIMINATOR_NARROW:         Kind = 234
```

## val TAG_DISCRIMINATOR_TYPE

```mach
pub val TAG_DISCRIMINATOR_TYPE:           Kind = 235
```

## val TAG_DUPLICATE_CASE

```mach
pub val TAG_DUPLICATE_CASE:               Kind = 236
```

## val TAG_NO_CASES

```mach
pub val TAG_NO_CASES:                     Kind = 237
```

## val TAG_NO_DISCRIMINATOR

```mach
pub val TAG_NO_DISCRIMINATOR:             Kind = 238
```

## val TAG_NOT_TAG

```mach
pub val TAG_NOT_TAG:                      Kind = 239
```

## val TAG_PAYLOAD_COUNT

```mach
pub val TAG_PAYLOAD_COUNT:                Kind = 240
```

## val TAG_PAYLOAD_MISSING

```mach
pub val TAG_PAYLOAD_MISSING:              Kind = 241
```

## val TAG_PAYLOAD_NAMED

```mach
pub val TAG_PAYLOAD_NAMED:                Kind = 242
```

## val TAG_PAYLOAD_UNEXPECTED

```mach
pub val TAG_PAYLOAD_UNEXPECTED:           Kind = 243
```

## val TAG_PAYLOAD_UNGUARDED

```mach
pub val TAG_PAYLOAD_UNGUARDED:            Kind = 244
```

## val TAG_SELECTOR_VALUE

```mach
pub val TAG_SELECTOR_VALUE:               Kind = 245
```

## val TAG_UNKNOWN_CASE

```mach
pub val TAG_UNKNOWN_CASE:                 Kind = 246
```

## val TARGET_INT_WIDTH

```mach
pub val TARGET_INT_WIDTH:                 Kind = 247
```

## val TARGET_NO_FLOAT

```mach
pub val TARGET_NO_FLOAT:                  Kind = 248
```

## val TARGET_UNSUPPORTED_OP

```mach
pub val TARGET_UNSUPPORTED_OP:            Kind = 249
```

## val TARGET_VECTOR_WIDTH

```mach
pub val TARGET_VECTOR_WIDTH:              Kind = 250
```

## val TEST_DUPLICATE

```mach
pub val TEST_DUPLICATE:                   Kind = 251
```

## val TEST_STATUS_RANGE

```mach
pub val TEST_STATUS_RANGE:                Kind = 252
```

## val TEST_STRING_NAME

```mach
pub val TEST_STRING_NAME:                 Kind = 253
```

## val TESTING_USE

```mach
pub val TESTING_USE:                      Kind = 254
```

## val TYPE_MISMATCH

```mach
pub val TYPE_MISMATCH:                    Kind = 255
```

## val TYPE_RECURSIVE

```mach
pub val TYPE_RECURSIVE:                   Kind = 256
```

## val UNI_LITERAL_FIELDS

```mach
pub val UNI_LITERAL_FIELDS:               Kind = 257
```

## val USE_UNRESOLVED

```mach
pub val USE_UNRESOLVED:                   Kind = 258
```

## val VARIADIC_NO_FIXED_PARAM

```mach
pub val VARIADIC_NO_FIXED_PARAM:          Kind = 259
```

## val VECTOR_INVALID_TYPE

```mach
pub val VECTOR_INVALID_TYPE:              Kind = 260
```

## val VECTOR_LANE_INDEX_DYNAMIC

```mach
pub val VECTOR_LANE_INDEX_DYNAMIC:        Kind = 261
```

## val VECTOR_LITERAL_LENGTH

```mach
pub val VECTOR_LITERAL_LENGTH:            Kind = 262
```

## val VECTOR_LITERAL_NAMED

```mach
pub val VECTOR_LITERAL_NAMED:             Kind = 263
```

## val VECTOR_LITERAL_TYPE

```mach
pub val VECTOR_LITERAL_TYPE:              Kind = 264
```

## val VECTOR_OPERAND_TYPE

```mach
pub val VECTOR_OPERAND_TYPE:              Kind = 265
```

## val VECTOR_OPERATOR_UNSUPPORTED

```mach
pub val VECTOR_OPERATOR_UNSUPPORTED:      Kind = 266
```

## val VECTOR_PACKED_FIELD

```mach
pub val VECTOR_PACKED_FIELD:              Kind = 267
```

## val VECTOR_RANGE_WIDTH

```mach
pub val VECTOR_RANGE_WIDTH:               Kind = 268
```

## val VECTOR_SCALARIZE_REFUSED

```mach
pub val VECTOR_SCALARIZE_REFUSED:         Kind = 269
```

## val VECTOR_SHIFT_COUNT

```mach
pub val VECTOR_SHIFT_COUNT:               Kind = 270
```

## val VECTOR_UNDECLARED_FORM

```mach
pub val VECTOR_UNDECLARED_FORM:           Kind = 271
```

## val VISIBILITY_NOT_EXPORTED

```mach
pub val VISIBILITY_NOT_EXPORTED:          Kind = 272
```

## val ALLOW_DUPLICATE_KEY

```mach
pub val ALLOW_DUPLICATE_KEY:              Kind = 273
```

## val ALLOW_ERROR_KEY

```mach
pub val ALLOW_ERROR_KEY:                  Kind = 274
```

## val ALLOW_UNKNOWN_KEY

```mach
pub val ALLOW_UNKNOWN_KEY:                Kind = 275
```

## val ARTIFACT_BLOCKED

```mach
pub val ARTIFACT_BLOCKED:                 Kind = 276
```

## val ARTIFACT_KIND_UNSUPPORTED

```mach
pub val ARTIFACT_KIND_UNSUPPORTED:        Kind = 277
```

## val ARTIFACT_NO_OUTPUT

```mach
pub val ARTIFACT_NO_OUTPUT:               Kind = 278
```

## val ARTIFACT_NONE_DECLARED

```mach
pub val ARTIFACT_NONE_DECLARED:           Kind = 279
```

## val ARTIFACT_NOT_BUILT

```mach
pub val ARTIFACT_NOT_BUILT:               Kind = 280
```

## val ARTIFACT_NOT_EXECUTABLE

```mach
pub val ARTIFACT_NOT_EXECUTABLE:          Kind = 281
```

## val ARTIFACT_NOT_RUNNABLE

```mach
pub val ARTIFACT_NOT_RUNNABLE:            Kind = 282
```

## val ARTIFACT_REQUIREMENT_UNREACHABLE

```mach
pub val ARTIFACT_REQUIREMENT_UNREACHABLE: Kind = 283
```

## val ARTIFACT_RESOURCE_EMPTY

```mach
pub val ARTIFACT_RESOURCE_EMPTY:          Kind = 284
```

## val ARTIFACT_RESOURCE_UNREADABLE

```mach
pub val ARTIFACT_RESOURCE_UNREADABLE:     Kind = 285
```

## val CATALOG_MALFORMED

```mach
pub val CATALOG_MALFORMED:                Kind = 286
```

## val CATALOG_UNSUPPORTED

```mach
pub val CATALOG_UNSUPPORTED:              Kind = 287
```

## val CLEAN_PATH_INVALID

```mach
pub val CLEAN_PATH_INVALID:               Kind = 288
```

## val CLI_FLAG_CONFLICT

```mach
pub val CLI_FLAG_CONFLICT:                Kind = 289
```

## val CLI_FLAG_INAPPLICABLE

```mach
pub val CLI_FLAG_INAPPLICABLE:            Kind = 290
```

## val CLI_FLAG_REMOVED

```mach
pub val CLI_FLAG_REMOVED:                 Kind = 291
```

## val CLI_FLAG_REPEATED

```mach
pub val CLI_FLAG_REPEATED:                Kind = 292
```

## val CLI_FLAG_UNKNOWN

```mach
pub val CLI_FLAG_UNKNOWN:                 Kind = 293
```

## val CLI_FLAG_VALUE

```mach
pub val CLI_FLAG_VALUE:                   Kind = 294
```

## val CLI_OPERAND

```mach
pub val CLI_OPERAND:                      Kind = 295
```

## val CLI_OUTPUT_PATH

```mach
pub val CLI_OUTPUT_PATH:                  Kind = 296
```

## val CLI_UNKNOWN_ACTION

```mach
pub val CLI_UNKNOWN_ACTION:               Kind = 297
```

## val CLI_UNKNOWN_COMMAND

```mach
pub val CLI_UNKNOWN_COMMAND:              Kind = 298
```

## val DEP_ADD

```mach
pub val DEP_ADD:                          Kind = 299
```

## val DEP_ALREADY_DECLARED

```mach
pub val DEP_ALREADY_DECLARED:             Kind = 300
```

## val DEP_CONFLICT

```mach
pub val DEP_CONFLICT:                     Kind = 301
```

## val DEP_CYCLE

```mach
pub val DEP_CYCLE:                        Kind = 302
```

## val DEP_DESTINATION_EXISTS

```mach
pub val DEP_DESTINATION_EXISTS:           Kind = 303
```

## val DEP_DIRTY

```mach
pub val DEP_DIRTY:                        Kind = 304
```

## val DEP_ENTRY_MISSING

```mach
pub val DEP_ENTRY_MISSING:                Kind = 305
```

## val DEP_FETCH

```mach
pub val DEP_FETCH:                        Kind = 306
```

## val DEP_GITMODULES_MISMATCH

```mach
pub val DEP_GITMODULES_MISMATCH:          Kind = 307
```

## val DEP_ID_MISMATCH

```mach
pub val DEP_ID_MISMATCH:                  Kind = 308
```

## val DEP_INDEX_CONFLICT

```mach
pub val DEP_INDEX_CONFLICT:               Kind = 309
```

## val DEP_INVALID_PATH

```mach
pub val DEP_INVALID_PATH:                 Kind = 310
```

## val DEP_LIMIT

```mach
pub val DEP_LIMIT:                        Kind = 311
```

## val DEP_MANUAL_EDIT

```mach
pub val DEP_MANUAL_EDIT:                  Kind = 312
```

## val DEP_NESTED

```mach
pub val DEP_NESTED:                       Kind = 313
```

## val DEP_NO_PROJECT_ID

```mach
pub val DEP_NO_PROJECT_ID:                Kind = 314
```

## val DEP_NO_RELEASE

```mach
pub val DEP_NO_RELEASE:                   Kind = 315
```

## val DEP_NO_REPOSITORY

```mach
pub val DEP_NO_REPOSITORY:                Kind = 316
```

## val DEP_NO_SOURCE

```mach
pub val DEP_NO_SOURCE:                    Kind = 317
```

## val DEP_NOT_DECLARED

```mach
pub val DEP_NOT_DECLARED:                 Kind = 318
```

## val DEP_NOT_DIRECTORY

```mach
pub val DEP_NOT_DIRECTORY:                Kind = 319
```

## val DEP_NOT_GITLINK

```mach
pub val DEP_NOT_GITLINK:                  Kind = 320
```

## val DEP_NOT_IN_CLOSURE

```mach
pub val DEP_NOT_IN_CLOSURE:               Kind = 321
```

## val DEP_NOT_REPOSITORY_ROOT

```mach
pub val DEP_NOT_REPOSITORY_ROOT:          Kind = 322
```

## val DEP_OFFLINE

```mach
pub val DEP_OFFLINE:                      Kind = 323
```

## val DEP_PATH_EMPTY

```mach
pub val DEP_PATH_EMPTY:                   Kind = 324
```

## val DEP_PATH_ESCAPE

```mach
pub val DEP_PATH_ESCAPE:                  Kind = 325
```

## val DEP_PATH_SAME

```mach
pub val DEP_PATH_SAME:                    Kind = 326
```

## val DEP_REF_INVALID

```mach
pub val DEP_REF_INVALID:                  Kind = 327
```

## val DEP_REF_MISMATCH

```mach
pub val DEP_REF_MISMATCH:                 Kind = 328
```

## val DEP_RELEASE_EXCLUDED

```mach
pub val DEP_RELEASE_EXCLUDED:             Kind = 329
```

## val DEP_RELEASE_SELECTOR

```mach
pub val DEP_RELEASE_SELECTOR:             Kind = 330
```

## val DEP_REPOSITORY_METADATA

```mach
pub val DEP_REPOSITORY_METADATA:          Kind = 331
```

## val DEP_SHADOWS_PROJECT

```mach
pub val DEP_SHADOWS_PROJECT:              Kind = 332
```

## val DEP_SLOT_OCCUPIED

```mach
pub val DEP_SLOT_OCCUPIED:                Kind = 333
```

## val DEP_SOURCE_COUNT

```mach
pub val DEP_SOURCE_COUNT:                 Kind = 334
```

## val DEP_STILL_REQUIRED

```mach
pub val DEP_STILL_REQUIRED:               Kind = 335
```

## val DEP_SYMLINK

```mach
pub val DEP_SYMLINK:                      Kind = 336
```

## val DEP_TREE_MISMATCH

```mach
pub val DEP_TREE_MISMATCH:                Kind = 337
```

## val DEP_UNPINNED

```mach
pub val DEP_UNPINNED:                     Kind = 338
```

## val DEP_UNREALIZED

```mach
pub val DEP_UNREALIZED:                   Kind = 339
```

## val DEP_UNREPRODUCIBLE

```mach
pub val DEP_UNREPRODUCIBLE:               Kind = 340
```

## val DEP_UNSATISFIABLE

```mach
pub val DEP_UNSATISFIABLE:                Kind = 341
```

## val DEP_UNSTAGED

```mach
pub val DEP_UNSTAGED:                     Kind = 342
```

## val DEP_VERSION_UNMET

```mach
pub val DEP_VERSION_UNMET:                Kind = 343
```

## val EDITOR_NO_BUFFER

```mach
pub val EDITOR_NO_BUFFER:                 Kind = 344
```

## val EDITOR_TARGET_SELECTION

```mach
pub val EDITOR_TARGET_SELECTION:          Kind = 345
```

## val ENV_NAME_COMPARE

```mach
pub val ENV_NAME_COMPARE:                 Kind = 346
```

## val ENV_READ

```mach
pub val ENV_READ:                         Kind = 347
```

## val EXTENSION_INVALID_NAME

```mach
pub val EXTENSION_INVALID_NAME:           Kind = 348
```

## val EXTENSION_TOO_MANY

```mach
pub val EXTENSION_TOO_MANY:               Kind = 349
```

## val FS_CLOSE

```mach
pub val FS_CLOSE:                         Kind = 350
```

## val FS_CREATE

```mach
pub val FS_CREATE:                        Kind = 351
```

## val FS_CURRENT_DIR

```mach
pub val FS_CURRENT_DIR:                   Kind = 352
```

## val FS_IO

```mach
pub val FS_IO:                            Kind = 353
```

## val FS_OPEN

```mach
pub val FS_OPEN:                          Kind = 354
```

## val FS_READ

```mach
pub val FS_READ:                          Kind = 355
```

## val FS_REMOVE

```mach
pub val FS_REMOVE:                        Kind = 356
```

## val FS_STAT

```mach
pub val FS_STAT:                          Kind = 357
```

## val FS_TEMP

```mach
pub val FS_TEMP:                          Kind = 358
```

## val FS_WRITE

```mach
pub val FS_WRITE:                         Kind = 359
```

## val GIT_COMMAND

```mach
pub val GIT_COMMAND:                      Kind = 360
```

## val GIT_MISSING

```mach
pub val GIT_MISSING:                      Kind = 361
```

## val GIT_OUTPUT_MALFORMED

```mach
pub val GIT_OUTPUT_MALFORMED:             Kind = 362
```

## val GLOB_INACCESSIBLE

```mach
pub val GLOB_INACCESSIBLE:                Kind = 363
```

## val GLOB_NO_MATCH

```mach
pub val GLOB_NO_MATCH:                    Kind = 364
```

## val GLOB_SHAPE

```mach
pub val GLOB_SHAPE:                       Kind = 365
```

## val GLOB_SPECIAL_ENTRY

```mach
pub val GLOB_SPECIAL_ENTRY:               Kind = 366
```

## val GLOB_SYMLINK

```mach
pub val GLOB_SYMLINK:                     Kind = 367
```

## val LINK_IMPORT_LIBRARY

```mach
pub val LINK_IMPORT_LIBRARY:              Kind = 368
```

## val LINK_INPUT_FORMAT

```mach
pub val LINK_INPUT_FORMAT:                Kind = 369
```

## val LINK_INPUT_INVALID

```mach
pub val LINK_INPUT_INVALID:               Kind = 370
```

## val LINK_INPUT_NOT_FOUND

```mach
pub val LINK_INPUT_NOT_FOUND:             Kind = 371
```

## val LINK_INPUT_UNREADABLE

```mach
pub val LINK_INPUT_UNREADABLE:            Kind = 372
```

## val LINK_LOADER_NAME_CONFLICT

```mach
pub val LINK_LOADER_NAME_CONFLICT:        Kind = 373
```

## val LINK_LOCAL_MISSING

```mach
pub val LINK_LOCAL_MISSING:               Kind = 374
```

## val LINK_RPATH_MISMATCH

```mach
pub val LINK_RPATH_MISMATCH:              Kind = 375
```

## val LINK_RPATH_UNRESOLVED

```mach
pub val LINK_RPATH_UNRESOLVED:            Kind = 376
```

## val LINK_STATIC_CLAIMS_SYMBOLS

```mach
pub val LINK_STATIC_CLAIMS_SYMBOLS:       Kind = 377
```

## val LINK_SYMBOL_CLAIM_CONFLICT

```mach
pub val LINK_SYMBOL_CLAIM_CONFLICT:       Kind = 378
```

## val LINK_UNKNOWN_TABLE

```mach
pub val LINK_UNKNOWN_TABLE:               Kind = 379
```

## val MACH_RANGE_MISSING

```mach
pub val MACH_RANGE_MISSING:               Kind = 380
```

## val MACH_VERSION_UNACCEPTED

```mach
pub val MACH_VERSION_UNACCEPTED:          Kind = 381
```

## val MANIFEST_CHANGED

```mach
pub val MANIFEST_CHANGED:                 Kind = 382
```

## val MANIFEST_DUPLICATE_ENTRY

```mach
pub val MANIFEST_DUPLICATE_ENTRY:         Kind = 383
```

## val MANIFEST_INVALID_NAME

```mach
pub val MANIFEST_INVALID_NAME:            Kind = 384
```

## val MANIFEST_INVALID_VALUE

```mach
pub val MANIFEST_INVALID_VALUE:           Kind = 385
```

## val MANIFEST_KEY_CONFLICT

```mach
pub val MANIFEST_KEY_CONFLICT:            Kind = 386
```

## val MANIFEST_MISSING_REQUIRED

```mach
pub val MANIFEST_MISSING_REQUIRED:        Kind = 387
```

## val MANIFEST_NUL_BYTE

```mach
pub val MANIFEST_NUL_BYTE:                Kind = 388
```

## val MANIFEST_PATH

```mach
pub val MANIFEST_PATH:                    Kind = 389
```

## val MANIFEST_REMOVED_KEY

```mach
pub val MANIFEST_REMOVED_KEY:             Kind = 390
```

## val MANIFEST_TOO_MANY_ENTRIES

```mach
pub val MANIFEST_TOO_MANY_ENTRIES:        Kind = 391
```

## val MANIFEST_UNKNOWN_KEY

```mach
pub val MANIFEST_UNKNOWN_KEY:             Kind = 392
```

## val MANIFEST_UNREADABLE

```mach
pub val MANIFEST_UNREADABLE:              Kind = 393
```

## val MANIFEST_VALUE_TYPE

```mach
pub val MANIFEST_VALUE_TYPE:              Kind = 394
```

## val NEED_CYCLE

```mach
pub val NEED_CYCLE:                       Kind = 395
```

## val NEED_FORM

```mach
pub val NEED_FORM:                        Kind = 396
```

## val NEED_NO_MATCH

```mach
pub val NEED_NO_MATCH:                    Kind = 397
```

## val NEED_SELF

```mach
pub val NEED_SELF:                        Kind = 398
```

## val OUTPUT_AMBIGUOUS

```mach
pub val OUTPUT_AMBIGUOUS:                 Kind = 399
```

## val OUTPUT_COLLISION

```mach
pub val OUTPUT_COLLISION:                 Kind = 400
```

## val OUTPUT_PATH_INVALID

```mach
pub val OUTPUT_PATH_INVALID:              Kind = 401
```

## val OUTPUT_WRITE

```mach
pub val OUTPUT_WRITE:                     Kind = 402
```

## val PATH_CREATE

```mach
pub val PATH_CREATE:                      Kind = 403
```

## val PATH_ENTRY_TYPE

```mach
pub val PATH_ENTRY_TYPE:                  Kind = 404
```

## val PATH_INACCESSIBLE

```mach
pub val PATH_INACCESSIBLE:                Kind = 405
```

## val PATH_NOT_CANONICAL

```mach
pub val PATH_NOT_CANONICAL:               Kind = 406
```

## val PATH_SYMLINK

```mach
pub val PATH_SYMLINK:                     Kind = 407
```

## val PROCESS_NAME_EMPTY

```mach
pub val PROCESS_NAME_EMPTY:               Kind = 408
```

## val PROCESS_NO_STATUS

```mach
pub val PROCESS_NO_STATUS:                Kind = 409
```

## val PROCESS_NOT_EXECUTABLE

```mach
pub val PROCESS_NOT_EXECUTABLE:           Kind = 410
```

## val PROCESS_NOT_FOUND

```mach
pub val PROCESS_NOT_FOUND:                Kind = 411
```

## val PROCESS_PATH_ENV

```mach
pub val PROCESS_PATH_ENV:                 Kind = 412
```

## val PROCESS_SPAWN

```mach
pub val PROCESS_SPAWN:                    Kind = 413
```

## val PROCESS_TIMEOUT

```mach
pub val PROCESS_TIMEOUT:                  Kind = 414
```

## val PROCESS_WAIT

```mach
pub val PROCESS_WAIT:                     Kind = 415
```

## val PROJECT_EMPTY

```mach
pub val PROJECT_EMPTY:                    Kind = 416
```

## val PROJECT_FILE_EXISTS

```mach
pub val PROJECT_FILE_EXISTS:              Kind = 417
```

## val PROJECT_FILE_KIND

```mach
pub val PROJECT_FILE_KIND:                Kind = 418
```

## val PROJECT_INVALID_ID

```mach
pub val PROJECT_INVALID_ID:               Kind = 419
```

## val PROJECT_MANIFEST_NAME

```mach
pub val PROJECT_MANIFEST_NAME:            Kind = 420
```

## val PROJECT_NO_MANIFEST

```mach
pub val PROJECT_NO_MANIFEST:              Kind = 421
```

## val PROJECT_PATH_INVALID

```mach
pub val PROJECT_PATH_INVALID:             Kind = 422
```

## val SELECTION_AMBIGUOUS

```mach
pub val SELECTION_AMBIGUOUS:              Kind = 423
```

## val SELECTION_DUPLICATE_DEFAULT

```mach
pub val SELECTION_DUPLICATE_DEFAULT:      Kind = 424
```

## val SELECTION_UNKNOWN

```mach
pub val SELECTION_UNKNOWN:                Kind = 425
```

## val SELECTION_UNSUPPORTED_TARGET

```mach
pub val SELECTION_UNSUPPORTED_TARGET:     Kind = 426
```

## val SOURCE_DEPENDENCY_TREE

```mach
pub val SOURCE_DEPENDENCY_TREE:           Kind = 427
```

## val SOURCE_LOAD

```mach
pub val SOURCE_LOAD:                      Kind = 428
```

## val SOURCE_NESTING_DEPTH

```mach
pub val SOURCE_NESTING_DEPTH:             Kind = 429
```

## val SOURCE_NOT_REGULAR

```mach
pub val SOURCE_NOT_REGULAR:               Kind = 430
```

## val SOURCE_NUL_BYTE

```mach
pub val SOURCE_NUL_BYTE:                  Kind = 431
```

## val STEP_ARGV_EMPTY

```mach
pub val STEP_ARGV_EMPTY:                  Kind = 432
```

## val STEP_CACHE_RECORD

```mach
pub val STEP_CACHE_RECORD:                Kind = 433
```

## val STEP_ENVIRONMENT

```mach
pub val STEP_ENVIRONMENT:                 Kind = 434
```

## val STEP_EXECUTABLE_NOT_FOUND

```mach
pub val STEP_EXECUTABLE_NOT_FOUND:        Kind = 435
```

## val STEP_EXIT

```mach
pub val STEP_EXIT:                        Kind = 436
```

## val STEP_OUTPUT_COLLISION

```mach
pub val STEP_OUTPUT_COLLISION:            Kind = 437
```

## val STEP_OUTPUT_DUPLICATE

```mach
pub val STEP_OUTPUT_DUPLICATE:            Kind = 438
```

## val STEP_OUTPUT_MISSING

```mach
pub val STEP_OUTPUT_MISSING:              Kind = 439
```

## val STEP_OUTPUT_RESERVED

```mach
pub val STEP_OUTPUT_RESERVED:             Kind = 440
```

## val STEP_OUTPUT_UNDECLARED

```mach
pub val STEP_OUTPUT_UNDECLARED:           Kind = 441
```

## val STEP_OUTPUT_UNVERIFIABLE

```mach
pub val STEP_OUTPUT_UNVERIFIABLE:         Kind = 442
```

## val STEP_PATH_OUTSIDE

```mach
pub val STEP_PATH_OUTSIDE:                Kind = 443
```

## val STEP_PUBLISH

```mach
pub val STEP_PUBLISH:                     Kind = 444
```

## val STEP_SIGNAL

```mach
pub val STEP_SIGNAL:                      Kind = 445
```

## val STEP_SNAPSHOT

```mach
pub val STEP_SNAPSHOT:                    Kind = 446
```

## val STEP_SPAWN

```mach
pub val STEP_SPAWN:                       Kind = 447
```

## val STEP_STAGING

```mach
pub val STEP_STAGING:                     Kind = 448
```

## val STEP_TIMEOUT

```mach
pub val STEP_TIMEOUT:                     Kind = 449
```

## val STEP_TOO_MANY_PATHS

```mach
pub val STEP_TOO_MANY_PATHS:              Kind = 450
```

## val STEP_WAIT

```mach
pub val STEP_WAIT:                        Kind = 451
```

## val TARGET_INVALID

```mach
pub val TARGET_INVALID:                   Kind = 452
```

## val TARGET_NO_ARTIFACTS

```mach
pub val TARGET_NO_ARTIFACTS:              Kind = 453
```

## val TARGET_NONE_DECLARED

```mach
pub val TARGET_NONE_DECLARED:             Kind = 454
```

## val TARGET_NOT_RUNNABLE

```mach
pub val TARGET_NOT_RUNNABLE:              Kind = 455
```

## val TARGET_OUTPUT_UNSUPPORTED

```mach
pub val TARGET_OUTPUT_UNSUPPORTED:        Kind = 456
```

## val TARGET_RESERVED_NAME

```mach
pub val TARGET_RESERVED_NAME:             Kind = 457
```

## val TARGET_STACK_SIZE

```mach
pub val TARGET_STACK_SIZE:                Kind = 458
```

## val TARGET_UNSUPPORTED

```mach
pub val TARGET_UNSUPPORTED:               Kind = 459
```

## val TEMPLATE_AMBIGUOUS_ARTIFACT

```mach
pub val TEMPLATE_AMBIGUOUS_ARTIFACT:      Kind = 460
```

## val TEMPLATE_ARTIFACT_NOT_REQUIRED

```mach
pub val TEMPLATE_ARTIFACT_NOT_REQUIRED:   Kind = 461
```

## val TEMPLATE_TOO_LARGE

```mach
pub val TEMPLATE_TOO_LARGE:               Kind = 462
```

## val TEMPLATE_UNAVAILABLE

```mach
pub val TEMPLATE_UNAVAILABLE:             Kind = 463
```

## val TEMPLATE_UNKNOWN_VARIABLE

```mach
pub val TEMPLATE_UNKNOWN_VARIABLE:        Kind = 464
```

## val TEMPLATE_UNTERMINATED

```mach
pub val TEMPLATE_UNTERMINATED:            Kind = 465
```

## val TEST_NONE_DECLARED

```mach
pub val TEST_NONE_DECLARED:               Kind = 466
```

## val TOML_MALFORMED

```mach
pub val TOML_MALFORMED:                   Kind = 467
```

## val TOML_VALUE_TOO_LONG

```mach
pub val TOML_VALUE_TOO_LONG:              Kind = 468
```

## val USE_NO_PUBLIC_MODULE

```mach
pub val USE_NO_PUBLIC_MODULE:             Kind = 469
```

## val VERSION_INVALID

```mach
pub val VERSION_INVALID:                  Kind = 470
```

## val VERSION_INVALID_RANGE

```mach
pub val VERSION_INVALID_RANGE:            Kind = 471
```

## val LINK_ABI_MISMATCH

```mach
pub val LINK_ABI_MISMATCH:                Kind = 472
```

## val LINK_DUPLICATE_SYMBOL

```mach
pub val LINK_DUPLICATE_SYMBOL:            Kind = 473
```

## val LINK_EMPTY_IMAGE

```mach
pub val LINK_EMPTY_IMAGE:                 Kind = 474
```

## val LINK_ENTRY_MISSING

```mach
pub val LINK_ENTRY_MISSING:               Kind = 475
```

## val LINK_ENTRY_PLACEMENT

```mach
pub val LINK_ENTRY_PLACEMENT:             Kind = 476
```

## val LINK_IMPORT_ALIAS

```mach
pub val LINK_IMPORT_ALIAS:                Kind = 477
```

## val LINK_IMPORT_CONFLICT

```mach
pub val LINK_IMPORT_CONFLICT:             Kind = 478
```

## val LINK_IMPORT_UNATTRIBUTED

```mach
pub val LINK_IMPORT_UNATTRIBUTED:         Kind = 479
```

## val LINK_NO_EXPORTS

```mach
pub val LINK_NO_EXPORTS:                  Kind = 480
```

## val LINK_NO_LOADER

```mach
pub val LINK_NO_LOADER:                   Kind = 481
```

## val LINK_RELOCATION_OVERFLOW

```mach
pub val LINK_RELOCATION_OVERFLOW:         Kind = 482
```

## val LINK_RELOCATION_UNSUPPORTED

```mach
pub val LINK_RELOCATION_UNSUPPORTED:      Kind = 483
```

## val LINK_SECTION_UNSUPPORTED

```mach
pub val LINK_SECTION_UNSUPPORTED:         Kind = 484
```

## val LINK_SIZE_LIMIT

```mach
pub val LINK_SIZE_LIMIT:                  Kind = 485
```

## val LINK_UNDEFINED_SYMBOL

```mach
pub val LINK_UNDEFINED_SYMBOL:            Kind = 486
```

## val LINK_WEAK_FALLBACK

```mach
pub val LINK_WEAK_FALLBACK:               Kind = 487
```

## val OBJECT_FORMAT_MISMATCH

```mach
pub val OBJECT_FORMAT_MISMATCH:           Kind = 488
```

## val OBJECT_MALFORMED

```mach
pub val OBJECT_MALFORMED:                 Kind = 489
```

## val OBJECT_UNSUPPORTED

```mach
pub val OBJECT_UNSUPPORTED:               Kind = 490
```

## val RESOURCE_ICON_MALFORMED

```mach
pub val RESOURCE_ICON_MALFORMED:          Kind = 491
```

## val RESOURCE_METADATA_INVALID

```mach
pub val RESOURCE_METADATA_INVALID:        Kind = 492
```

## val RESOURCE_SIZE_LIMIT

```mach
pub val RESOURCE_SIZE_LIMIT:              Kind = 493
```

## val TARGET_FLOAT_WIDTH

```mach
pub val TARGET_FLOAT_WIDTH:               Kind = 494
```

## val NAME_BUILTIN_TYPE

```mach
pub val NAME_BUILTIN_TYPE:                Kind = 495
```

## val DECORATOR_NOT_CONSTANT

```mach
pub val DECORATOR_NOT_CONSTANT:           Kind = 496
```

## val SELECTION_NO_HOST_TARGET

```mach
pub val SELECTION_NO_HOST_TARGET:         Kind = 497
```

## val TARGET_DEFAULT_DEPRECATED

```mach
pub val TARGET_DEFAULT_DEPRECATED:        Kind = 498
```

## val READ_WRITEONLY_STORAGE

```mach
pub val READ_WRITEONLY_STORAGE:           Kind = 499
```

## val SPIRV_WRITEONLY_LOAD

```mach
pub val SPIRV_WRITEONLY_LOAD:             Kind = 500
```

## val OP_SIGNATURE

```mach
pub val OP_SIGNATURE:                     Kind = 501
```

## val OP_OPERAND_NOT_CONSTANT

```mach
pub val OP_OPERAND_NOT_CONSTANT:          Kind = 502
```

## rec Spec

```mach
pub rec Spec;
```

kind:    the row's own kind, which is its position in the table plus one
key:     the dotted key the kind is printed and selected by
level:   the severity every diagnostic of the kind is raised at
source:  the kind is decided by the source alone, whatever the target, goal
         or profile, so an `#[expect]` of it that nothing fulfils is reported
retired: nothing raises the kind any more; the row stays so its key is
         never given to another kind

## val COUNT

```mach
pub val COUNT: usize       = 502
```

## fun at

```mach
pub fun at(i: usize) *Spec;
```

the row at `i` in declaration order, nil past the end

## fun spec

```mach
pub fun spec(k: Kind) *Spec;
```

the row of `k`, nil for NONE or a kind no row declares

## fun live

```mach
pub fun live(k: Kind) bool;
```

the kind can be raised: a row declares it and it is not retired

## fun key_of

```mach
pub fun key_of(k: Kind) str;
```

the key of `k`, nil for NONE or a kind no row declares

## fun named

```mach
pub fun named(key: str) opt[Kind];
```

the kind whose key is exactly `key`, retired or not

## val SET_WORDS

```mach
pub val SET_WORDS: usize = COUNT / 64 + 1
```

a set of kinds, one bit for every kind the table declares

## rec KindSet

```mach
pub rec KindSet;
```

## fun set_empty

```mach
pub fun set_empty() KindSet;
```

## fun set_add

```mach
pub fun set_add(s: *KindSet, k: Kind);
```

a kind no row declares is never a member

## fun set_has

```mach
pub fun set_has(s: *KindSet, k: Kind) bool;
```

## fun set_union

```mach
pub fun set_union(s: *KindSet, other: *KindSet);
```

## fun set_is_empty

```mach
pub fun set_is_empty(s: *KindSet) bool;
```

## rec Selection

```mach
pub rec Selection;
```

what one key or family selects from the table

warnings: the warning rows it covers
errors:   how many error rows it covers
source:   a covered warning row is decided by the source alone

## fun select

```mach
pub fun select(selector: str) Selection;
```

the rows `selector` covers; nothing at all when it is unknown

## fun selection_unknown

```mach
pub fun selection_unknown(s: *Selection) bool;
```

the selection covers no row

## fun selection_empty

```mach
pub fun selection_empty(s: *Selection) bool;
```

the selection covers no warning row

