# mach.lang.diagnostic.kind

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
pub val IMPORT_UNUSED:                  Kind = 1
```

## val DECL_DEPRECATED

```mach
pub val DECL_DEPRECATED:                Kind = 2
```

## val DOC_LINT

```mach
pub val DOC_LINT:                       Kind = 3
```

## val FWD_INSTANCES

```mach
pub val FWD_INSTANCES:                  Kind = 4
```

## val DEBUG_DROPPED

```mach
pub val DEBUG_DROPPED:                  Kind = 5
```

## val TARGET_SKIPPED

```mach
pub val TARGET_SKIPPED:                 Kind = 6
```

## val TARGET_NATIVE_FALLBACK

```mach
pub val TARGET_NATIVE_FALLBACK:         Kind = 7
```

## val SECRET_NOT_OBLIVIOUS

```mach
pub val SECRET_NOT_OBLIVIOUS:           Kind = 8
```

## val FLOAT_INEXACT

```mach
pub val FLOAT_INEXACT:                  Kind = 9
```

## val VECTOR_SCALARIZE

```mach
pub val VECTOR_SCALARIZE:               Kind = 10
```

## val EXPECT_UNFULFILLED

```mach
pub val EXPECT_UNFULFILLED:             Kind = 11
```

## val ABI_TYPE_HAS_BODY

```mach
pub val ABI_TYPE_HAS_BODY:              Kind = 12
```

## val ABI_TYPE_OPAQUE

```mach
pub val ABI_TYPE_OPAQUE:                Kind = 13
```

## val ABI_TYPE_UNAVAILABLE

```mach
pub val ABI_TYPE_UNAVAILABLE:           Kind = 14
```

## val ABI_TYPE_UNKNOWN

```mach
pub val ABI_TYPE_UNKNOWN:               Kind = 15
```

## val ADDRESS_COMPTIME_PARAM

```mach
pub val ADDRESS_COMPTIME_PARAM:         Kind = 16
```

## val ADDRESS_GENERIC_FUNCTION

```mach
pub val ADDRESS_GENERIC_FUNCTION:       Kind = 17
```

## val ADDRESS_PACKED_MEMBER

```mach
pub val ADDRESS_PACKED_MEMBER:          Kind = 18
```

## val ADDRESS_TEMPORARY

```mach
pub val ADDRESS_TEMPORARY:              Kind = 19
```

## val ALLOCA_VARIABLE_SIZE

```mach
pub val ALLOCA_VARIABLE_SIZE:           Kind = 20
```

## val ARRAY_INFERRED_LENGTH

```mach
pub val ARRAY_INFERRED_LENGTH:          Kind = 21
```

## val ARRAY_LENGTH_NOT_CONST

```mach
pub val ARRAY_LENGTH_NOT_CONST:         Kind = 22
```

## val ARRAY_LENGTH_VALUE

```mach
pub val ARRAY_LENGTH_VALUE:             Kind = 23
```

## val ARRAY_LITERAL_LENGTH

```mach
pub val ARRAY_LITERAL_LENGTH:           Kind = 24
```

## val ASM_DIRECTIVE

```mach
pub val ASM_DIRECTIVE:                  Kind = 25
```

## val ASM_EFFECTS

```mach
pub val ASM_EFFECTS:                    Kind = 26
```

## val ASM_EXTENSION

```mach
pub val ASM_EXTENSION:                  Kind = 27
```

## val ASM_IMMEDIATE_RANGE

```mach
pub val ASM_IMMEDIATE_RANGE:            Kind = 28
```

## val ASM_INSTRUCTION_UNAVAILABLE

```mach
pub val ASM_INSTRUCTION_UNAVAILABLE:    Kind = 29
```

## val ASM_ISA_MISSING

```mach
pub val ASM_ISA_MISSING:                Kind = 30
```

## val ASM_LABEL

```mach
pub val ASM_LABEL:                      Kind = 31
```

## val ASM_LOCAL

```mach
pub val ASM_LOCAL:                      Kind = 32
```

## val ASM_OPERAND

```mach
pub val ASM_OPERAND:                    Kind = 33
```

## val ASM_SYNTAX

```mach
pub val ASM_SYNTAX:                     Kind = 34
```

## val ASM_TARGET_MISMATCH

```mach
pub val ASM_TARGET_MISMATCH:            Kind = 35
```

## val ASM_UNAVAILABLE

```mach
pub val ASM_UNAVAILABLE:                Kind = 36
```

## val ASM_UNKNOWN_INSTRUCTION

```mach
pub val ASM_UNKNOWN_INSTRUCTION:        Kind = 37
```

## val ASM_UNSUPPORTED

```mach
pub val ASM_UNSUPPORTED:                Kind = 38
```

## val ASM_UNTERMINATED

```mach
pub val ASM_UNTERMINATED:               Kind = 39
```

## val ASSIGN_IMMUTABLE

```mach
pub val ASSIGN_IMMUTABLE:               Kind = 40
```

## val ASSIGN_NOT_PLACE

```mach
pub val ASSIGN_NOT_PLACE:               Kind = 41
```

## val ASSIGN_READONLY_STORAGE

```mach
pub val ASSIGN_READONLY_STORAGE:        Kind = 42
```

## val BINDING_UNTYPED

```mach
pub val BINDING_UNTYPED:                Kind = 43
```

## val BINDING_VAL_UNINITIALIZED

```mach
pub val BINDING_VAL_UNINITIALIZED:      Kind = 44
```

## val CALL_ARITY

```mach
pub val CALL_ARITY:                     Kind = 45
```

## val CALL_C_VARIADIC_PROMOTION

```mach
pub val CALL_C_VARIADIC_PROMOTION:      Kind = 46
```

## val CALL_NOT_CALLABLE

```mach
pub val CALL_NOT_CALLABLE:              Kind = 47
```

## val CALL_SPREAD

```mach
pub val CALL_SPREAD:                    Kind = 48
```

## val CAST_INVALID

```mach
pub val CAST_INVALID:                   Kind = 49
```

## val CAST_REINTERPRET_SIZE

```mach
pub val CAST_REINTERPRET_SIZE:          Kind = 50
```

## val CAST_TAG_REPRESENTATION

```mach
pub val CAST_TAG_REPRESENTATION:        Kind = 51
```

## val CAST_VECTOR_ELEMENT

```mach
pub val CAST_VECTOR_ELEMENT:            Kind = 52
```

## val CAST_VECTOR_SHAPE

```mach
pub val CAST_VECTOR_SHAPE:              Kind = 53
```

## val COMPILER_INTERNAL

```mach
pub val COMPILER_INTERNAL:              Kind = 54
```

## val COMPTIME_ARGUMENT_NOT_CONSTANT

```mach
pub val COMPTIME_ARGUMENT_NOT_CONSTANT: Kind = 55
```

## val COMPTIME_CASE_DESCRIPTOR

```mach
pub val COMPTIME_CASE_DESCRIPTOR:       Kind = 56
```

## val COMPTIME_CASE_NO_PAYLOAD

```mach
pub val COMPTIME_CASE_NO_PAYLOAD:       Kind = 57
```

## val COMPTIME_CASE_PAYLOAD

```mach
pub val COMPTIME_CASE_PAYLOAD:          Kind = 58
```

## val COMPTIME_CASES_NOT_TAG

```mach
pub val COMPTIME_CASES_NOT_TAG:         Kind = 59
```

## val COMPTIME_CAST

```mach
pub val COMPTIME_CAST:                  Kind = 60
```

## val COMPTIME_CAST_UNTYPED

```mach
pub val COMPTIME_CAST_UNTYPED:          Kind = 61
```

## val COMPTIME_CT_MUL

```mach
pub val COMPTIME_CT_MUL:                Kind = 62
```

## val COMPTIME_DESCRIPTOR_MEMBER

```mach
pub val COMPTIME_DESCRIPTOR_MEMBER:     Kind = 63
```

## val COMPTIME_DISCRIMINANT_OF

```mach
pub val COMPTIME_DISCRIMINANT_OF:       Kind = 64
```

## val COMPTIME_DIVISION_BY_ZERO

```mach
pub val COMPTIME_DIVISION_BY_ZERO:      Kind = 65
```

## val COMPTIME_ERROR_DIRECTIVE

```mach
pub val COMPTIME_ERROR_DIRECTIVE:       Kind = 66
```

## val COMPTIME_EXTENSION_UNKNOWN

```mach
pub val COMPTIME_EXTENSION_UNKNOWN:     Kind = 67
```

## val COMPTIME_FIELD_DESCRIPTOR

```mach
pub val COMPTIME_FIELD_DESCRIPTOR:      Kind = 68
```

## val COMPTIME_FIELD_TYPE_BINDING

```mach
pub val COMPTIME_FIELD_TYPE_BINDING:    Kind = 69
```

## val COMPTIME_FIELDS_NOT_RECORD

```mach
pub val COMPTIME_FIELDS_NOT_RECORD:     Kind = 70
```

## val COMPTIME_FLOAT_WIDTH_MISMATCH

```mach
pub val COMPTIME_FLOAT_WIDTH_MISMATCH:  Kind = 71
```

## val COMPTIME_GATE_ONLY

```mach
pub val COMPTIME_GATE_ONLY:             Kind = 72
```

## val COMPTIME_INTRINSIC_ARITY

```mach
pub val COMPTIME_INTRINSIC_ARITY:       Kind = 73
```

## val COMPTIME_LENGTH_OF

```mach
pub val COMPTIME_LENGTH_OF:             Kind = 74
```

## val COMPTIME_NEEDS_INSTANCE

```mach
pub val COMPTIME_NEEDS_INSTANCE:        Kind = 75
```

## val COMPTIME_NOT_CONSTANT

```mach
pub val COMPTIME_NOT_CONSTANT:          Kind = 76
```

## val COMPTIME_NOT_EVALUABLE

```mach
pub val COMPTIME_NOT_EVALUABLE:         Kind = 77
```

## val COMPTIME_NOT_INTEGER

```mach
pub val COMPTIME_NOT_INTEGER:           Kind = 78
```

## val COMPTIME_OFFSET_OF

```mach
pub val COMPTIME_OFFSET_OF:             Kind = 79
```

## val COMPTIME_OPERAND_TYPE

```mach
pub val COMPTIME_OPERAND_TYPE:          Kind = 80
```

## val COMPTIME_OVERFLOW

```mach
pub val COMPTIME_OVERFLOW:              Kind = 81
```

## val COMPTIME_PARAM_FN_VALUE

```mach
pub val COMPTIME_PARAM_FN_VALUE:        Kind = 82
```

## val COMPTIME_PATH_RESERVED

```mach
pub val COMPTIME_PATH_RESERVED:         Kind = 83
```

## val COMPTIME_PATH_ROOT

```mach
pub val COMPTIME_PATH_ROOT:             Kind = 84
```

## val COMPTIME_PATH_UNAVAILABLE

```mach
pub val COMPTIME_PATH_UNAVAILABLE:      Kind = 85
```

## val COMPTIME_PATH_UNEVALUABLE

```mach
pub val COMPTIME_PATH_UNEVALUABLE:      Kind = 86
```

## val COMPTIME_PATH_UNKNOWN

```mach
pub val COMPTIME_PATH_UNKNOWN:          Kind = 87
```

## val COMPTIME_POINTEE_OF

```mach
pub val COMPTIME_POINTEE_OF:            Kind = 88
```

## val COMPTIME_SHIFT_COUNT

```mach
pub val COMPTIME_SHIFT_COUNT:           Kind = 89
```

## val COMPTIME_TOO_EARLY

```mach
pub val COMPTIME_TOO_EARLY:             Kind = 90
```

## val COMPTIME_TYPE_ARGUMENT

```mach
pub val COMPTIME_TYPE_ARGUMENT:         Kind = 91
```

## val COMPTIME_TYPE_CONSTRUCTOR

```mach
pub val COMPTIME_TYPE_CONSTRUCTOR:      Kind = 92
```

## val COMPTIME_TYPE_ID

```mach
pub val COMPTIME_TYPE_ID:               Kind = 93
```

## val COMPTIME_TYPE_NOT_VALUE

```mach
pub val COMPTIME_TYPE_NOT_VALUE:        Kind = 94
```

## val COMPTIME_TYPE_OPERAND

```mach
pub val COMPTIME_TYPE_OPERAND:          Kind = 95
```

## val COMPTIME_TYPE_UNRESOLVED

```mach
pub val COMPTIME_TYPE_UNRESOLVED:       Kind = 96
```

## val COMPTIME_UNMEASURABLE

```mach
pub val COMPTIME_UNMEASURABLE:          Kind = 97
```

## val COMPTIME_USER_ERROR

```mach
pub val COMPTIME_USER_ERROR:            Kind = 98
```

## val CONDITION_TYPE

```mach
pub val CONDITION_TYPE:                 Kind = 99
```

## val CONST_OUT_OF_RANGE

```mach
pub val CONST_OUT_OF_RANGE:             Kind = 100
```

## val CONST_TUPLE_DEPENDENT

```mach
pub val CONST_TUPLE_DEPENDENT:          Kind = 101
```

## val DECL_VECTOR_SPELLED_NAME

```mach
pub val DECL_VECTOR_SPELLED_NAME:       Kind = 102
```

## val DECORATOR_ALIGN

```mach
pub val DECORATOR_ALIGN:                Kind = 103
```

## val DECORATOR_ARGUMENT

```mach
pub val DECORATOR_ARGUMENT:             Kind = 104
```

## val DECORATOR_ARITY

```mach
pub val DECORATOR_ARITY:                Kind = 105
```

## val DECORATOR_CONFLICT

```mach
pub val DECORATOR_CONFLICT:             Kind = 106
```

## val DECORATOR_DUPLICATE

```mach
pub val DECORATOR_DUPLICATE:            Kind = 107
```

## val DECORATOR_MISPLACED

```mach
pub val DECORATOR_MISPLACED:            Kind = 108
```

## val DECORATOR_REQUIRES

```mach
pub val DECORATOR_REQUIRES:             Kind = 109
```

## val DECORATOR_UNKNOWN

```mach
pub val DECORATOR_UNKNOWN:              Kind = 110
```

## val EACH_ELEMENT_TYPE

```mach
pub val EACH_ELEMENT_TYPE:              Kind = 111
```

## val EACH_INITIALIZER

```mach
pub val EACH_INITIALIZER:               Kind = 112
```

## val EACH_MUTABLE

```mach
pub val EACH_MUTABLE:                   Kind = 113
```

## val EACH_SEQUENCE

```mach
pub val EACH_SEQUENCE:                  Kind = 114
```

## val EMBED_ARGUMENT

```mach
pub val EMBED_ARGUMENT:                 Kind = 115
```

## val EMBED_ESCAPES_ROOT

```mach
pub val EMBED_ESCAPES_ROOT:             Kind = 116
```

## val EMBED_INITIALIZER

```mach
pub val EMBED_INITIALIZER:              Kind = 117
```

## val EMBED_NO_LOCATION

```mach
pub val EMBED_NO_LOCATION:              Kind = 118
```

## val EMBED_PATH_TEMPLATE

```mach
pub val EMBED_PATH_TEMPLATE:            Kind = 119
```

## val EMBED_SIZE

```mach
pub val EMBED_SIZE:                     Kind = 120
```

## val EMBED_TARGET

```mach
pub val EMBED_TARGET:                   Kind = 121
```

## val EMBED_TYPE

```mach
pub val EMBED_TYPE:                     Kind = 122
```

## val EMBED_UNREADABLE

```mach
pub val EMBED_UNREADABLE:               Kind = 123
```

## val EXPECT_DUPLICATE_KEY

```mach
pub val EXPECT_DUPLICATE_KEY:           Kind = 124
```

## val EXPECT_ERROR_KEY

```mach
pub val EXPECT_ERROR_KEY:               Kind = 125
```

## val EXPECT_UNKNOWN_KEY

```mach
pub val EXPECT_UNKNOWN_KEY:             Kind = 126
```

## val EXT_DATA_INITIALIZER

```mach
pub val EXT_DATA_INITIALIZER:           Kind = 127
```

## val EXT_VARIADIC_BODY

```mach
pub val EXT_VARIADIC_BODY:              Kind = 128
```

## val EXTENSION_DUPLICATE

```mach
pub val EXTENSION_DUPLICATE:            Kind = 129
```

## val EXTENSION_TARGET_ONLY

```mach
pub val EXTENSION_TARGET_ONLY:          Kind = 130
```

## val EXTENSION_UNKNOWN

```mach
pub val EXTENSION_UNKNOWN:              Kind = 131
```

## val FIELD_DUPLICATE_INIT

```mach
pub val FIELD_DUPLICATE_INIT:           Kind = 132
```

## val FIELD_MISSING_VALUE

```mach
pub val FIELD_MISSING_VALUE:            Kind = 133
```

## val FIELD_UNKNOWN

```mach
pub val FIELD_UNKNOWN:                  Kind = 134
```

## val FIELD_VOID_RECEIVER

```mach
pub val FIELD_VOID_RECEIVER:            Kind = 135
```

## val FIN_JUMP

```mach
pub val FIN_JUMP:                       Kind = 136
```

## val FIN_RET

```mach
pub val FIN_RET:                        Kind = 137
```

## val FWD_NOT_SYMBOL

```mach
pub val FWD_NOT_SYMBOL:                 Kind = 138
```

## val FWD_PUB

```mach
pub val FWD_PUB:                        Kind = 139
```

## val GATE_CONST_NOT_INTEGER

```mach
pub val GATE_CONST_NOT_INTEGER:         Kind = 140
```

## val GATE_CYCLE

```mach
pub val GATE_CYCLE:                     Kind = 141
```

## val GATE_NOT_CONSTANT

```mach
pub val GATE_NOT_CONSTANT:              Kind = 142
```

## val GATE_NOT_U8

```mach
pub val GATE_NOT_U8:                    Kind = 143
```

## val GATE_RUNTIME_VALUE

```mach
pub val GATE_RUNTIME_VALUE:             Kind = 144
```

## val GATE_TYPE_QUESTION

```mach
pub val GATE_TYPE_QUESTION:             Kind = 145
```

## val GENERIC_ARGUMENT

```mach
pub val GENERIC_ARGUMENT:               Kind = 146
```

## val GENERIC_ARITY

```mach
pub val GENERIC_ARITY:                  Kind = 147
```

## val GENERIC_COMPTIME_VALUE_PARAM

```mach
pub val GENERIC_COMPTIME_VALUE_PARAM:   Kind = 148
```

## val GENERIC_INSTANTIATION_LIMIT

```mach
pub val GENERIC_INSTANTIATION_LIMIT:    Kind = 149
```

## val GENERIC_MISSING_TYPE_ARGS

```mach
pub val GENERIC_MISSING_TYPE_ARGS:      Kind = 150
```

## val GENERIC_NOT_GENERIC

```mach
pub val GENERIC_NOT_GENERIC:            Kind = 151
```

## val GLOBAL_INIT_NOT_CONSTANT

```mach
pub val GLOBAL_INIT_NOT_CONSTANT:       Kind = 152
```

## val GUARD_ASSIGN

```mach
pub val GUARD_ASSIGN:                   Kind = 153
```

## val GUARD_UNSTABLE_PLACE

```mach
pub val GUARD_UNSTABLE_PLACE:           Kind = 154
```

## val HANDLE_ARGUMENTS

```mach
pub val HANDLE_ARGUMENTS:               Kind = 155
```

## val HANDLE_HAS_BODY

```mach
pub val HANDLE_HAS_BODY:                Kind = 156
```

## val HANDLE_NOT_DEF

```mach
pub val HANDLE_NOT_DEF:                 Kind = 157
```

## val HANDLE_OPERAND_COUNT

```mach
pub val HANDLE_OPERAND_COUNT:           Kind = 158
```

## val HANDLE_OPERAND_REFUSED

```mach
pub val HANDLE_OPERAND_REFUSED:         Kind = 159
```

## val HANDLE_OPERAND_TYPE

```mach
pub val HANDLE_OPERAND_TYPE:            Kind = 160
```

## val HANDLE_OPERAND_WORD

```mach
pub val HANDLE_OPERAND_WORD:            Kind = 161
```

## val HANDLE_ROLE

```mach
pub val HANDLE_ROLE:                    Kind = 162
```

## val HANDLE_STORAGE

```mach
pub val HANDLE_STORAGE:                 Kind = 163
```

## val HANDLE_UNBOUND

```mach
pub val HANDLE_UNBOUND:                 Kind = 164
```

## val HANDLE_UNKNOWN_CONSTRUCTOR

```mach
pub val HANDLE_UNKNOWN_CONSTRUCTOR:     Kind = 165
```

## val INDEX_NOT_INDEXABLE

```mach
pub val INDEX_NOT_INDEXABLE:            Kind = 166
```

## val INDEX_OUT_OF_BOUNDS

```mach
pub val INDEX_OUT_OF_BOUNDS:            Kind = 167
```

## val INDEX_TYPE

```mach
pub val INDEX_TYPE:                     Kind = 168
```

## val LAYOUT_CYCLE

```mach
pub val LAYOUT_CYCLE:                   Kind = 169
```

## val LAYOUT_MEASUREMENT_OVERFLOW

```mach
pub val LAYOUT_MEASUREMENT_OVERFLOW:    Kind = 170
```

## val LAYOUT_UNAVAILABLE

```mach
pub val LAYOUT_UNAVAILABLE:             Kind = 171
```

## val LITERAL_INVALID_ESCAPE

```mach
pub val LITERAL_INVALID_ESCAPE:         Kind = 172
```

## val LITERAL_MALFORMED

```mach
pub val LITERAL_MALFORMED:              Kind = 173
```

## val LITERAL_OUT_OF_RANGE

```mach
pub val LITERAL_OUT_OF_RANGE:           Kind = 174
```

## val LITERAL_UNTERMINATED_CHAR

```mach
pub val LITERAL_UNTERMINATED_CHAR:      Kind = 175
```

## val LITERAL_UNTERMINATED_STRING

```mach
pub val LITERAL_UNTERMINATED_STRING:    Kind = 176
```

## val MODULE_ALIAS_EXPECTED

```mach
pub val MODULE_ALIAS_EXPECTED:          Kind = 177
```

## val MODULE_NOT_IN_DEPS

```mach
pub val MODULE_NOT_IN_DEPS:             Kind = 178
```

## val NAKED_BODY

```mach
pub val NAKED_BODY:                     Kind = 179
```

## val NAME_DUPLICATE

```mach
pub val NAME_DUPLICATE:                 Kind = 180
```

## val NAME_NOT_TYPE

```mach
pub val NAME_NOT_TYPE:                  Kind = 181
```

## val NAME_NOT_VALUE

```mach
pub val NAME_NOT_VALUE:                 Kind = 182
```

## val NAME_UNRESOLVED

```mach
pub val NAME_UNRESOLVED:                Kind = 183
```

## val OP_OPERAND_COUNT

```mach
pub val OP_OPERAND_COUNT:               Kind = 184
```

## val OP_UNKNOWN

```mach
pub val OP_UNKNOWN:                     Kind = 185
```

## val OPERATOR_AGGREGATE_EQUALITY

```mach
pub val OPERATOR_AGGREGATE_EQUALITY:    Kind = 186
```

## val OPERATOR_OPERAND_MISMATCH

```mach
pub val OPERATOR_OPERAND_MISMATCH:      Kind = 187
```

## val OPERATOR_OPERAND_TYPE

```mach
pub val OPERATOR_OPERAND_TYPE:          Kind = 188
```

## val PACK_BODYLESS

```mach
pub val PACK_BODYLESS:                  Kind = 189
```

## val PACK_MEMBER

```mach
pub val PACK_MEMBER:                    Kind = 190
```

## val PACK_SPREAD

```mach
pub val PACK_SPREAD:                    Kind = 191
```

## val PTR_DEREF_NON_POINTER

```mach
pub val PTR_DEREF_NON_POINTER:          Kind = 192
```

## val PTR_UNTYPED_ACCESS

```mach
pub val PTR_UNTYPED_ACCESS:             Kind = 193
```

## val RANGE_COUNT_INVALID

```mach
pub val RANGE_COUNT_INVALID:            Kind = 194
```

## val RANGE_COUNT_NOT_CONSTANT

```mach
pub val RANGE_COUNT_NOT_CONSTANT:       Kind = 195
```

## val RANGE_OUT_OF_BOUNDS

```mach
pub val RANGE_OUT_OF_BOUNDS:            Kind = 196
```

## val RET_VALUE

```mach
pub val RET_VALUE:                      Kind = 197
```

## val SECRET_ADDRESS

```mach
pub val SECRET_ADDRESS:                 Kind = 198
```

## val SECRET_BRANCH

```mach
pub val SECRET_BRANCH:                  Kind = 199
```

## val SECRET_CAST_QUALIFIER

```mach
pub val SECRET_CAST_QUALIFIER:          Kind = 200
```

## val SECRET_INDEX

```mach
pub val SECRET_INDEX:                   Kind = 201
```

## val SECRET_PTR_ERASE

```mach
pub val SECRET_PTR_ERASE:               Kind = 202
```

## val SECRET_STRIP_TARGET

```mach
pub val SECRET_STRIP_TARGET:            Kind = 203
```

## val SECRET_TAG_CASE

```mach
pub val SECRET_TAG_CASE:                Kind = 204
```

## val SECRET_TARGET_UNVERIFIABLE

```mach
pub val SECRET_TARGET_UNVERIFIABLE:     Kind = 205
```

## val SECRET_UNVERIFIABLE

```mach
pub val SECRET_UNVERIFIABLE:            Kind = 206
```

## val SECRET_VARIABLE_LATENCY

```mach
pub val SECRET_VARIABLE_LATENCY:        Kind = 207
```

## val SECRET_VARIADIC

```mach
pub val SECRET_VARIADIC:                Kind = 208
```

## val SECRET_VARIANT_MISMATCH

```mach
pub val SECRET_VARIANT_MISMATCH:        Kind = 209
```

## val SECRET_VARIANT_UNPROVABLE

```mach
pub val SECRET_VARIANT_UNPROVABLE:      Kind = 210
```

## val SEL_NOT_A_CASE

```mach
pub val SEL_NOT_A_CASE:                 Kind = 211
```

## val SEL_NOT_PLACE

```mach
pub val SEL_NOT_PLACE:                  Kind = 212
```

## val SEL_NOT_TAG

```mach
pub val SEL_NOT_TAG:                    Kind = 213
```

## val SHIFT_COUNT

```mach
pub val SHIFT_COUNT:                    Kind = 214
```

## val SOURCE_CONTROL_CHAR

```mach
pub val SOURCE_CONTROL_CHAR:            Kind = 215
```

## val SOURCE_UNEXPECTED_CHAR

```mach
pub val SOURCE_UNEXPECTED_CHAR:         Kind = 216
```

## val SPIRV_CAPABILITY

```mach
pub val SPIRV_CAPABILITY:               Kind = 217
```

## val SPIRV_CONTROL_FLOW

```mach
pub val SPIRV_CONTROL_FLOW:             Kind = 218
```

## val SPIRV_INTERFACE

```mach
pub val SPIRV_INTERFACE:                Kind = 219
```

## val SPIRV_READONLY_STORE

```mach
pub val SPIRV_READONLY_STORE:           Kind = 220
```

## val SPIRV_RECURSION

```mach
pub val SPIRV_RECURSION:                Kind = 221
```

## val SPIRV_RECURSIVE_TYPE

```mach
pub val SPIRV_RECURSIVE_TYPE:           Kind = 222
```

## val SPIRV_STAGE_SIGNATURE

```mach
pub val SPIRV_STAGE_SIGNATURE:          Kind = 223
```

## val SPIRV_UNSUPPORTED

```mach
pub val SPIRV_UNSUPPORTED:              Kind = 224
```

## val STACK_RESERVE_EXCEEDED

```mach
pub val STACK_RESERVE_EXCEEDED:         Kind = 225
```

## val SYNTAX_EXPECTED_DECL

```mach
pub val SYNTAX_EXPECTED_DECL:           Kind = 226
```

## val SYNTAX_EXPECTED_EXPR

```mach
pub val SYNTAX_EXPECTED_EXPR:           Kind = 227
```

## val SYNTAX_EXPECTED_NAME

```mach
pub val SYNTAX_EXPECTED_NAME:           Kind = 228
```

## val SYNTAX_EXPECTED_PATH

```mach
pub val SYNTAX_EXPECTED_PATH:           Kind = 229
```

## val SYNTAX_EXPECTED_TOKEN

```mach
pub val SYNTAX_EXPECTED_TOKEN:          Kind = 230
```

## val SYNTAX_EXPECTED_TYPE

```mach
pub val SYNTAX_EXPECTED_TYPE:           Kind = 231
```

## val SYNTAX_NESTING_DEPTH

```mach
pub val SYNTAX_NESTING_DEPTH:           Kind = 232
```

## val TAG_CONSTRUCTOR_FORM

```mach
pub val TAG_CONSTRUCTOR_FORM:           Kind = 233
```

## val TAG_DISCRIMINATOR_NARROW

```mach
pub val TAG_DISCRIMINATOR_NARROW:       Kind = 234
```

## val TAG_DISCRIMINATOR_TYPE

```mach
pub val TAG_DISCRIMINATOR_TYPE:         Kind = 235
```

## val TAG_DUPLICATE_CASE

```mach
pub val TAG_DUPLICATE_CASE:             Kind = 236
```

## val TAG_NO_CASES

```mach
pub val TAG_NO_CASES:                   Kind = 237
```

## val TAG_NO_DISCRIMINATOR

```mach
pub val TAG_NO_DISCRIMINATOR:           Kind = 238
```

## val TAG_NOT_TAG

```mach
pub val TAG_NOT_TAG:                    Kind = 239
```

## val TAG_PAYLOAD_COUNT

```mach
pub val TAG_PAYLOAD_COUNT:              Kind = 240
```

## val TAG_PAYLOAD_MISSING

```mach
pub val TAG_PAYLOAD_MISSING:            Kind = 241
```

## val TAG_PAYLOAD_NAMED

```mach
pub val TAG_PAYLOAD_NAMED:              Kind = 242
```

## val TAG_PAYLOAD_UNEXPECTED

```mach
pub val TAG_PAYLOAD_UNEXPECTED:         Kind = 243
```

## val TAG_PAYLOAD_UNGUARDED

```mach
pub val TAG_PAYLOAD_UNGUARDED:          Kind = 244
```

## val TAG_SELECTOR_VALUE

```mach
pub val TAG_SELECTOR_VALUE:             Kind = 245
```

## val TAG_UNKNOWN_CASE

```mach
pub val TAG_UNKNOWN_CASE:               Kind = 246
```

## val TARGET_INT_WIDTH

```mach
pub val TARGET_INT_WIDTH:               Kind = 247
```

## val TARGET_NO_FLOAT

```mach
pub val TARGET_NO_FLOAT:                Kind = 248
```

## val TARGET_UNSUPPORTED_OP

```mach
pub val TARGET_UNSUPPORTED_OP:          Kind = 249
```

## val TARGET_VECTOR_WIDTH

```mach
pub val TARGET_VECTOR_WIDTH:            Kind = 250
```

## val TEST_DUPLICATE

```mach
pub val TEST_DUPLICATE:                 Kind = 251
```

## val TEST_STATUS_RANGE

```mach
pub val TEST_STATUS_RANGE:              Kind = 252
```

## val TEST_STRING_NAME

```mach
pub val TEST_STRING_NAME:               Kind = 253
```

## val TESTING_USE

```mach
pub val TESTING_USE:                    Kind = 254
```

## val TYPE_MISMATCH

```mach
pub val TYPE_MISMATCH:                  Kind = 255
```

## val TYPE_RECURSIVE

```mach
pub val TYPE_RECURSIVE:                 Kind = 256
```

## val UNI_LITERAL_FIELDS

```mach
pub val UNI_LITERAL_FIELDS:             Kind = 257
```

## val USE_UNRESOLVED

```mach
pub val USE_UNRESOLVED:                 Kind = 258
```

## val VARIADIC_NO_FIXED_PARAM

```mach
pub val VARIADIC_NO_FIXED_PARAM:        Kind = 259
```

## val VECTOR_INVALID_TYPE

```mach
pub val VECTOR_INVALID_TYPE:            Kind = 260
```

## val VECTOR_LANE_INDEX_DYNAMIC

```mach
pub val VECTOR_LANE_INDEX_DYNAMIC:      Kind = 261
```

## val VECTOR_LITERAL_LENGTH

```mach
pub val VECTOR_LITERAL_LENGTH:          Kind = 262
```

## val VECTOR_LITERAL_NAMED

```mach
pub val VECTOR_LITERAL_NAMED:           Kind = 263
```

## val VECTOR_LITERAL_TYPE

```mach
pub val VECTOR_LITERAL_TYPE:            Kind = 264
```

## val VECTOR_OPERAND_TYPE

```mach
pub val VECTOR_OPERAND_TYPE:            Kind = 265
```

## val VECTOR_OPERATOR_UNSUPPORTED

```mach
pub val VECTOR_OPERATOR_UNSUPPORTED:    Kind = 266
```

## val VECTOR_PACKED_FIELD

```mach
pub val VECTOR_PACKED_FIELD:            Kind = 267
```

## val VECTOR_RANGE_WIDTH

```mach
pub val VECTOR_RANGE_WIDTH:             Kind = 268
```

## val VECTOR_SCALARIZE_REFUSED

```mach
pub val VECTOR_SCALARIZE_REFUSED:       Kind = 269
```

## val VECTOR_SHIFT_COUNT

```mach
pub val VECTOR_SHIFT_COUNT:             Kind = 270
```

## val VECTOR_UNDECLARED_FORM

```mach
pub val VECTOR_UNDECLARED_FORM:         Kind = 271
```

## val VISIBILITY_NOT_EXPORTED

```mach
pub val VISIBILITY_NOT_EXPORTED:        Kind = 272
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
pub val COUNT: usize       = 272
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

