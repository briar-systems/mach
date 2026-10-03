# mach.lang.target.definition

the operation and type tables an instruction set declares for `#[op]`
intrinsics and target handle types, as plain data

## val NO_OP_SET

```mach
pub val NO_OP_SET: u32 = 0
```

## val NO_OPCODE

```mach
pub val NO_OPCODE: u32 = 0xFFFFFFFF
```

## def OpOperandKind

```mach
pub def OpOperandKind: u8
```

how one operand of an `#[op]` instruction is written: an ordinary value id, an
id that must be an integer constant by emission, a literal word inline, a
pointer whose storage class the call site's access chain decides, which the
instruction only reads through, only stores through, or reads and writes, a
handle value whose memory the instruction reads or writes (a handle passed as
an ordinary value names its descriptor and touches none of its memory), or a
truth value the target writes as its boolean type, from an integer argument
whose nonzero is true. a memory scope or a memory semantics operand is a
constant id too, one the target reads by value and holds to what that value
needs under its memory model

## val OP_OPERAND_VALUE

```mach
pub val OP_OPERAND_VALUE:          OpOperandKind = 0
```

## val OP_OPERAND_CONSTANT

```mach
pub val OP_OPERAND_CONSTANT:       OpOperandKind = 1
```

## val OP_OPERAND_LITERAL

```mach
pub val OP_OPERAND_LITERAL:        OpOperandKind = 2
```

## val OP_OPERAND_POINTER_WRITE

```mach
pub val OP_OPERAND_POINTER_WRITE:  OpOperandKind = 4
```

## val OP_OPERAND_POINTER_UPDATE

```mach
pub val OP_OPERAND_POINTER_UPDATE: OpOperandKind = 5
```

## val OP_OPERAND_HANDLE_READ

```mach
pub val OP_OPERAND_HANDLE_READ:    OpOperandKind = 6
```

## val OP_OPERAND_HANDLE_WRITE

```mach
pub val OP_OPERAND_HANDLE_WRITE:   OpOperandKind = 7
```

## val OP_OPERAND_BOOL

```mach
pub val OP_OPERAND_BOOL:           OpOperandKind = 8
```

## val OP_OPERAND_SCOPE

```mach
pub val OP_OPERAND_SCOPE:          OpOperandKind = 9
```

## val OP_OPERAND_SEMANTICS

```mach
pub val OP_OPERAND_SEMANTICS:      OpOperandKind = 10
```

## def OpResult

```mach
pub def OpResult: u8
```

whether an `#[op]` instruction has a result id, and whether that result is a
pointer into the space the row declares, or a truth value of the target's boolean
type, which the declaration's integer return type receives as 1 or 0

## val OP_RESULT_VALUE

```mach
pub val OP_RESULT_VALUE:   OpResult = 0
```

## val OP_RESULT_NONE

```mach
pub val OP_RESULT_NONE:    OpResult = 1
```

## val OP_RESULT_POINTER

```mach
pub val OP_RESULT_POINTER: OpResult = 2
```

## val OP_RESULT_BOOL

```mach
pub val OP_RESULT_BOOL:    OpResult = 3
```

## val OP_MAX_OPERANDS

```mach
pub val OP_MAX_OPERANDS: u32 = 16
```

## val NO_OP_SPACE

```mach
pub val NO_OP_SPACE: u32 = 0xFFFFFFFF
```

## val NO_OP_CAPABILITY

```mach
pub val NO_OP_CAPABILITY: u32 = 0xFFFFFFFF
```

## val NO_OP_SOURCE

```mach
pub val NO_OP_SOURCE: u32 = 0xFFFFFFFF
```

## rec OpEnumerant

```mach
pub rec OpEnumerant;
```

one value a literal operand may take and what that value brings: the target's
own capability word it needs (NO_OP_CAPABILITY for none), the extensions of the
isa's vocabulary that capability needs selected, and the operands it adds to the end
of the instruction, one letter each as `op_def_shaped` spells them (`""` for none).
what a value brings is never a pointer or a handle: a row's memory effects are known
before a call's literal is

## rec OpEnum

```mach
pub rec OpEnum;
```

the closed set of values a literal operand takes. a value enumeration admits
exactly one of its values. a mask admits any union of its single-bit values,
each set bit bringing its own requirement and operands, which follow in
ascending bit order, and 0 is the empty mask

## def OpDataClass

```mach
pub def OpDataClass: u8
```

the class of the data type a typed row operates on, a vector's element when the typing
admits vectors: an integer, a float, or anything else (an aggregate, a handle), which no
requirement admits

## val OP_DATA_INT

```mach
pub val OP_DATA_INT:   OpDataClass = 0
```

## val OP_DATA_FLOAT

