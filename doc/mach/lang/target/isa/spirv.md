# mach.lang.target.isa.spirv

## val SPV_MAGIC

```mach
pub val SPV_MAGIC: u32 = 0x07230203
```

## val SPV_MODULE_SECTION

```mach
pub val SPV_MODULE_SECTION: str = ".spirv"
```

the one section a SPIR-V object image carries: the finished module

## val SPV_VERSION_1_2

```mach
pub val SPV_VERSION_1_2: u32 = 0x00010200
```

## val SPV_VERSION_1_3

```mach
pub val SPV_VERSION_1_3: u32 = 0x00010300
```

## val SPV_VERSION_1_5

```mach
pub val SPV_VERSION_1_5: u32 = 0x00010500
```

## val SPV_VERSION_1_6

```mach
pub val SPV_VERSION_1_6: u32 = 0x00010600
```

## rec EnvProfile

```mach
pub rec EnvProfile;
```

## val ENV_PROFILE_COUNT

```mach
pub val ENV_PROFILE_COUNT: u32 = 4
```

## val SPV_GENERATOR

```mach
pub val SPV_GENERATOR: u32 = 0
```

## val OP_SOURCE

```mach
pub val OP_SOURCE:                    u32 = 3
```

## val OP_NAME

```mach
pub val OP_NAME:                      u32 = 5
```

## val OP_MEMBER_NAME

```mach
pub val OP_MEMBER_NAME:               u32 = 6
```

## val OP_STRING

```mach
pub val OP_STRING:                    u32 = 7
```

## val OP_LINE

```mach
pub val OP_LINE:                      u32 = 8
```

## val OP_NO_LINE

```mach
pub val OP_NO_LINE:                   u32 = 317
```

## val OP_EXTENSION

```mach
pub val OP_EXTENSION:                 u32 = 10
```

## val OP_EXT_INST

```mach
pub val OP_EXT_INST:                  u32 = 12
```

## val OP_MEMORY_MODEL

```mach
pub val OP_MEMORY_MODEL:              u32 = 14
```

## val OP_ENTRY_POINT

```mach
pub val OP_ENTRY_POINT:               u32 = 15
```

## val OP_EXECUTION_MODE

```mach
pub val OP_EXECUTION_MODE:            u32 = 16
```

## val OP_EXECUTION_MODE_ID

```mach
pub val OP_EXECUTION_MODE_ID:         u32 = 331
```

## val OP_CAPABILITY

```mach
pub val OP_CAPABILITY:                u32 = 17
```

## val OP_TYPE_VOID

```mach
pub val OP_TYPE_VOID:                 u32 = 19
```

## val OP_TYPE_BOOL

```mach
pub val OP_TYPE_BOOL:                 u32 = 20
```

## val OP_TYPE_INT

```mach
pub val OP_TYPE_INT:                  u32 = 21
```

## val OP_TYPE_FLOAT

```mach
pub val OP_TYPE_FLOAT:                u32 = 22
```

## val OP_TYPE_VECTOR

```mach
pub val OP_TYPE_VECTOR:               u32 = 23
```

## val OP_TYPE_IMAGE

```mach
pub val OP_TYPE_IMAGE:                u32 = 25
```

## val OP_TYPE_SAMPLER

```mach
pub val OP_TYPE_SAMPLER:              u32 = 26
```

## val OP_TYPE_SAMPLED_IMAGE

```mach
pub val OP_TYPE_SAMPLED_IMAGE:        u32 = 27
```

## val OP_TYPE_ARRAY

```mach
pub val OP_TYPE_ARRAY:                u32 = 28
```

## val OP_TYPE_STRUCT

```mach
pub val OP_TYPE_STRUCT:               u32 = 30
```

## val OP_TYPE_POINTER

```mach
pub val OP_TYPE_POINTER:              u32 = 32
```

## val OP_TYPE_FUNCTION

```mach
pub val OP_TYPE_FUNCTION:             u32 = 33
```

## val OP_CONSTANT_TRUE

```mach
pub val OP_CONSTANT_TRUE:             u32 = 41
```

## val OP_CONSTANT_FALSE

```mach
pub val OP_CONSTANT_FALSE:            u32 = 42
```

## val OP_CONSTANT

```mach
pub val OP_CONSTANT:                  u32 = 43
```

## val OP_CONSTANT_COMPOSITE

```mach
pub val OP_CONSTANT_COMPOSITE:        u32 = 44
```

## val OP_CONSTANT_NULL

```mach
pub val OP_CONSTANT_NULL:             u32 = 46
```

## val OP_SPEC_CONSTANT

```mach
pub val OP_SPEC_CONSTANT:             u32 = 50
```

## val OP_SPEC_CONSTANT_COMPOSITE

```mach
pub val OP_SPEC_CONSTANT_COMPOSITE:   u32 = 51
```

## val OP_FUNCTION

```mach
pub val OP_FUNCTION:                  u32 = 54
```

## val OP_FUNCTION_PARAMETER

```mach
pub val OP_FUNCTION_PARAMETER:        u32 = 55
```

## val OP_FUNCTION_END

```mach
pub val OP_FUNCTION_END:              u32 = 56
```

## val OP_FUNCTION_CALL

```mach
pub val OP_FUNCTION_CALL:             u32 = 57
```

## val OP_VARIABLE

```mach
pub val OP_VARIABLE:                  u32 = 59
```

## val OP_LOAD

```mach
pub val OP_LOAD:                      u32 = 61
```

## val OP_STORE

```mach
pub val OP_STORE:                     u32 = 62
```

## val OP_IMAGE_TEXEL_POINTER

```mach
pub val OP_IMAGE_TEXEL_POINTER:       u32 = 60
```

## val OP_ACCESS_CHAIN

```mach
pub val OP_ACCESS_CHAIN:              u32 = 65
```

## val OP_PTR_ACCESS_CHAIN

```mach
pub val OP_PTR_ACCESS_CHAIN:          u32 = 67
```

## val OP_TYPE_FORWARD_POINTER

```mach
pub val OP_TYPE_FORWARD_POINTER:      u32 = 39
```

## val OP_CONVERT_PTR_TO_U

```mach
pub val OP_CONVERT_PTR_TO_U:          u32 = 117
```

## val OP_CONVERT_U_TO_PTR

```mach
pub val OP_CONVERT_U_TO_PTR:          u32 = 120
```

## val OP_DECORATE

```mach
pub val OP_DECORATE:                  u32 = 71
```

## val OP_MEMBER_DECORATE

```mach
pub val OP_MEMBER_DECORATE:           u32 = 72
```

## val OP_COMPOSITE_CONSTRUCT

```mach
pub val OP_COMPOSITE_CONSTRUCT:       u32 = 80
```

## val OP_COMPOSITE_EXTRACT

```mach
pub val OP_COMPOSITE_EXTRACT:         u32 = 81
```

## val OP_COMPOSITE_INSERT

```mach
pub val OP_COMPOSITE_INSERT:          u32 = 82
```

## val OP_COPY_LOGICAL

```mach
pub val OP_COPY_LOGICAL:              u32 = 400
```

## val OP_SAMPLED_IMAGE

```mach
pub val OP_SAMPLED_IMAGE:             u32 = 86
```

## val OP_IMAGE_SAMPLE_IMPLICIT_LOD

```mach
pub val OP_IMAGE_SAMPLE_IMPLICIT_LOD: u32 = 87
```

## val OP_IMAGE_SAMPLE_EXPLICIT_LOD

```mach
pub val OP_IMAGE_SAMPLE_EXPLICIT_LOD: u32 = 88
```

## val OP_IMAGE_FETCH

```mach
pub val OP_IMAGE_FETCH:               u32 = 95
```

## val OP_IMAGE_GATHER

```mach
pub val OP_IMAGE_GATHER:              u32 = 96
```

## val OP_IMAGE_READ

