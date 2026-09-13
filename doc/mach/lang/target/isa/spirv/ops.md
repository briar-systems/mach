# mach.lang.target.isa.spirv.ops

## rec CoreOp

```mach
pub rec CoreOp;
```

the core value instructions the emitter forms from a MIR opcode: one row
per SPIR-V opcode with the id operands it takes after `<result type>
<result id>`. every row carries a result type and a result id, so an
instruction's word count is a table read and never a literal beside a
call. the structural instructions (loads, stores, composites, control
flow) take variable or context-shaped operands and are formed where their
shape is decided

## fun core

```mach
pub fun core(opcode: u32) *CoreOp;
```

the row of a core value opcode; the blank row for an opcode outside the
table, whose arity of 0 no emitter shape accepts

## fun operand_words

```mach
pub fun operand_words(row: *CoreOp) u32;
```

the operand words of a core value instruction: result type, result id and
the row's operands. the encoded word count is one more

## def Shape

```mach
pub def Shape: u8
```

how a MIR opcode reaches SPIR-V: the emitter shape that forms the
instruction and the opcode for each lane class. an opcode with no float
pairing names the same opcode in both columns

## val SH_NONE

```mach
pub val SH_NONE: Shape = 0
```

## val SH_BINARY

```mach
pub val SH_BINARY: Shape = 1
```

<type> <result> a b, result typed as the destination

## val SH_UNARY

```mach
pub val SH_UNARY: Shape = 2
```

<type> <result> a, result typed as the destination

## val SH_COMPARE

```mach
pub val SH_COMPARE: Shape = 3
```

<bool> <result> a b, operands at the comparison width

## val SH_CONVERT

```mach
pub val SH_CONVERT: Shape = 4
```

<type> <result> a, the source typed from `src_float` when its operand
carries no type of its own

## rec MirMap

```mach
pub rec MirMap;
```

## fun map_of

```mach
pub fun map_of(op: mir.MirOpcode) *MirMap;
```

the mapping row of a MIR opcode, nil when the value emitter has none (the
structural instructions are formed by their own emitters)

## fun select

```mach
pub fun select(row: *MirMap, lane_float: bool) u32;
```

the opcode of a mapped MIR instruction for its lane class

## fun float_compare

```mach
pub fun float_compare(cc: u8) u32;
```

the ordered float comparisons the FCMP condition codes name

## fun shape_arity

```mach
pub fun shape_arity(shape: Shape) opt[u8];
```

the arity an emitter shape forms, 0 for the shape that forms none; absent
for a tag outside the catalog