```mach
pub val OP_DATA_FLOAT: OpDataClass = 1
```

## val OP_DATA_OTHER

```mach
pub val OP_DATA_OTHER: OpDataClass = 2
```

## rec OpTypeRequirement

```mach
pub rec OpTypeRequirement;
```

one data type a typed row admits in one of the target's address spaces (NO_OP_SPACE
for every space), and what a use of it there needs: the target's capability word
(NO_OP_CAPABILITY for none) and the extensions of the isa's vocabulary

## rec OpTyping

```mach
pub rec OpTyping;
```

the data types a row operates on, read from its operand `operand` (a pointer's
pointee), and what each needs in each address space. a type no requirement admits in
any space is refused at the declaration, and one admitted only in other spaces at the
call. a typing with no requirements admits every type and needs nothing: it only names
the operand the row's relations are stated against. static data the target owns for
the life of the program, like an OpEnum

## val OP_NUMERIC_NONE

```mach
pub val OP_NUMERIC_NONE:     OpNumeric = 0
```

## val OP_NUMERIC_ANY

```mach
pub val OP_NUMERIC_ANY:      OpNumeric = 1
```

## val OP_NUMERIC_FLOAT

```mach
pub val OP_NUMERIC_FLOAT:    OpNumeric = 2
```

## val OP_NUMERIC_SIGNED

```mach
pub val OP_NUMERIC_SIGNED:   OpNumeric = 3
```

## val OP_NUMERIC_UNSIGNED

```mach
pub val OP_NUMERIC_UNSIGNED: OpNumeric = 4
```

## val OP_NUMERIC_INTEGER

```mach
pub val OP_NUMERIC_INTEGER:  OpNumeric = 5
```

## fun op_numeric_admits

```mach
pub fun op_numeric_admits(n: OpNumeric, class: OpDataClass, signed: bool) bool;
```

whether a scalar of `class`, signed when `signed`, is a number `n` admits

## fun op_numeric_name

```mach
pub fun op_numeric_name(n: OpNumeric) str;
```

the numbers `n` admits, as a refusal names them

## def OpRelation

```mach
pub def OpRelation: u8
```

how an operand's or the result's type relates to the row's data type, the type of its
typing's operand (a pointer operand's pointee): unrelated, that same type, a texel, a
scalar or vector whose component is the sampled type of the handle the typing's operand
is, with as many components as the row's texel count states, for a pointer result
only, a pointer to that sampled type, into the memory of the handle the typing's pointer
operand addresses, or, for a value result only, a handle whose constructor composes over
the handle the typing's operand is, taking its texels from it as a sampled image does
from its image. an operand may also be a sampler, a handle whose constructor states how
texels are sampled, whatever the data type, or shaped like the data type: a scalar or
vector of the scalar the row fixes for it, with as many components as the data type has.
a pointer operand's relation holds its pointee, the type the instruction stores or reads
through it

## val OP_RELATION_NONE

```mach
pub val OP_RELATION_NONE:     OpRelation = 0
```

## val OP_RELATION_DATA

```mach
pub val OP_RELATION_DATA:     OpRelation = 1
```

## val OP_RELATION_TEXEL

```mach
pub val OP_RELATION_TEXEL:    OpRelation = 2
```

## val OP_RELATION_POINTEE

```mach
pub val OP_RELATION_POINTEE:  OpRelation = 3
```

## val OP_RELATION_COMPOSED

```mach
pub val OP_RELATION_COMPOSED: OpRelation = 4
```

## val OP_RELATION_SAMPLER

```mach
pub val OP_RELATION_SAMPLER:  OpRelation = 5
```

## val OP_RELATION_SHAPED

```mach
pub val OP_RELATION_SHAPED:   OpRelation = 6
```

## val OP_COUNT_EXACT

```mach
pub val OP_COUNT_EXACT:      OpCountKind = 0
```

## val OP_COUNT_FORMAT

```mach
pub val OP_COUNT_FORMAT:     OpCountKind = 1
```

## rec OpTexelCount

```mach
pub rec OpTexelCount;
```

the number of components a row's texel has, a scalar being one: `components` exactly,
or as `kind` says. `rule` names the rule a refusal cites. static data the target owns
for the life of the program, like an OpTyping

## rec OpScalar

```mach
pub rec OpScalar;
```

a scalar data type by class, width and signedness, such as a handle's sampled type

## rec OpOperandScalar

```mach
pub rec OpOperandScalar;
```

the one scalar type an operand must be whatever the row's data type, such as the 32-bit
float reference a depth comparison takes. `rule` names the rule a refusal cites. static
data the target owns for the life of the program, like an OpTexelCount

## rec Op

```mach
pub rec Op;
```

## val TYPE_OPERAND_WORD