```mach
pub val OP_IMAGE_READ:                u32 = 98
```

## val OP_IMAGE_WRITE

```mach
pub val OP_IMAGE_WRITE:               u32 = 99
```

## val OP_IMAGE_QUERY_SIZE_LOD

```mach
pub val OP_IMAGE_QUERY_SIZE_LOD:      u32 = 103
```

## val OP_IMAGE_QUERY_SIZE

```mach
pub val OP_IMAGE_QUERY_SIZE:          u32 = 104
```

## val OP_IMAGE_QUERY_LEVELS

```mach
pub val OP_IMAGE_QUERY_LEVELS:        u32 = 106
```

## val OP_IMAGE_QUERY_SAMPLES

```mach
pub val OP_IMAGE_QUERY_SAMPLES:       u32 = 107
```

## val OP_CONVERT_F_TO_U

```mach
pub val OP_CONVERT_F_TO_U:            u32 = 109
```

## val OP_CONVERT_F_TO_S

```mach
pub val OP_CONVERT_F_TO_S:            u32 = 110
```

## val OP_CONVERT_S_TO_F

```mach
pub val OP_CONVERT_S_TO_F:            u32 = 111
```

## val OP_CONVERT_U_TO_F

```mach
pub val OP_CONVERT_U_TO_F:            u32 = 112
```

## val OP_U_CONVERT

```mach
pub val OP_U_CONVERT:                 u32 = 113
```

## val OP_S_CONVERT

```mach
pub val OP_S_CONVERT:                 u32 = 114
```

## val OP_F_CONVERT

```mach
pub val OP_F_CONVERT:                 u32 = 115
```

## val OP_BITCAST

```mach
pub val OP_BITCAST:                   u32 = 124
```

## val OP_S_NEGATE

```mach
pub val OP_S_NEGATE:                  u32 = 126
```

## val OP_F_NEGATE

```mach
pub val OP_F_NEGATE:                  u32 = 127
```

## val OP_I_ADD

```mach
pub val OP_I_ADD:                     u32 = 128
```

## val OP_F_ADD

```mach
pub val OP_F_ADD:                     u32 = 129
```

## val OP_I_SUB

```mach
pub val OP_I_SUB:                     u32 = 130
```

## val OP_F_SUB

```mach
pub val OP_F_SUB:                     u32 = 131
```

## val OP_I_MUL

```mach
pub val OP_I_MUL:                     u32 = 132
```

## val OP_F_MUL

```mach
pub val OP_F_MUL:                     u32 = 133
```

## val OP_U_DIV

```mach
pub val OP_U_DIV:                     u32 = 134
```

## val OP_S_DIV

```mach
pub val OP_S_DIV:                     u32 = 135
```

## val OP_F_DIV

```mach
pub val OP_F_DIV:                     u32 = 136
```

## val OP_U_MOD

```mach
pub val OP_U_MOD:                     u32 = 137
```

## val OP_S_REM

```mach
pub val OP_S_REM:                     u32 = 138
```

## val OP_F_REM

```mach
pub val OP_F_REM:                     u32 = 140
```

## val OP_DOT

```mach
pub val OP_DOT:                       u32 = 148
```

## val OP_SELECT

```mach
pub val OP_SELECT:                    u32 = 169
```

## val OP_I_EQUAL

```mach
pub val OP_I_EQUAL:                   u32 = 170
```

## val OP_I_NOT_EQUAL

```mach
pub val OP_I_NOT_EQUAL:               u32 = 171
```

## val OP_U_LESS_THAN

```mach
pub val OP_U_LESS_THAN:               u32 = 176
```

## val OP_S_LESS_THAN

```mach
pub val OP_S_LESS_THAN:               u32 = 177
```

## val OP_U_LESS_THAN_EQUAL

```mach
pub val OP_U_LESS_THAN_EQUAL:         u32 = 178
```

## val OP_S_LESS_THAN_EQUAL

```mach
pub val OP_S_LESS_THAN_EQUAL:         u32 = 179
```

## val OP_F_ORD_EQUAL

```mach
pub val OP_F_ORD_EQUAL:               u32 = 180
```

## val OP_F_UNORD_NOT_EQUAL

```mach
pub val OP_F_UNORD_NOT_EQUAL:         u32 = 183
```

## val OP_F_ORD_LESS_THAN

```mach
pub val OP_F_ORD_LESS_THAN:           u32 = 184
```

## val OP_F_ORD_GREATER_THAN

```mach
pub val OP_F_ORD_GREATER_THAN:        u32 = 186
```

## val OP_F_ORD_LESS_THAN_EQUAL

```mach
pub val OP_F_ORD_LESS_THAN_EQUAL:     u32 = 188
```

## val OP_F_ORD_GREATER_THAN_EQUAL

```mach
pub val OP_F_ORD_GREATER_THAN_EQUAL:  u32 = 190
```

## val OP_SHIFT_RIGHT_LOGICAL

```mach
pub val OP_SHIFT_RIGHT_LOGICAL:       u32 = 194
```

## val OP_SHIFT_RIGHT_ARITHMETIC

```mach
pub val OP_SHIFT_RIGHT_ARITHMETIC:    u32 = 195
```

## val OP_SHIFT_LEFT_LOGICAL

```mach
pub val OP_SHIFT_LEFT_LOGICAL:        u32 = 196
```

## val OP_BITWISE_OR

```mach
pub val OP_BITWISE_OR:                u32 = 197
```

## val OP_BITWISE_XOR

```mach
pub val OP_BITWISE_XOR:               u32 = 198
```

## val OP_BITWISE_AND

```mach
pub val OP_BITWISE_AND:               u32 = 199
```

## val OP_NOT

```mach
pub val OP_NOT:                       u32 = 200
```

## val OP_CONTROL_BARRIER

```mach
pub val OP_CONTROL_BARRIER:           u32 = 224
```

## val OP_MEMORY_BARRIER

```mach
pub val OP_MEMORY_BARRIER:            u32 = 225
```

## val OP_ATOMIC_LOAD

```mach
pub val OP_ATOMIC_LOAD:               u32 = 227
```

## val OP_ATOMIC_STORE

```mach
pub val OP_ATOMIC_STORE:              u32 = 228
```

## val OP_ATOMIC_EXCHANGE

```mach
pub val OP_ATOMIC_EXCHANGE:           u32 = 229
```

## val OP_ATOMIC_COMPARE_EXCHANGE

```mach
pub val OP_ATOMIC_COMPARE_EXCHANGE:   u32 = 230
```

## val OP_ATOMIC_I_INCREMENT

```mach
pub val OP_ATOMIC_I_INCREMENT:        u32 = 232
```

## val OP_ATOMIC_I_DECREMENT

```mach
pub val OP_ATOMIC_I_DECREMENT:        u32 = 233
```

## val OP_ATOMIC_I_ADD

```mach
pub val OP_ATOMIC_I_ADD:              u32 = 234
```

## val OP_ATOMIC_I_SUB

```mach
pub val OP_ATOMIC_I_SUB:              u32 = 235
```

## val OP_ATOMIC_S_MIN

```mach
pub val OP_ATOMIC_S_MIN:              u32 = 236
```

## val OP_ATOMIC_U_MIN

```mach
pub val OP_ATOMIC_U_MIN:              u32 = 237
```

## val OP_ATOMIC_S_MAX

```mach
pub val OP_ATOMIC_S_MAX:              u32 = 238
```

## val OP_ATOMIC_U_MAX

```mach
pub val OP_ATOMIC_U_MAX:              u32 = 239
```

## val OP_ATOMIC_AND

```mach
pub val OP_ATOMIC_AND:                u32 = 240
```

## val OP_ATOMIC_OR

```mach
pub val OP_ATOMIC_OR:                 u32 = 241
```

## val OP_ATOMIC_XOR

