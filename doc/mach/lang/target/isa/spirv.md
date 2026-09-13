# mach.lang.target.isa.spirv

## val SPV_MAGIC

```mach
pub val SPV_MAGIC: u32 = 0x07230203
```

## val SPV_VERSION_1_0

```mach
pub val SPV_VERSION_1_0: u32 = 0x00010000
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

## val OP_NAME

```mach
pub val OP_NAME:                      u32 = 5
```

## val OP_EXT_INST_IMPORT

```mach
pub val OP_EXT_INST_IMPORT:           u32 = 11
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

## val OP_ACCESS_CHAIN

```mach
pub val OP_ACCESS_CHAIN:              u32 = 65
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

## val CAP_SAMPLED_1D

```mach
pub val CAP_SAMPLED_1D:         u32 = 43
```

## val CAP_SAMPLED_CUBE_ARRAY

```mach
pub val CAP_SAMPLED_CUBE_ARRAY: u32 = 45
```

## val NEED_INT8

```mach
pub val NEED_INT8:               u32 = 0x01
```

## val NEED_INT16

```mach
pub val NEED_INT16:              u32 = 0x02
```

## val NEED_INT64

```mach
pub val NEED_INT64:              u32 = 0x04
```

## val NEED_FLOAT16

```mach
pub val NEED_FLOAT16:            u32 = 0x08
```

## val NEED_FLOAT64

```mach
pub val NEED_FLOAT64:            u32 = 0x10
```

## val NEED_SAMPLED_1D

```mach
pub val NEED_SAMPLED_1D:         u32 = 0x20
```

## val NEED_SAMPLED_CUBE_ARRAY

```mach
pub val NEED_SAMPLED_CUBE_ARRAY: u32 = 0x40
```

## fun env_profile

```mach
pub fun env_profile(id: u32) *EnvProfile;
```

## fun env_profile_name

```mach
pub fun env_profile_name(id: u32) str;
```

## fun need_name

```mach
pub fun need_name(bit: u32) str;
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

## val ADDRESSING_LOGICAL

```mach
pub val ADDRESSING_LOGICAL: u32 = 0
```

## val DIM_1D

```mach
pub val DIM_1D:           u32 = 0
```

## val DIM_2D

```mach
pub val DIM_2D:           u32 = 1
```

## val DIM_3D

```mach
pub val DIM_3D:           u32 = 2
```

## val DIM_CUBE

```mach
pub val DIM_CUBE:         u32 = 3
```

## val DIM_RECT

```mach
pub val DIM_RECT:         u32 = 4
```

## val DIM_BUFFER

```mach
pub val DIM_BUFFER:       u32 = 5
```

## val DIM_SUBPASS_DATA

```mach
pub val DIM_SUBPASS_DATA: u32 = 6
```

## val IMAGE_FORMAT_UNKNOWN

```mach
pub val IMAGE_FORMAT_UNKNOWN: u32 = 0
```

## val IMAGE_SAMPLED_YES

```mach
pub val IMAGE_SAMPLED_YES: u32 = 1
```

## val MEMORY_GLSL450

```mach
pub val MEMORY_GLSL450: u32 = 1
```

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

## val STORAGE_STORAGE_BUFFER

```mach
pub val STORAGE_STORAGE_BUFFER: u32 = 12
```

## val STORAGE_UNIFORM_CONSTANT

```mach
pub val STORAGE_UNIFORM_CONSTANT: u32 = 0
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

## val DECOR_NON_WRITABLE

```mach
pub val DECOR_NON_WRITABLE: u32 = 24
```

## val DECOR_FLAT

```mach
pub val DECOR_FLAT: u32 = 14
```

## val BUILTIN_POSITION

```mach
pub val BUILTIN_POSITION:             u32 = 0
```

## val BUILTIN_POINT_SIZE

```mach
pub val BUILTIN_POINT_SIZE:           u32 = 1
```

## val BUILTIN_FRAG_COORD

```mach
pub val BUILTIN_FRAG_COORD:           u32 = 15
```

## val BUILTIN_WORKGROUP_ID

```mach
pub val BUILTIN_WORKGROUP_ID:         u32 = 26
```

## val BUILTIN_LOCAL_INVOCATION_ID

```mach
pub val BUILTIN_LOCAL_INVOCATION_ID:  u32 = 27
```

## val BUILTIN_GLOBAL_INVOCATION_ID

```mach
pub val BUILTIN_GLOBAL_INVOCATION_ID: u32 = 28
```

## val BUILTIN_VERTEX_INDEX

```mach
pub val BUILTIN_VERTEX_INDEX:         u32 = 42
```

## val BUILTIN_INSTANCE_INDEX

```mach
pub val BUILTIN_INSTANCE_INDEX:       u32 = 43
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

## rec Builder

```mach
pub rec Builder;
```

## rec Vec

```mach
pub rec Vec;
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