```mach
pub val TYPE_OPERAND_WORD: u32 = 0xFFFFFFFF
```

## val NO_TYPE_CTOR

```mach
pub val NO_TYPE_CTOR: u32 = 0xFFFFFFFF
```

## val NO_TEXEL_OPERAND

```mach
pub val NO_TEXEL_OPERAND: u32 = 0xFFFFFFFF
```

a constructor whose handles state their own texels, or have none

## val HANDLE_BIND_SAMPLER

```mach
pub val HANDLE_BIND_SAMPLER: u32 = 0
```

a handle bound as a sampled descriptor, `#[sampler(set, binding)]`

## val HANDLE_BIND_STORAGE

```mach
pub val HANDLE_BIND_STORAGE: u32 = 1
```

a handle bound as a storage descriptor, `#[storage(set, binding, ...)]`

## rec Type

```mach
pub rec Type;
```

## rec Table

```mach
pub rec Table;
```

## fun op

```mach
pub fun op(set: str, name: str, set_tag: u32, opcode: u32, arity: u32) Op;
```

a row whose every operand is a value and whose result is a value

## fun op_shaped

```mach
pub fun op_shaped(set: str, name: str, set_tag: u32, opcode: u32,
result: OpResult, result_space: u32, signature: str) Op;
```

a row spelled by its operand signature, one letter per operand in order: `v` a
value, `c` a constant id, `l` a literal word, `r` a pointer only read through, `w`
a pointer only stored through, `u` a pointer read and written (read-modify-write),
`i` a handle whose memory is read, `o` a handle whose memory is written, `b` a
truth value, `s` a memory scope and `m` a memory semantics, each a constant id.
a row spells only its own operands: the ones an enumerated literal's value brings
follow them, typed by the enumerant (`op_enumerated`). a `|` makes the literal after
it optional (`"iv|l"`), a call passing it or leaving it out with every operand its
value would bring, and nothing else follows it.
a letter outside the set is an invalid kind, which registration refuses

## fun op_requiring

```mach
pub fun op_requiring(d: Op, capability: u32, requires: u64) Op;
```

`d` needing the target's capability word `capability` and the extensions `requires` on every use

## fun op_enumerated

```mach
pub fun op_enumerated(d: Op, operand: u32, e: *OpEnum) Op;
```

`d` with its literal operand `operand` held to the values of `e`, and room after the
operands it spells for the most every enumerated literal's value brings

## fun op_typed

```mach
pub fun op_typed(d: Op, t: *OpTyping) Op;
```

`d` operating on the data types `t` admits, each with its own requirement

## fun op_related

```mach
pub fun op_related(d: Op, result: OpRelation, operands: str) Op;
```

`d` with its result's type related to its data type by `result`, and each operand's by
one letter of `operands` in order: `-` unrelated, `=` the data type itself, `t` a texel
of the handle the typing's operand is, `s` a sampler, `n` shaped like the data type in the
scalar `op_scalar_operand` fixes. a letter outside the set is an invalid relation,
which registration refuses

## fun op_confined

```mach
pub fun op_confined(d: Op) Op;
```

`d` with its result consumed only by another row's operand in the block that produces it

## fun op_derived

```mach
pub fun op_derived(d: Op, operand: u32) Op;
```

`d` with its pointer or handle result derived from its operand `operand`: a pointer
result points into what that pointer operand addresses, a handle result holds the
descriptor that operand holds

## fun op_scalar_operand

```mach
pub fun op_scalar_operand(d: Op, operand: u32, s: *OpOperandScalar) Op;
```

`d` with its operand `operand` held to the one scalar type `s` states

## fun op_operand_scalar

```mach
pub fun op_operand_scalar(d: *Op, i: u32) *OpOperandScalar;
```

the scalar type operand `i` of `d` must be, nil when the row fixes none

## fun op_texel_counted

```mach
pub fun op_texel_counted(d: Op, count: *OpTexelCount) Op;
```

`d` with the texel it relates holding `count` components

## fun op_texel_count_admits

```mach
pub fun op_texel_count_admits(count: *OpTexelCount, n: u32, stored: u32) bool;
```

whether `n` components meet `count`, against a handle whose format stores `stored`, 0
when its format is unknown

## fun op_operand_relation

```mach
pub fun op_operand_relation(d: *Op, i: u32) OpRelation;
```

## fun op_typing_constrains

```mach
pub fun op_typing_constrains(t: *OpTyping) bool;
```

whether `t` holds the row's data type to requirements, rather than only naming its operand

## fun op_type_requirement

```mach
pub fun op_type_requirement(t: *OpTyping, class: OpDataClass, bits: u32, space: u32) *OpTypeRequirement;
```