```mach
pub val OP_ATOMIC_XOR:                u32 = 242
```

## val OP_GROUP_NON_UNIFORM_I_ADD

```mach
pub val OP_GROUP_NON_UNIFORM_I_ADD:   u32 = 349
```

## val OP_ATOMIC_F_MIN_EXT

```mach
pub val OP_ATOMIC_F_MIN_EXT:          u32 = 5614
```

## val OP_ATOMIC_F_MAX_EXT

```mach
pub val OP_ATOMIC_F_MAX_EXT:          u32 = 5615
```

## val OP_ATOMIC_F_ADD_EXT

```mach
pub val OP_ATOMIC_F_ADD_EXT:          u32 = 6035
```

## val OP_PHI

```mach
pub val OP_PHI:                       u32 = 245
```

## val OP_LOOP_MERGE

```mach
pub val OP_LOOP_MERGE:                u32 = 246
```

## val OP_SELECTION_MERGE

```mach
pub val OP_SELECTION_MERGE:           u32 = 247
```

## val OP_LABEL

```mach
pub val OP_LABEL:                     u32 = 248
```

## val OP_BRANCH

```mach
pub val OP_BRANCH:                    u32 = 249
```

## val OP_BRANCH_CONDITIONAL

```mach
pub val OP_BRANCH_CONDITIONAL:        u32 = 250
```

## val OP_SWITCH

```mach
pub val OP_SWITCH:                    u32 = 251
```

## val OP_RETURN

```mach
pub val OP_RETURN:                    u32 = 253
```

## val OP_RETURN_VALUE

```mach
pub val OP_RETURN_VALUE:              u32 = 254
```

## val OP_UNREACHABLE

```mach
pub val OP_UNREACHABLE:               u32 = 255
```

## val OP_IMAGE_SAMPLE_DREF_IMPLICIT_LOD

```mach
pub val OP_IMAGE_SAMPLE_DREF_IMPLICIT_LOD: u32 = 89
```

the depth-comparison image instructions beside OpImageSampleImplicitLod

## val OP_IMAGE_SAMPLE_DREF_EXPLICIT_LOD

```mach
pub val OP_IMAGE_SAMPLE_DREF_EXPLICIT_LOD: u32 = 90
```

## val OP_IMAGE_DREF_GATHER

```mach
pub val OP_IMAGE_DREF_GATHER:              u32 = 97
```

## val OP_GROUP_NON_UNIFORM_ELECT

```mach
pub val OP_GROUP_NON_UNIFORM_ELECT:              u32 = 333
```

the non-uniform subgroup instructions beside OpGroupNonUniformIAdd

## val OP_GROUP_NON_UNIFORM_ALL

```mach
pub val OP_GROUP_NON_UNIFORM_ALL:                u32 = 334
```

## val OP_GROUP_NON_UNIFORM_ANY

```mach
pub val OP_GROUP_NON_UNIFORM_ANY:                u32 = 335
```

## val OP_GROUP_NON_UNIFORM_ALL_EQUAL

```mach
pub val OP_GROUP_NON_UNIFORM_ALL_EQUAL:          u32 = 336
```

## val OP_GROUP_NON_UNIFORM_BROADCAST

```mach
pub val OP_GROUP_NON_UNIFORM_BROADCAST:          u32 = 337
```

## val OP_GROUP_NON_UNIFORM_BROADCAST_FIRST

```mach
pub val OP_GROUP_NON_UNIFORM_BROADCAST_FIRST:    u32 = 338
```

## val OP_GROUP_NON_UNIFORM_BALLOT

```mach
pub val OP_GROUP_NON_UNIFORM_BALLOT:             u32 = 339
```

## val OP_GROUP_NON_UNIFORM_INVERSE_BALLOT

```mach
pub val OP_GROUP_NON_UNIFORM_INVERSE_BALLOT:     u32 = 340
```

## val OP_GROUP_NON_UNIFORM_BALLOT_BIT_EXTRACT

```mach
pub val OP_GROUP_NON_UNIFORM_BALLOT_BIT_EXTRACT: u32 = 341
```

## val OP_GROUP_NON_UNIFORM_BALLOT_BIT_COUNT

```mach
pub val OP_GROUP_NON_UNIFORM_BALLOT_BIT_COUNT:   u32 = 342
```

## val OP_GROUP_NON_UNIFORM_BALLOT_FIND_LSB

```mach
pub val OP_GROUP_NON_UNIFORM_BALLOT_FIND_LSB:    u32 = 343
```

## val OP_GROUP_NON_UNIFORM_BALLOT_FIND_MSB

```mach
pub val OP_GROUP_NON_UNIFORM_BALLOT_FIND_MSB:    u32 = 344
```

## val OP_GROUP_NON_UNIFORM_SHUFFLE

```mach
pub val OP_GROUP_NON_UNIFORM_SHUFFLE:            u32 = 345
```

## val OP_GROUP_NON_UNIFORM_SHUFFLE_XOR

```mach
pub val OP_GROUP_NON_UNIFORM_SHUFFLE_XOR:        u32 = 346
```

## val OP_GROUP_NON_UNIFORM_SHUFFLE_UP

```mach
pub val OP_GROUP_NON_UNIFORM_SHUFFLE_UP:         u32 = 347
```

## val OP_GROUP_NON_UNIFORM_SHUFFLE_DOWN

```mach
pub val OP_GROUP_NON_UNIFORM_SHUFFLE_DOWN:       u32 = 348
```

## val OP_GROUP_NON_UNIFORM_F_ADD

```mach
pub val OP_GROUP_NON_UNIFORM_F_ADD:              u32 = 350
```

## val OP_GROUP_NON_UNIFORM_I_MUL

```mach
pub val OP_GROUP_NON_UNIFORM_I_MUL:              u32 = 351
```

## val OP_GROUP_NON_UNIFORM_F_MUL

```mach
pub val OP_GROUP_NON_UNIFORM_F_MUL:              u32 = 352
```

## val OP_GROUP_NON_UNIFORM_S_MIN

```mach
pub val OP_GROUP_NON_UNIFORM_S_MIN:              u32 = 353
```

## val OP_GROUP_NON_UNIFORM_U_MIN

```mach
pub val OP_GROUP_NON_UNIFORM_U_MIN:              u32 = 354
```

## val OP_GROUP_NON_UNIFORM_F_MIN

```mach
pub val OP_GROUP_NON_UNIFORM_F_MIN:              u32 = 355
```

## val OP_GROUP_NON_UNIFORM_S_MAX

```mach
pub val OP_GROUP_NON_UNIFORM_S_MAX:              u32 = 356
```

## val OP_GROUP_NON_UNIFORM_U_MAX

```mach
pub val OP_GROUP_NON_UNIFORM_U_MAX:              u32 = 357
```

## val OP_GROUP_NON_UNIFORM_F_MAX

```mach
pub val OP_GROUP_NON_UNIFORM_F_MAX:              u32 = 358
```

## val OP_GROUP_NON_UNIFORM_BITWISE_AND

```mach
pub val OP_GROUP_NON_UNIFORM_BITWISE_AND:        u32 = 359
```

## val OP_GROUP_NON_UNIFORM_BITWISE_OR

```mach
pub val OP_GROUP_NON_UNIFORM_BITWISE_OR:         u32 = 360
```

## val OP_GROUP_NON_UNIFORM_BITWISE_XOR

```mach
pub val OP_GROUP_NON_UNIFORM_BITWISE_XOR:        u32 = 361
```

## val OP_GROUP_NON_UNIFORM_LOGICAL_AND

```mach
pub val OP_GROUP_NON_UNIFORM_LOGICAL_AND:        u32 = 362
```

## val OP_GROUP_NON_UNIFORM_LOGICAL_OR

```mach
pub val OP_GROUP_NON_UNIFORM_LOGICAL_OR:         u32 = 363
```

