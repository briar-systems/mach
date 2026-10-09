# mach.lang.fe.comptime.intrinsic

the comptime intrinsic registry: one row per `$name(...)` the compiler answers,
which every phase reads by id

each phase decodes a comptime call's `$name` to the id it spells and decides by
the id and its row, never by the name. a new intrinsic is a row here and the
consumers that answer it

## def Id

```mach
pub def Id: u8
```

## val NONE

```mach
pub val NONE:            Id = 0
```

## val SIZE_OF

```mach
pub val SIZE_OF:         Id = 1
```

## val ALIGN_OF

```mach
pub val ALIGN_OF:        Id = 2
```

## val OFFSET_OF

```mach
pub val OFFSET_OF:       Id = 3
```

## val LENGTH_OF

```mach
pub val LENGTH_OF:       Id = 4
```

## val TYPE_ID

```mach
pub val TYPE_ID:         Id = 5
```

## val TYPE_NAME

```mach
pub val TYPE_NAME:       Id = 6
```

## val IS_RECORD

```mach
pub val IS_RECORD:       Id = 7
```

## val IS_UNION

```mach
pub val IS_UNION:        Id = 8
```

## val IS_TAG

```mach
pub val IS_TAG:          Id = 9
```

## val IS_POINTER

```mach
pub val IS_POINTER:      Id = 10
```

## val IS_SECRET

```mach
pub val IS_SECRET:       Id = 11
```

## val HOLDS_SECRET

```mach
pub val HOLDS_SECRET:    Id = 12
```

## val IS_INTEGER

```mach
pub val IS_INTEGER:      Id = 13
```

## val IS_FLOAT

```mach
pub val IS_FLOAT:        Id = 14
```

## val TYPE_OF

```mach
pub val TYPE_OF:         Id = 15
```

## val POINTEE_OF

```mach
pub val POINTEE_OF:      Id = 16
```

## val DISCRIMINANT_OF

```mach
pub val DISCRIMINANT_OF: Id = 17
```

## def Class

```mach
pub def Class: u8
```

what an intrinsic answers

CLASS_LAYOUT: a constant measured from a type's layout or identity
CLASS_QUERY: a predicate over a type, or its spelling
CLASS_GATE: a type compared inside a `$if` gate, never a value
CLASS_CONSTRUCTOR: a type built from a type, written where a type is expected
CLASS_ITERATION: the members a `$each` walks
CLASS_DIRECTIVE: a compile-time refusal

## val CLASS_LAYOUT

```mach
pub val CLASS_LAYOUT:      Class = 0
```

## val CLASS_QUERY

```mach
pub val CLASS_QUERY:       Class = 1
```

## rec Row

```mach
pub rec Row;
```

one intrinsic

args: the argument count it takes
type_operand: its first argument is a type, written in the full type grammar

## fun row

```mach
pub fun row(id: Id) *Row;
```

the row of `id`; nil for NONE and any id no row declares

## fun id_of

```mach
pub fun id_of(source: str, name: lang_source.Span) Id;
```

the intrinsic the name at `name` in `source` spells, without its `$`; NONE when it spells none

## fun in_class

```mach
pub fun in_class(id: Id, class: Class) bool;
```

whether `id` answers within `class`

## fun constructor_refusal

```mach
pub fun constructor_refusal(a: *A.Allocator) res[str, textbuild.Error];
```

the refusal of a `$` in a type that names no type constructor, listing the ones there are

## fun call_intrinsic

```mach
pub fun call_intrinsic(a: *ast.Ast, source: str, eid: ast_id.ExprId) Id;
```

the intrinsic a comptime call `$name(...)` names; NONE for any other expression

## fun ident_intrinsic

```mach
pub fun ident_intrinsic(source: str, full: lang_source.Span) Id;
```

the intrinsic a `$name` token spells

## fun first_call_arg

```mach
pub fun first_call_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun intrinsic_takes_type_operand

```mach
pub fun intrinsic_takes_type_operand(source: str, full: lang_source.Span) bool;
```

## fun is_type_of_call

```mach
pub fun is_type_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun type_of_arg

```mach
pub fun type_of_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun is_fields_call

```mach
pub fun is_fields_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_cases_call

```mach
pub fun is_cases_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_layout_intrinsic_call

```mach
pub fun is_layout_intrinsic_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_type_id_call

```mach
pub fun is_type_id_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_length_of_call

```mach
pub fun is_length_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_offset_of_call

```mach
pub fun is_offset_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_size_of_call

```mach
pub fun is_size_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_align_of_call

```mach
pub fun is_align_of_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun layout_intrinsic_type_arg

```mach
pub fun layout_intrinsic_type_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun fields_type_arg

```mach
pub fun fields_type_arg(a: *ast.Ast, eid: ast_id.ExprId) ast_id.ExprId;
```

## fun is_error_call

```mach
pub fun is_error_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun type_operand_value

```mach
pub fun type_operand_value(a: *ast.Ast, source: str, operand: ast_id.ExprId) ast_id.ExprId;
```

## fun is_type_comparison

```mach
pub fun is_type_comparison(a: *ast.Ast, source: str, bin: *ast_expr.ExprBinary) bool;
```

## fun is_type_comparison_binary

```mach
pub fun is_type_comparison_binary(a: *ast.Ast, source: str, bin: *ast_expr.ExprBinary,
lhs_is_field_type: bool, rhs_is_field_type: bool) bool;
```

## fun is_field_type_member

```mach
pub fun is_field_type_member(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_field_descriptor_member

```mach
pub fun is_field_descriptor_member(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_type_query_call

```mach
pub fun is_type_query_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

## fun is_type_question_call

```mach
pub fun is_type_question_call(a: *ast.Ast, source: str, eid: ast_id.ExprId) bool;
```

a call that asks about a type: its layout, its identity, or a predicate over it

## fun is_path_call

```mach
pub fun is_path_call(a: *ast.Ast, e: ast_id.ExprId) bool;
```

a call whose callee is a rooted comptime path: `$mach.build.ct_mul(low, 64)`

## fun is_comptime_value

```mach
pub fun is_comptime_value(a: *ast.Ast, e: ast_id.ExprId) bool;
```

a rooted comptime path, or a call on one: both fold to a constant

## fun is_comptime_path

```mach
pub fun is_comptime_path(a: *ast.Ast, e: ast_id.ExprId) bool;
```

## fun comptime_ident_name

```mach
pub fun comptime_ident_name(full: lang_source.Span) lang_source.Span;
```