the requirement of `t` admitting a `class` of `bits` in `space`, nil when none does.
`space` NO_OP_SPACE asks whether any space admits the type

## fun op_operand_enum

```mach
pub fun op_operand_enum(d: *Op, i: u32) *OpEnum;
```

## fun op_tail_literal

```mach
pub fun op_tail_literal(d: *Op) *OpEnum;
```

the enumeration of the optional literal that leads `d`'s tail, nil when the tail has none

## fun op_enumerant_brings

```mach
pub fun op_enumerant_brings(v: *OpEnumerant) u32;
```

how many operands the enumerant `v` brings

## fun op_enumerant_of

```mach
pub fun op_enumerant_of(e: *OpEnum, value: u32) *OpEnumerant;
```

the enumerant of `e` whose value is exactly `value`, nil when none is

## val NO_OPERAND

```mach
pub val NO_OPERAND: u32 = 0xFFFFFFFF
```

## fun op_literal_span

```mach
pub fun op_literal_span(d: *Op, argc: u32) u32;
```

how many of a call's leading operands are enumerated-literal positions it passes: the
required operands, and the literal leading the tail when the call passes more than them

## fun op_call_arity

```mach
pub fun op_call_arity(d: *Op, argc: u32, words: *u32, out_bad: *u32) u32;
```

the operand count a call of `d` takes with the literal words `words` it passes:
the operands before its tail, the literal leading the tail when the call passes one,
and every operand an enumerated value brings. `out_bad` receives the first
enumerated operand whose word is no value of its enumeration, else NO_OPERAND

## fun op_call_kinds

```mach
pub fun op_call_kinds(d: *Op, argc: u32, words: *u32, out: *OpOperandKind);
```

the kind of each operand of a call of `d` passing `argc` operands with the literal
words `words`, into `out` (OP_MAX_OPERANDS long): the operands the row spells by the
row, then each operand a literal's value brings by the enumerant that brings it, in
the order of the literals and of each mask's bits. a call `op_call_arity` refuses
has the kinds of the operands it does take, and the rest are values

## fun op_operand_kind

```mach
pub fun op_operand_kind(d: *Op, i: u32) OpOperandKind;
```

the kind of operand `i` as the row spells it. an operand a literal's value brings is
typed by its enumerant (`op_call_kinds`), and is never a pointer or a handle, so it
reads here as a value

## fun op_kind_is_pointer

```mach
pub fun op_kind_is_pointer(k: OpOperandKind) bool;
```

## fun op_kind_is_constant

```mach
pub fun op_kind_is_constant(k: OpOperandKind) bool;
```

an id that must be an integer constant by emission, whatever the target reads its value as

## fun op_kind_is_handle

```mach
pub fun op_kind_is_handle(k: OpOperandKind) bool;
```

## fun op_shape_refusal

```mach
pub fun op_shape_refusal(d: *Op) str;
```

the reason a row's shape is malformed, nil when it is well formed

## fun op_by_code

```mach
pub fun op_by_code(defs: *Table, set_tag: u32, opcode: u32) *Op;
```

## fun type

```mach
pub fun type(name: str, tag: u32, operands: *u32, arity: u32, refuse: TypeRefuseFn) Type;
```

## fun type_binding

```mach
pub fun type_binding(td: *Type, ops: *u32, n: u32) u32;
```

the role a handle of constructor `td` with operands `ops` binds through

## fun type_sampled

```mach
pub fun type_sampled(td: *Type, ops: *u32, n: u32) OpScalar;
```

the scalar texels of a handle of constructor `td` with operands `ops` are, class
OP_DATA_OTHER when the constructor declares none

## fun type_components

```mach
pub fun type_components(td: *Type, ops: *u32, n: u32) u32;
```

the number of components a handle of constructor `td` with operands `ops` stores each
texel in, 0 when its format is unknown or the constructor declares none

## fun type_address

```mach
pub fun type_address(td: *Type, ops: *u32, n: u32) str;
```

why no instruction may derive a pointer into a handle of constructor `td` with operands
`ops`, nil when one may

## fun table

```mach
pub fun table(ops: *Op, op_count: u32, types: *Type, type_count: u32) Table;
```

## fun type_of_region

```mach
pub fun type_of_region(defs: *Table, src: str, off: usize, len: usize) *Type;
```

## fun type_by_tag

```mach
pub fun type_by_tag(defs: *Table, tag: u32) *Type;
```

## fun op_of_region

```mach
pub fun op_of_region(defs: *Table, set_src: str, set_off: usize, set_len: usize,
nam_src: str, nam_off: usize, nam_len: usize) *Op;
```

## fun op_set_exists_region

```mach
pub fun op_set_exists_region(defs: *Table, src: str, off: usize, len: usize) bool;
```