## val OP_GROUP_NON_UNIFORM_LOGICAL_XOR

```mach
pub val OP_GROUP_NON_UNIFORM_LOGICAL_XOR:        u32 = 364
```

## val OP_GROUP_NON_UNIFORM_QUAD_BROADCAST

```mach
pub val OP_GROUP_NON_UNIFORM_QUAD_BROADCAST:     u32 = 365
```

## val OP_GROUP_NON_UNIFORM_QUAD_SWAP

```mach
pub val OP_GROUP_NON_UNIFORM_QUAD_SWAP:          u32 = 366
```

## val CAP_SHADER

```mach
pub val CAP_SHADER:  u32 = 1
```

## val CAP_LINKAGE

```mach
pub val CAP_LINKAGE: u32 = 5
```

## val CAP_FLOAT16

```mach
pub val CAP_FLOAT16: u32 = 9
```

## val CAP_FLOAT64

```mach
pub val CAP_FLOAT64: u32 = 10
```

## val CAP_INT64

```mach
pub val CAP_INT64:   u32 = 11
```

## val CAP_INT16

```mach
pub val CAP_INT16:   u32 = 22
```

## val CAP_INT8

```mach
pub val CAP_INT8:    u32 = 39
```

## val CAP_IMAGE_GATHER_EXTENDED

```mach
pub val CAP_IMAGE_GATHER_EXTENDED:          u32 = 25
```

## val CAP_STORAGE_IMAGE_MULTISAMPLE

```mach
pub val CAP_STORAGE_IMAGE_MULTISAMPLE:      u32 = 27
```

## val CAP_MIN_LOD

```mach
pub val CAP_MIN_LOD:                        u32 = 42
```

## val CAP_SAMPLED_BUFFER

```mach
pub val CAP_SAMPLED_BUFFER:                 u32 = 46
```

## val CAP_IMAGE_BUFFER

```mach
pub val CAP_IMAGE_BUFFER:                   u32 = 47
```

## val CAP_IMAGE_MS_ARRAY

```mach
pub val CAP_IMAGE_MS_ARRAY:                 u32 = 48
```

## val CAP_STORAGE_IMAGE_EXTENDED_FORMATS

```mach
pub val CAP_STORAGE_IMAGE_EXTENDED_FORMATS: u32 = 49
```

## val CAP_IMAGE_QUERY

```mach
pub val CAP_IMAGE_QUERY:                    u32 = 50
```

## val CAP_STORAGE_IMAGE_READ_NO_FORMAT

```mach
pub val CAP_STORAGE_IMAGE_READ_NO_FORMAT:   u32 = 55
```

## val CAP_STORAGE_IMAGE_WRITE_NO_FORMAT

```mach
pub val CAP_STORAGE_IMAGE_WRITE_NO_FORMAT:  u32 = 56
```

## val CAP_GROUP_NON_UNIFORM

```mach
pub val CAP_GROUP_NON_UNIFORM:            u32 = 61
```

## val CAP_GROUP_NON_UNIFORM_ARITHMETIC

```mach
pub val CAP_GROUP_NON_UNIFORM_ARITHMETIC: u32 = 63
```

## val CAP_GROUP_NON_UNIFORM_CLUSTERED

```mach
pub val CAP_GROUP_NON_UNIFORM_CLUSTERED:  u32 = 67
```

## val CAP_GROUP_NON_UNIFORM_VOTE

```mach
pub val CAP_GROUP_NON_UNIFORM_VOTE:             u32 = 62
```

## val CAP_GROUP_NON_UNIFORM_BALLOT

```mach
pub val CAP_GROUP_NON_UNIFORM_BALLOT:           u32 = 64
```

## val CAP_GROUP_NON_UNIFORM_SHUFFLE

```mach
pub val CAP_GROUP_NON_UNIFORM_SHUFFLE:          u32 = 65
```

## val CAP_GROUP_NON_UNIFORM_SHUFFLE_RELATIVE

```mach
pub val CAP_GROUP_NON_UNIFORM_SHUFFLE_RELATIVE: u32 = 66
```

## val CAP_GROUP_NON_UNIFORM_QUAD

```mach
pub val CAP_GROUP_NON_UNIFORM_QUAD:             u32 = 68
```

## val CAP_INT64_ATOMICS

```mach
pub val CAP_INT64_ATOMICS:                      u32 = 12
```

## val CAP_ATOMIC_FLOAT32_MIN_MAX

```mach
pub val CAP_ATOMIC_FLOAT32_MIN_MAX:             u32 = 5612
```

## val CAP_ATOMIC_FLOAT64_MIN_MAX

```mach
pub val CAP_ATOMIC_FLOAT64_MIN_MAX:             u32 = 5613
```

## val CAP_ATOMIC_FLOAT32_ADD

```mach
pub val CAP_ATOMIC_FLOAT32_ADD:                 u32 = 6033
```

## val CAP_ATOMIC_FLOAT64_ADD

```mach
pub val CAP_ATOMIC_FLOAT64_ADD:                 u32 = 6034
```

## val CAP_INT64_IMAGE

```mach
pub val CAP_INT64_IMAGE:                        u32 = 5016
```

## val CAP_VULKAN_MEMORY_MODEL_DEVICE_SCOPE

```mach
pub val CAP_VULKAN_MEMORY_MODEL_DEVICE_SCOPE:   u32 = 5346
```

## val CAP_PHYSICAL_STORAGE_BUFFER_ADDRESSES

```mach
pub val CAP_PHYSICAL_STORAGE_BUFFER_ADDRESSES:  u32 = 5347
```

## val CAP_ATOMIC_FLOAT16_ADD

```mach
pub val CAP_ATOMIC_FLOAT16_ADD:                 u32 = 6095
```

## val CAP_ATOMIC_FLOAT16_MIN_MAX

```mach
pub val CAP_ATOMIC_FLOAT16_MIN_MAX:             u32 = 5616
```

## val CAP_STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val CAP_STORAGE_BUFFER_16BIT_ACCESS:             u32 = 4433
```

## val CAP_UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val CAP_UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS: u32 = 4434
```

## val CAP_STORAGE_PUSH_CONSTANT16

```mach
pub val CAP_STORAGE_PUSH_CONSTANT16:                 u32 = 4435
```

## val CAP_STORAGE_INPUT_OUTPUT16

```mach
pub val CAP_STORAGE_INPUT_OUTPUT16:                  u32 = 4436
```

## val CAP_STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val CAP_STORAGE_BUFFER_8BIT_ACCESS:              u32 = 4448
```

## val CAP_UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val CAP_UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS:  u32 = 4449
```

## val CAP_STORAGE_PUSH_CONSTANT8

```mach
pub val CAP_STORAGE_PUSH_CONSTANT8:                  u32 = 4450
```

## rec Capability

```mach
pub rec Capability;
```

