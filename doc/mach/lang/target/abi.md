# mach.lang.target.abi

## def ParamClass

```mach
pub def ParamClass: u8
```

## val CLASS_GP

```mach
pub val CLASS_GP:              ParamClass = 0
```

## val CLASS_FP

```mach
pub val CLASS_FP:              ParamClass = 1
```

## val CLASS_STACK

```mach
pub val CLASS_STACK:           ParamClass = 2
```

## val CLASS_SRET

```mach
pub val CLASS_SRET:            ParamClass = 3
```

## val CLASS_BYREF

```mach
pub val CLASS_BYREF:           ParamClass = 4
```

## val CLASS_STACK_BYREF

```mach
pub val CLASS_STACK_BYREF:     ParamClass = 5
```

## val CLASS_VEC_BYREF

```mach
pub val CLASS_VEC_BYREF:       ParamClass = 6
```

## val CLASS_VEC_STACK_BYREF

```mach
pub val CLASS_VEC_STACK_BYREF: ParamClass = 7
```

## val CLASS_VALUE

```mach
pub val CLASS_VALUE:           ParamClass = 8
```

## def PieceKind

```mach
pub def PieceKind: u8
```

## val PIECE_GP

```mach
pub val PIECE_GP:    PieceKind = 0
```

## val PIECE_FP

```mach
pub val PIECE_FP:    PieceKind = 1
```

## val PIECE_STACK

```mach
pub val PIECE_STACK: PieceKind = 2
```

## val PARAM_MAX_PIECES

```mach
pub val PARAM_MAX_PIECES: u32 = 4
```

## rec ParamPiece

```mach
pub rec ParamPiece;
```

## rec AggField

```mach
pub rec AggField;
```

a small aggregate's scalar leaves, as a convention that passes one field by
field reads them

## rec AggLayout

```mach
pub rec AggLayout;
```

## fun agg_none

```mach
pub fun agg_none() AggLayout;
```

## def TypeShape

```mach
pub def TypeShape: u8
```

what a type is, as every convention reads it. a convention derives its own
aggregate facts (eightbyte classes, homogeneous members, leaves) by walking
a type through a TypeView; the code generator only describes the type

## val SHAPE_NONE

```mach
pub val SHAPE_NONE: TypeShape = 0
```

a type the view cannot describe, which no walk descends into

## val SHAPE_INT

```mach
pub val SHAPE_INT:   TypeShape = 1
```

an integer or pointer scalar

## val SHAPE_FLOAT

```mach
pub val SHAPE_FLOAT: TypeShape = 2
```

## val SHAPE_HALF

```mach
pub val SHAPE_HALF:   TypeShape = 3
```

an integer carrying a binary16, which the convention's half rule places

## val SHAPE_VECTOR

```mach
pub val SHAPE_VECTOR: TypeShape = 4
```

## val SHAPE_RECORD

```mach
pub val SHAPE_RECORD: TypeShape = 5
```

members at their own offsets

## val SHAPE_ARRAY

```mach
pub val SHAPE_ARRAY: TypeShape = 6
```

`count` elements of one type

## val SHAPE_UNION

```mach
pub val SHAPE_UNION: TypeShape = 7
```

members all at offset 0

## val SHAPE_TAG

```mach
pub val SHAPE_TAG: TypeShape = 8
```

a discriminator of `disc_bytes` at offset 0, then `count` cases at one payload offset

## rec TypeFacts

```mach
pub rec TypeFacts;
```

## rec TypeMember

```mach
pub rec TypeMember;
```

member `i` of a type and its offset from the type's start

## rec TypeView

```mach
pub rec TypeView;
```

a read-only view of the program's types, handed to a classifier with the
type it places; `state` is the describer's own

## fun type_facts

```mach
pub fun type_facts(view: *TypeView, ty: u32) TypeFacts;
```

## fun type_member

```mach
pub fun type_member(view: *TypeView, ty: u32, i: u32) TypeMember;
```

## fun shape_is_float

```mach
pub fun shape_is_float(vt: *AbiVTable, shape: TypeShape) bool;
```

a type the convention places as a float: a float, or a binary16 where the
convention's half row takes the float bank

## fun arg_is_float

```mach
pub fun arg_is_float(vt: *AbiVTable, q: *ArgQuery) bool;
```

an argument the convention places as a float. an unnamed one is a float
only where the variadic save model carries a float of its width

## fun ret_is_float

```mach
pub fun ret_is_float(vt: *AbiVTable, q: *RetQuery) bool;
```

## fun arg_members_read

```mach
pub fun arg_members_read(q: *ArgQuery) bool;
```

a named aggregate argument is placed by its members; an unnamed one by its
size alone

## fun homogeneous_float

```mach
pub fun homogeneous_float(vt: *AbiVTable, view: *TypeView, ty: u32, out_members: *u8, out_elem: *u8);
```

a homogeneous float aggregate of one to four members of two, four or eight
bytes: its member count and width, both 0 for any other type

## rec ParamSlot

```mach
pub rec ParamSlot;
```

reg is the physical register the slot rides, MIR_PREG_NIL for a stack slot;
the constructors take the isa regid the packs select (-1 for none).
gp_consumed and fp_consumed are how many general and floating-point
registers the argument takes out of each sequence, which the constructors set
to the pieces it rides in each bank; a classifier raises them where its ABI
skips or exhausts registers without using them (AAPCS64 C.10 rounds a
double-word to an even register, C.11 and C.13 give the remaining general
registers to nothing once an argument that wanted them goes to the stack, and
C.3 does the same for the vector registers after an HFA or HVA)