a capability beyond Shader and Linkage, which a module declares only when something
it emits needs it: a type's width or dimensionality, an instruction row, a literal's
value or an operand's type. `need` is its bit in Builder.caps_needed and the table's
order is the order the module declares them in. an environment's ceiling decides the
capabilities in CEILING_DOMAIN, and every capability needs its SPIR-V version and,
when `extension` is not nil, the SPIR-V extension that defines it, which the module
then declares with OpExtension below `ext_core`, the version that made it core (0 for
an extension no version took in). `graphics` is the extension a use from a vertex or
fragment stage needs beyond what the use itself names, 0 when every stage has the
capability alike. `feature` is the device feature a type's capability needs under an
environment, whose ceiling only admits it (#4318), 0 for a capability raised by a use,
which names its own

## val NEED_INT8

```mach
pub val NEED_INT8:                               u64 = 0x001
```

## val NEED_INT16

```mach
pub val NEED_INT16:                              u64 = 0x002
```

## val NEED_INT64

```mach
pub val NEED_INT64:                              u64 = 0x004
```

## val NEED_FLOAT16

```mach
pub val NEED_FLOAT16:                            u64 = 0x008
```

## val NEED_FLOAT64

```mach
pub val NEED_FLOAT64:                            u64 = 0x010
```

## val NEED_SAMPLED_1D

```mach
pub val NEED_SAMPLED_1D:                         u64 = 0x020
```

## val NEED_SAMPLED_CUBE_ARRAY

```mach
pub val NEED_SAMPLED_CUBE_ARRAY:                 u64 = 0x040
```

## val NEED_IMAGE_1D

```mach
pub val NEED_IMAGE_1D:                           u64 = 0x400
```

## val NEED_IMAGE_CUBE_ARRAY

```mach
pub val NEED_IMAGE_CUBE_ARRAY:                   u64 = 0x800
```

## val NEED_SAMPLED_BUFFER

```mach
pub val NEED_SAMPLED_BUFFER:                     u64 = 0x1000
```

## val NEED_IMAGE_BUFFER

```mach
pub val NEED_IMAGE_BUFFER:                       u64 = 0x2000
```

## val NEED_STORAGE_EXTENDED_FORMATS

```mach
pub val NEED_STORAGE_EXTENDED_FORMATS:           u64 = 0x4000
```

## val NEED_VULKAN_MEMORY_MODEL

```mach
pub val NEED_VULKAN_MEMORY_MODEL:              u64 = 0x200000000
```

## val NEED_PHYSICAL_STORAGE_BUFFER

```mach
pub val NEED_PHYSICAL_STORAGE_BUFFER:          u64 = 0x800000000
```

## val CAPABILITY_COUNT

```mach
pub val CAPABILITY_COUNT: u32 = 45
```

## val SPV_KHR_VULKAN_MEMORY_MODEL

```mach
pub val SPV_KHR_VULKAN_MEMORY_MODEL: str = "SPV_KHR_vulkan_memory_model"
```

the extension defining the Vulkan memory model below SPIR-V 1.5, where it is core.
the module declares it with the model rather than with a capability, since it
covers both of the model's capabilities

## val SPV_KHR_PHYSICAL_STORAGE_BUFFER

```mach
pub val SPV_KHR_PHYSICAL_STORAGE_BUFFER: str = "SPV_KHR_physical_storage_buffer"
```

the extension defining physical pointers below SPIR-V 1.5, where they are core. the
module declares it with the PhysicalStorageBuffer64 addressing model it brings

## val CAPABILITIES

```mach
pub val CAPABILITIES: [CAPABILITY_COUNT]Capability = [CAPABILITY_COUNT]Capability;
```

## val CEILING_DOMAIN

```mach
pub val CEILING_DOMAIN: u64 = NEED_INT8 | NEED_INT16 | NEED_INT64 | NEED_FLOAT16 | NEED_FLOAT64 | VULKAN_IMAGE_CEILING
```

the capabilities an environment's ceiling speaks for. the others are held to their
version and to the extensions the row that needs them names

## fun capability_of

```mach
pub fun capability_of(word: u32) *Capability;
```

the table row of the capability word `word`, nil when the table has none

## fun version_major

```mach
pub fun version_major(version: u32) u32;
```

`SPIR-V 1.3` for 0x00010300

## fun version_minor

```mach
pub fun version_minor(version: u32) u32;
```

## fun entry_interface_lists

```mach
pub fun entry_interface_lists(version: u32, storage: u32) bool;
```

before 1.4 an entry point's interface list names only its Input and Output
variables, from 1.4 every global the entry point statically uses

## fun storage_buffer_class

```mach
pub fun storage_buffer_class(version: u32) u32;
```

the StorageBuffer storage class is core from 1.3. before that a storage buffer is
a Uniform variable whose block is decorated BufferBlock

## fun storage_block_decoration

```mach
pub fun storage_block_decoration(version: u32) u32;
```

## val EXT_FLOAT16

```mach
pub val EXT_FLOAT16:                      u64 = 0x1
```

the spirv extension vocabulary: capabilities and device features an
environment guarantees beyond the core, or a consumer enables, which the
catalog's rows, the instruction rows and the emitter read. float16 is the
Float16 capability, so f16 is the native OpTypeFloat 16 (#3801).
zero_init_workgroup is the shaderZeroInitializeWorkgroupMemory feature, so a
`#[shared]` variable takes an OpConstantNull initializer instead of the zeroing
the compiler inserts (#4270). the subgroup_ family are the VOTE, ARITHMETIC,
BALLOT, SHUFFLE, SHUFFLE_RELATIVE, CLUSTERED and QUAD bits of Vulkan's
subgroupSupportedOperations, which no Vulkan version guarantees, and
subgroup_graphics_stages is subgroupSupportedStages reaching the vertex and
fragment stages, where Vulkan guarantees only compute. storage_read_without_format and
storage_write_without_format are the shaderStorageImageReadWithoutFormat and
shaderStorageImageWriteWithoutFormat features, so a storage image of Unknown
format may be read or written (#4272). storage_image_multisample is the
shaderStorageImageMultisample feature, which enables both StorageImageMultisample
and ImageMSArray, so a storage image may be multisampled, arrayed or not (#4298).
resource_min_lod is the shaderResourceMinLod feature, which enables MinLod, so a
sample may name the least level of detail it reads. image_gather_extended is the
shaderImageGatherExtended feature, which enables ImageGatherExtended, so a gather
may take an offset computed at run time. maintenance8 is the
maintenance8 feature, under which a fetch or a sample takes a run-time offset too
(VUID-RuntimeSpirv-Offset-10213) (#4303).
vulkan_memory_model is the vulkanMemoryModel feature, and holding it selects the
Vulkan memory model in place of GLSL450. vulkan_memory_model_device_scope is
vulkanMemoryModelDeviceScope, which the Device scope needs under that model (#4308).
int8, int16, int64, float16 and float64 are the shaderInt8, shaderInt16, shaderInt64,
shaderFloat16 and shaderFloat64 features, which no Vulkan version guarantees: an
environment's ceiling only admits their capabilities (#4318). under int8 and int16 an
integer of that width is computed at its own width rather than carried in a wider
one (#4302), and under float16 an f16 is the native OpTypeFloat 16 (#3801).
buffer_device_address is the bufferDeviceAddress feature, under which a pointer held in
memory is a physical pointer into a buffer the host passes by address (#4307).
the storage features are Vulkan's 16- and 8-bit storage features, each the one
capability of its name: storage_buffer_16bit_access is storageBuffer16BitAccess,
so a storage buffer, or a buffer reached through a physical pointer, may hold a
16-bit member, uniform_and_storage_buffer_16bit_access
a uniform block, storage_push_constant16 a push block and storage_input_output16 a
stage interface, and the 8-bit three the same for an 8-bit member. no Vulkan version
guarantees one, and Vulkan has no 8-bit stage interface (#4299)

## val EXT_ZERO_INIT_WORKGROUP

```mach
pub val EXT_ZERO_INIT_WORKGROUP:          u64 = 0x2
```

## val EXT_SUBGROUP_ARITHMETIC

```mach
pub val EXT_SUBGROUP_ARITHMETIC:          u64 = 0x4
```

## val EXT_SUBGROUP_CLUSTERED

```mach
pub val EXT_SUBGROUP_CLUSTERED:           u64 = 0x8
```

## val EXT_STORAGE_READ_WITHOUT_FORMAT

```mach
pub val EXT_STORAGE_READ_WITHOUT_FORMAT:  u64 = 0x10
```

## val EXT_STORAGE_WRITE_WITHOUT_FORMAT

```mach
pub val EXT_STORAGE_WRITE_WITHOUT_FORMAT: u64 = 0x20
```

## val EXT_SUBGROUP_VOTE

```mach
pub val EXT_SUBGROUP_VOTE:                u64 = 0x40
```

## val EXT_SUBGROUP_BALLOT

```mach
pub val EXT_SUBGROUP_BALLOT:              u64 = 0x80
```

## val EXT_SUBGROUP_SHUFFLE

```mach
pub val EXT_SUBGROUP_SHUFFLE:             u64 = 0x100
```

## val EXT_SUBGROUP_SHUFFLE_RELATIVE

```mach
pub val EXT_SUBGROUP_SHUFFLE_RELATIVE:    u64 = 0x200
```

## val EXT_SUBGROUP_QUAD

```mach
pub val EXT_SUBGROUP_QUAD:                u64 = 0x400
```

## val EXT_BUFFER_INT64_ATOMICS

```mach
pub val EXT_BUFFER_INT64_ATOMICS:             u64 = 0x1000
```

the atomic features of VkPhysicalDeviceShaderAtomicInt64Features,
VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT, VkPhysicalDeviceShaderAtomicFloatFeaturesEXT
and VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT, each named for its shaderBuffer*,
shaderShared* or shaderImage* member: an atomic on a type beyond 32-bit integers needs the
one for its storage class, which no Vulkan version guarantees. image_int64_atomics is also
the feature that enables Int64ImageEXT, which any image of 64-bit texels declares

## val EXT_SHARED_INT64_ATOMICS

```mach
pub val EXT_SHARED_INT64_ATOMICS:             u64 = 0x2000
```

## val EXT_BUFFER_FLOAT32_ATOMICS

```mach
pub val EXT_BUFFER_FLOAT32_ATOMICS:           u64 = 0x4000
```

## val EXT_BUFFER_FLOAT32_ATOMIC_ADD

```mach
pub val EXT_BUFFER_FLOAT32_ATOMIC_ADD:        u64 = 0x8000
```

## val EXT_BUFFER_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val EXT_BUFFER_FLOAT32_ATOMIC_MIN_MAX:    u64 = 0x10000
```

## val EXT_BUFFER_FLOAT64_ATOMICS

```mach
pub val EXT_BUFFER_FLOAT64_ATOMICS:           u64 = 0x20000
```

## val EXT_BUFFER_FLOAT64_ATOMIC_ADD

```mach
pub val EXT_BUFFER_FLOAT64_ATOMIC_ADD:        u64 = 0x40000
```

## val EXT_BUFFER_FLOAT64_ATOMIC_MIN_MAX

```mach
pub val EXT_BUFFER_FLOAT64_ATOMIC_MIN_MAX:    u64 = 0x80000
```

## val EXT_SHARED_FLOAT32_ATOMICS

```mach
pub val EXT_SHARED_FLOAT32_ATOMICS:           u64 = 0x100000
```

## val EXT_SHARED_FLOAT32_ATOMIC_ADD

```mach
pub val EXT_SHARED_FLOAT32_ATOMIC_ADD:        u64 = 0x200000
```

## val EXT_SHARED_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val EXT_SHARED_FLOAT32_ATOMIC_MIN_MAX:    u64 = 0x400000
```

## val EXT_SHARED_FLOAT64_ATOMICS

```mach
pub val EXT_SHARED_FLOAT64_ATOMICS:           u64 = 0x800000
```

## val EXT_SHARED_FLOAT64_ATOMIC_ADD

```mach
pub val EXT_SHARED_FLOAT64_ATOMIC_ADD:        u64 = 0x1000000
```

## val EXT_SHARED_FLOAT64_ATOMIC_MIN_MAX

```mach
pub val EXT_SHARED_FLOAT64_ATOMIC_MIN_MAX:    u64 = 0x2000000
```

## val EXT_STORAGE_IMAGE_MULTISAMPLE

```mach
pub val EXT_STORAGE_IMAGE_MULTISAMPLE:        u64 = 0x4000000
```

## val EXT_RESOURCE_MIN_LOD

```mach
pub val EXT_RESOURCE_MIN_LOD:                 u64 = 0x8000000
```

## val EXT_IMAGE_GATHER_EXTENDED

```mach
pub val EXT_IMAGE_GATHER_EXTENDED:            u64 = 0x10000000
```

## val EXT_MAINTENANCE8

```mach
pub val EXT_MAINTENANCE8:                     u64 = 0x20000000
```

## val EXT_IMAGE_INT64_ATOMICS

```mach
pub val EXT_IMAGE_INT64_ATOMICS:              u64 = 0x40000000
```

## val EXT_IMAGE_FLOAT32_ATOMICS

```mach
pub val EXT_IMAGE_FLOAT32_ATOMICS:            u64 = 0x80000000
```

## val EXT_IMAGE_FLOAT32_ATOMIC_ADD

```mach
pub val EXT_IMAGE_FLOAT32_ATOMIC_ADD:         u64 = 0x100000000
```

## val EXT_IMAGE_FLOAT32_ATOMIC_MIN_MAX

```mach
pub val EXT_IMAGE_FLOAT32_ATOMIC_MIN_MAX:     u64 = 0x200000000
```

## val EXT_VULKAN_MEMORY_MODEL

```mach
pub val EXT_VULKAN_MEMORY_MODEL:              u64 = 0x400000000
```

## val EXT_VULKAN_MEMORY_MODEL_DEVICE_SCOPE

```mach
pub val EXT_VULKAN_MEMORY_MODEL_DEVICE_SCOPE: u64 = 0x800000000
```

## val EXT_INT8

```mach
pub val EXT_INT8:                             u64 = 0x1000000000
```

## val EXT_INT16

```mach
pub val EXT_INT16:                            u64 = 0x2000000000
```

## val EXT_BUFFER_DEVICE_ADDRESS

```mach
pub val EXT_BUFFER_DEVICE_ADDRESS:            u64 = 0x4000000000
```

## val EXT_BUFFER_FLOAT16_ATOMICS

```mach
pub val EXT_BUFFER_FLOAT16_ATOMICS:           u64 = 0x20000000000
```

## val EXT_BUFFER_FLOAT16_ATOMIC_ADD

```mach
pub val EXT_BUFFER_FLOAT16_ATOMIC_ADD:        u64 = 0x40000000000
```

## val EXT_BUFFER_FLOAT16_ATOMIC_MIN_MAX

```mach
pub val EXT_BUFFER_FLOAT16_ATOMIC_MIN_MAX:    u64 = 0x80000000000
```

## val EXT_SHARED_FLOAT16_ATOMICS

```mach
pub val EXT_SHARED_FLOAT16_ATOMICS:           u64 = 0x100000000000
```

## val EXT_SHARED_FLOAT16_ATOMIC_ADD

```mach
pub val EXT_SHARED_FLOAT16_ATOMIC_ADD:        u64 = 0x200000000000
```

## val EXT_SHARED_FLOAT16_ATOMIC_MIN_MAX

```mach
pub val EXT_SHARED_FLOAT16_ATOMIC_MIN_MAX:    u64 = 0x400000000000
```

## val EXT_STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val EXT_STORAGE_BUFFER_16BIT_ACCESS:             u64 = 0x800000000000
```

## val EXT_UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS

```mach
pub val EXT_UNIFORM_AND_STORAGE_BUFFER_16BIT_ACCESS: u64 = 0x1000000000000
```

## val EXT_STORAGE_PUSH_CONSTANT16

```mach
pub val EXT_STORAGE_PUSH_CONSTANT16:                 u64 = 0x2000000000000
```

## val EXT_STORAGE_INPUT_OUTPUT16

```mach
pub val EXT_STORAGE_INPUT_OUTPUT16:                  u64 = 0x4000000000000
```

## val EXT_STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val EXT_STORAGE_BUFFER_8BIT_ACCESS:              u64 = 0x8000000000000
```

## val EXT_UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS

```mach
pub val EXT_UNIFORM_AND_STORAGE_BUFFER_8BIT_ACCESS:  u64 = 0x10000000000000
```

## val EXT_STORAGE_PUSH_CONSTANT8

```mach
pub val EXT_STORAGE_PUSH_CONSTANT8:                  u64 = 0x20000000000000
```

## val EXTENSION_COUNT

```mach
pub val EXTENSION_COUNT: u32 = 54
```

## val EXTENSIONS

```mach
pub val EXTENSIONS: [EXTENSION_COUNT]extension.Extension = [EXTENSION_COUNT]extension.Extension;
```

## val EXT_OPEN

```mach
pub val EXT_OPEN: u64 = EXT_FLOAT16 | EXT_ZERO_INIT_WORKGROUP | EXT_SUBGROUP_ARITHMETIC | EXT_SUBGROUP_CLUSTERED
| EXT_STORAGE_READ_WITHOUT_FORMAT | EXT_STORAGE_WRITE_WITHOUT_FORMAT
| EXT_SUBGROUP_VOTE | EXT_SUBGROUP_BALLOT | EXT_SUBGROUP_SHUFFLE | EXT_SUBGROUP_SHUFFLE_RELATIVE | EXT_SUBGROUP_QUAD
| EXT_SUBGROUP_GRAPHICS_STAGES | EXT_ATOMICS | EXT_STORAGE_IMAGE_MULTISAMPLE | EXT_RESOURCE_MIN_LOD
| EXT_IMAGE_GATHER_EXTENDED | EXT_MAINTENANCE8 | EXT_VULKAN_MEMORY_MODEL | EXT_VULKAN_MEMORY_MODEL_DEVICE_SCOPE
| EXT_INT8 | EXT_INT16 | EXT_BUFFER_DEVICE_ADDRESS | EXT_INT64 | EXT_FLOAT64 | EXT_STORAGE_ACCESS
```

every extension of the vocabulary, which a module naming no environment holds

## fun env_extensions

```mach
pub fun env_extensions(id: u32) u64;
```

the extensions an environment guarantees: from vulkan1.3 zero_init_workgroup, where
the feature is core, the two without-format features, whose capabilities it accepts
with none enabled, and the two memory model features and buffer device address, which
vulkan1.3 requires of every device. no version guarantees a type's feature, which its
ceiling only admits

## fun env_profile

```mach
pub fun env_profile(id: u32) *EnvProfile;
```

## fun env_profile_name

```mach
pub fun env_profile_name(id: u32) str;
```

## fun env_name_from

```mach
pub fun env_name_from(version: u32) str;
```

the first environment whose version reaches `version`

## fun need_name

```mach
pub fun need_name(bit: u64) str;
```

## val EXEC_MODEL_VERTEX

```mach
pub val EXEC_MODEL_VERTEX:    u32 = 0
```

## val EXEC_MODEL_FRAGMENT

```mach
pub val EXEC_MODEL_FRAGMENT:  u32 = 4
```

## val EXEC_MODEL_GLCOMPUTE

```mach
pub val EXEC_MODEL_GLCOMPUTE: u32 = 5
```

## val EXEC_MODE_ORIGIN_UPPER_LEFT

```mach
pub val EXEC_MODE_ORIGIN_UPPER_LEFT: u32 = 7
```

## val EXEC_MODE_LOCAL_SIZE

```mach
pub val EXEC_MODE_LOCAL_SIZE:        u32 = 17
```

## val EXEC_MODE_LOCAL_SIZE_ID

```mach
pub val EXEC_MODE_LOCAL_SIZE_ID:     u32 = 38
```

## val ADDRESSING_LOGICAL

```mach
pub val ADDRESSING_LOGICAL:                    u32 = 0
```

## val ADDRESSING_PHYSICAL_STORAGE_BUFFER_64

```mach
pub val ADDRESSING_PHYSICAL_STORAGE_BUFFER_64: u32 = 5348
```

## val DIM_1D

```mach
pub val DIM_1D:     u32 = 0
```

## val DIM_2D

```mach
pub val DIM_2D:     u32 = 1
```

## val DIM_3D

```mach
pub val DIM_3D:     u32 = 2
```

## val DIM_CUBE

```mach
pub val DIM_CUBE:   u32 = 3
```

## val DIM_BUFFER

```mach
pub val DIM_BUFFER: u32 = 5
```

## val IMAGE_FORMAT_UNKNOWN

```mach
pub val IMAGE_FORMAT_UNKNOWN: u32 = 0
```

## val IMAGE_FORMAT_LAST_FLOAT

```mach
pub val IMAGE_FORMAT_LAST_FLOAT: u32 = 20
```

the last format of each texel class: float formats, then signed, then unsigned integer

## val IMAGE_FORMAT_LAST_SINT

```mach
pub val IMAGE_FORMAT_LAST_SINT:  u32 = 29
```

## val IMAGE_FORMAT_LAST_UINT

```mach
pub val IMAGE_FORMAT_LAST_UINT:  u32 = 39
```

## val IMAGE_FORMAT_R32F

```mach
pub val IMAGE_FORMAT_R32F:  u32 = 3
```

the formats an image atomic is defined on, the 64-bit ones under Int64ImageEXT

## val IMAGE_FORMAT_R32I

```mach
pub val IMAGE_FORMAT_R32I:  u32 = 24
```

## val IMAGE_FORMAT_R32UI

```mach
pub val IMAGE_FORMAT_R32UI: u32 = 33
```

## val IMAGE_FORMAT_R64UI

```mach
pub val IMAGE_FORMAT_R64UI: u32 = 40
```

## val IMAGE_FORMAT_R64I

```mach
pub val IMAGE_FORMAT_R64I:  u32 = 41
```

## val IMAGE_SAMPLED_YES

```mach
pub val IMAGE_SAMPLED_YES:     u32 = 1
```

## val IMAGE_SAMPLED_STORAGE

```mach
pub val IMAGE_SAMPLED_STORAGE: u32 = 2
```

## fun image_format_is_base

```mach
pub fun image_format_is_base(fmt: u32) bool;
```

a format the Shader capability declares: Rgba32f, Rgba16f, R32f, Rgba8 and Rgba8Snorm,
their Rgba32/Rgba16/Rgba8/R32 signed and unsigned integer forms. every other format
needs StorageImageExtendedFormats

## val MEMORY_GLSL450

```mach
pub val MEMORY_GLSL450: u32 = 1
```

## val MEMORY_VULKAN

```mach
pub val MEMORY_VULKAN:  u32 = 3
```

## fun vulkan_memory_model

```mach
pub fun vulkan_memory_model(extensions: u64) bool;
```

the Vulkan memory model is selected where the target holds its feature

## val STORAGE_FUNCTION

```mach
pub val STORAGE_FUNCTION: u32 = 7
```

## val STORAGE_INPUT

```mach
pub val STORAGE_INPUT:   u32 = 1
```

## val STORAGE_UNIFORM

```mach
pub val STORAGE_UNIFORM: u32 = 2
```

## val STORAGE_OUTPUT

```mach
pub val STORAGE_OUTPUT:  u32 = 3
```

## val STORAGE_WORKGROUP

```mach
pub val STORAGE_WORKGROUP: u32 = 4
```

## val STORAGE_PUSH_CONSTANT

```mach
pub val STORAGE_PUSH_CONSTANT: u32 = 9
```

## val STORAGE_STORAGE_BUFFER

```mach
pub val STORAGE_STORAGE_BUFFER: u32 = 12
```

## val STORAGE_IMAGE

```mach
pub val STORAGE_IMAGE: u32 = 11
```

## val STORAGE_UNIFORM_CONSTANT

```mach
pub val STORAGE_UNIFORM_CONSTANT: u32 = 0
```

## val STORAGE_PHYSICAL_STORAGE_BUFFER

```mach
pub val STORAGE_PHYSICAL_STORAGE_BUFFER: u32 = 5349
```

## val DECOR_BLOCK

```mach
pub val DECOR_BLOCK:          u32 = 2
```

## val DECOR_BUILTIN

```mach
pub val DECOR_BUILTIN:        u32 = 11
```

## val DECOR_LOCATION

```mach
pub val DECOR_LOCATION:       u32 = 30
```

## val DECOR_BINDING

```mach
pub val DECOR_BINDING:        u32 = 33
```

## val DECOR_DESCRIPTOR_SET

```mach
pub val DECOR_DESCRIPTOR_SET: u32 = 34
```

## val DECOR_OFFSET

```mach
pub val DECOR_OFFSET:         u32 = 35
```

## val DECOR_ARRAY_STRIDE

```mach
pub val DECOR_ARRAY_STRIDE:   u32 = 6
```

## val DECOR_COHERENT

```mach
pub val DECOR_COHERENT:     u32 = 23
```

## val DECOR_NON_WRITABLE

```mach
pub val DECOR_NON_WRITABLE: u32 = 24
```

## val DECOR_NON_READABLE

```mach
pub val DECOR_NON_READABLE: u32 = 25
```

## val DECOR_ALIASED

```mach
pub val DECOR_ALIASED:         u32 = 20
```

## val DECOR_ALIASED_POINTER

```mach
pub val DECOR_ALIASED_POINTER: u32 = 5356
```

## val DECOR_FLAT

```mach
pub val DECOR_FLAT: u32 = 14
```

## val DECOR_SPEC_ID

```mach
pub val DECOR_SPEC_ID: u32 = 1
```

## val BUILTIN_POSITION

```mach
pub val BUILTIN_POSITION:               u32 = 0
```

## val BUILTIN_POINT_SIZE

```mach
pub val BUILTIN_POINT_SIZE:             u32 = 1
```

## val BUILTIN_FRAG_COORD

```mach
pub val BUILTIN_FRAG_COORD:             u32 = 15
```

## val BUILTIN_NUM_WORKGROUPS

```mach
pub val BUILTIN_NUM_WORKGROUPS:         u32 = 24
```

## val BUILTIN_WORKGROUP_SIZE

```mach
pub val BUILTIN_WORKGROUP_SIZE:         u32 = 25
```

## val BUILTIN_WORKGROUP_ID

```mach
pub val BUILTIN_WORKGROUP_ID:           u32 = 26
```

## val BUILTIN_LOCAL_INVOCATION_ID

```mach
pub val BUILTIN_LOCAL_INVOCATION_ID:    u32 = 27
```

## val BUILTIN_GLOBAL_INVOCATION_ID

```mach
pub val BUILTIN_GLOBAL_INVOCATION_ID:   u32 = 28
```

## val BUILTIN_LOCAL_INVOCATION_INDEX

```mach
pub val BUILTIN_LOCAL_INVOCATION_INDEX: u32 = 29
```

## val BUILTIN_VERTEX_INDEX

```mach
pub val BUILTIN_VERTEX_INDEX:           u32 = 42
```

## val BUILTIN_INSTANCE_INDEX

```mach
pub val BUILTIN_INSTANCE_INDEX:         u32 = 43
```

## val BUILTIN_SUBGROUP_SIZE

```mach
pub val BUILTIN_SUBGROUP_SIZE:                u32 = 36
```

## val BUILTIN_NUM_SUBGROUPS

```mach
pub val BUILTIN_NUM_SUBGROUPS:                u32 = 38
```

## val BUILTIN_SUBGROUP_ID

```mach
pub val BUILTIN_SUBGROUP_ID:                  u32 = 40
```

## val BUILTIN_SUBGROUP_LOCAL_INVOCATION_ID

```mach
pub val BUILTIN_SUBGROUP_LOCAL_INVOCATION_ID: u32 = 41
```

## val DECOR_LINKAGE_ATTRIBUTES

```mach
pub val DECOR_LINKAGE_ATTRIBUTES: u32 = 41
```

## val LINKAGE_EXPORT

```mach
pub val LINKAGE_EXPORT: u32 = 0
```

## val FUNCTION_CONTROL_NONE

```mach
pub val FUNCTION_CONTROL_NONE: u32 = 0
```

## val LOOP_CONTROL_NONE

```mach
pub val LOOP_CONTROL_NONE: u32 = 0
```

## val SELECTION_CONTROL_NONE

```mach
pub val SELECTION_CONTROL_NONE: u32 = 0
```

## val SCOPE_DEVICE

```mach
pub val SCOPE_DEVICE:       u32 = 1
```

## val SCOPE_WORKGROUP

```mach
pub val SCOPE_WORKGROUP:    u32 = 2
```

## val SCOPE_QUEUE_FAMILY

```mach
pub val SCOPE_QUEUE_FAMILY: u32 = 5
```

## val SEMANTICS_ACQUIRE_RELEASE

```mach
pub val SEMANTICS_ACQUIRE_RELEASE:  u32 = 0x8
```

## val SEMANTICS_SEQ_CST

```mach
pub val SEMANTICS_SEQ_CST:          u32 = 0x10
```

## val SEMANTICS_WORKGROUP_MEMORY

```mach
pub val SEMANTICS_WORKGROUP_MEMORY: u32 = 0x100
```

## val SEMANTICS_VULKAN_ONLY

```mach
pub val SEMANTICS_VULKAN_ONLY: u32 = SEMANTICS_MAKE_AVAILABLE | SEMANTICS_MAKE_VISIBLE | SEMANTICS_VOLATILE
```

the semantics bits only the Vulkan memory model defines

## val MEMORY_ACCESS_VOLATILE

```mach
pub val MEMORY_ACCESS_VOLATILE:               u32 = 0x1
```

the Memory Operands of OpLoad and OpStore. Aligned is followed by its literal, the
availability and visibility bits by their scope id, each needing NonPrivatePointer beside
it, and the operands follow the bits in their order

## val MEMORY_ACCESS_ALIGNED

```mach
pub val MEMORY_ACCESS_ALIGNED:                u32 = 0x2
```

## val MEMORY_ACCESS_MAKE_POINTER_AVAILABLE

```mach
pub val MEMORY_ACCESS_MAKE_POINTER_AVAILABLE: u32 = 0x8
```

## val MEMORY_ACCESS_MAKE_POINTER_VISIBLE

```mach
pub val MEMORY_ACCESS_MAKE_POINTER_VISIBLE:   u32 = 0x10
```

## val MEMORY_ACCESS_NON_PRIVATE_POINTER

```mach
pub val MEMORY_ACCESS_NON_PRIVATE_POINTER:    u32 = 0x20
```

## rec Builder

```mach
pub rec Builder;
```

## fun builder_init

```mach
pub fun builder_init(alloc: *A.Allocator) Builder;
```

## fun builder_dnit

```mach
pub fun builder_dnit(b: *Builder);
```

## fun alloc_id

```mach
pub fun alloc_id(b: *Builder) u32;
```

## fun inst_word

```mach
pub fun inst_word(opcode: u32, words: u32) u32;
```

## fun inst

```mach
pub fun inst(b: *Builder, v: *Vec, opcode: u32, ops: *u32, n: u32);
```

## fun ext_inst_import

```mach
pub fun ext_inst_import(b: *Builder, set: str) u32;
```

## fun inst_string

```mach
pub fun inst_string(b: *Builder, v: *Vec, opcode: u32, lead: *u32, lead_n: u32, s: str);
```

an instruction of lead operands followed by one literal string, the shape of OpString,
OpName and OpMemberName

## fun string_words

```mach
pub fun string_words(s: str) res[u32, fail.Fail];
```

## fun pack_string

```mach
pub fun pack_string(s: str, words: u32, out: *u32);
```

## fun serialize

```mach
pub fun serialize(b: *Builder, out_len: *u32) res[*u8, fail.Fail];
```