## rec ArgQuery

```mach
pub rec ArgQuery;
```

one argument a convention places. align is the alignment the register
assignment honors: the type's own, capped where the platform relaxes it
(darwin aarch64 starts a 16-byte value in any x register). vec_bytes is the
widest vector one register of the target carries under its selected
extensions (isa.vector_register_bytes), which a convention that places a
vector by its register width reads (System V's ymm under avx)

## rec RetQuery

```mach
pub rec RetQuery;
```

the result a convention places, described as an argument is

## def ArgPassingFn

```mach
pub def ArgPassingFn: fun(*AbiVTable, *ArgQuery) ParamSlot
```

a classifier reads the descriptor it is called through, so one classifier
serves every member of a family, each member's parameters in `family`

## def RetPassingFn

```mach
pub def RetPassingFn: fun(*AbiVTable, *RetQuery) ParamSlot
```

## val HALF_FLOAT

```mach
pub val HALF_FLOAT:        HalfRule = 1
```

## val HALF_FLOAT_NANBOX

```mach
pub val HALF_FLOAT_NANBOX: HalfRule = 2
```

## fun half_is_float

```mach
pub fun half_is_float(rule: HalfRule) bool;
```

a binary16 takes the float bank under this rule

## fun half_float_padding

```mach
pub fun half_float_padding(rule: HalfRule) u16;
```

the bits above a binary16 in its four-byte float carrier. a register wider
than four bytes is boxed by the four-byte load itself, as a binary32 is

## fun piece_is_half

```mach
pub fun piece_is_half(p: *ParamPiece) bool;
```

a piece of a float register two bytes wide carries a binary16: no other
float or vector a convention places is that narrow

## rec AbiVTable

```mach
pub rec AbiVTable;
```

## rec SigLayout

```mach
pub rec SigLayout;
```

## val VA_MODEL_REG_SAVE

```mach
pub val VA_MODEL_REG_SAVE: u8 = 0
```

## val VA_MODEL_HOME

```mach
pub val VA_MODEL_HOME:     u8 = 1
```

## rec VaModel

```mach
pub rec VaModel;
```

## fun va_model_make

```mach
pub fun va_model_make(kind: u8, float_dup_gp: bool, vector_count_reg: i32,
variadic_float_bits: u32) VaModel;
```

## fun variadic_float_in_fp_bank

```mach
pub fun variadic_float_in_fp_bank(m: *VaModel, width: u64) bool;
```

## fun validate

```mach
pub fun validate(a: *A.Allocator, vt: *AbiVTable) err[fail.Fail];
```

why a descriptor is malformed, read once when a registry adds it

a: formats the refusal
vt: the descriptor

## fun name_of

```mach
pub fun name_of(vt: *AbiVTable) str;
```

## fun id_of

```mach
pub fun id_of(vt: *AbiVTable) u32;
```

## fun covers_isa

```mach
pub fun covers_isa(vt: *AbiVTable, arch_id: u32) bool;
```

## fun make_slot

```mach
pub fun make_slot(class: ParamClass, reg: i32, offset: i64, size: u64, carrier_width: u8) ParamSlot;
```

a slot of a direct class. total over the classes a classifier names: an
indirect vector class needs make_slot_indirect, and passing one is a caller defect

## fun make_slot_indirect

```mach
pub fun make_slot_indirect(class: ParamClass, reg: i32, offset: i64, size: u64,
indirect_size: u64, indirect_align: u32) ParamSlot;
```

a slot of an indirect vector class with its storage geometry. total over a
declared geometry: a size, and an alignment that is a power of two

## fun make_slot_value

```mach
pub fun make_slot_value(size: u64) ParamSlot;
```

## fun make_slot_pair

```mach
pub fun make_slot_pair(class: ParamClass, reg: i32, reg2: i32, offset: i64, size: u64,
word: u32) ParamSlot;
```

## fun make_slot_hfa

```mach
pub fun make_slot_hfa(base_reg: i32, count: u8, elem: u8, size: u64) ParamSlot;
```

a homogeneous float aggregate in `count` consecutive float registers. total
over the slot's fixed piece storage: more than PARAM_MAX_PIECES is a caller defect

## fun make_piece

```mach
pub fun make_piece(kind: PieceKind, reg: i32, src_off: i64, stk_off: i64, width: u8) ParamPiece;
```

## fun make_slot_pieces

```mach
pub fun make_slot_pieces(class: ParamClass, pieces: *ParamPiece, count: u32, size: u64) ParamSlot;
```

a slot carried in `count` pieces. total over the slot's fixed piece storage:
more than PARAM_MAX_PIECES is a caller defect

## fun piece_memory_width

```mach
pub fun piece_memory_width(slot: *ParamSlot, piece: *ParamPiece) res[u8, fail.Fail];
```

## fun carrier_storage_extent

```mach
pub fun carrier_storage_extent(slot: *ParamSlot) res[u64, fail.Fail];
```

## fun carrier_storage_alignment

```mach
pub fun carrier_storage_alignment(slot: *ParamSlot) res[u32, fail.Fail];
```

