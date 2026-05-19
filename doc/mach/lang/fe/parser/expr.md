# mach.lang.fe.parser.expr

Expression and type parsing. Expressions use a hybrid recursive-descent
primary / Pratt climber; types use plain recursive descent. The two
grammars live in one module because they are mutually recursive: types
contain expressions (array lengths), expressions contain types (casts,
struct-literal and array-literal prefixes, generic arguments).

## Public surface

### `parse_expr`

```mach
pub fun parse_expr(p: *parser.Parser) id.ExprId
```

Parses one full expression at the lowest binding power (i.e. accepting
every infix operator). Returns the [`ExprId`](../ast/id.md#exprid) of
the parsed expression, or an [`EXPR_KIND_ERROR`](../ast/expr.md#exprkind)
node when no expression could be recognised at the current token.

| Param | Type                                          | Description    |
|-------|-----------------------------------------------|----------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state.  |

### `parse_expr_bp`

```mach
pub fun parse_expr_bp(p: *parser.Parser, min_bp: u8) id.ExprId
```

Pratt-climb entry point parameterised by the minimum binding power.
Used by [`parse_expr`](#parse_expr) (`min_bp = 0`) and recursively
during infix climbing. Higher `min_bp` values stop the climber earlier;
callers seldom invoke this directly.

| Param  | Type                                  | Description                                              |
|--------|---------------------------------------|----------------------------------------------------------|
| p      | [`*parser.Parser`](state.md#parser)   | Parser state.                                            |
| min_bp | `u8`                                  | Minimum infix precedence to accept (see [token.infix_precedence](../token.md#infix_precedence)).|

### `parse_type`

```mach
pub fun parse_type(p: *parser.Parser) id.TypeId
```

Parses one type expression by recursive descent. Dispatches on the
leading token and returns the [`TypeId`](../ast/id.md#typeid) of the
constructed syntactic [`Type`](../ast/type.md#type), or an
[`TYPE_KIND_ERROR`](../ast/type.md#typekind) node when no type could
be recognised.

| Param | Type                                          | Description    |
|-------|-----------------------------------------------|----------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state.  |

## Expression grammar

```
expr     := prefix postfix* (infix expr_bp)*
prefix   := literal
          | ident
          | comptime_ident
          | paren
          | unary
          | array_literal
          | typed_literal
literal  := INT | FLOAT | CHAR | STR | ZSTR | "nil"
ident    := IDENT
comptime_ident := "$" IDENT ("." IDENT)*
paren    := "(" expr ")"
unary    := ("-" | "!" | "~" | "?" | "@") prefix
array_literal  := "[" expr "]" type "{" expr ("," expr)* ","? "}"
typed_literal  := named_type "{" field_init ("," field_init)* ","? "}"
field_init     := IDENT ":" expr
postfix  := call | index | member | cast
call     := "(" (expr ("," expr)* ","?)? ")"
index    := "[" expr "]"
member   := "." IDENT
cast     := "::" type
infix    := "+" | "-" | "*" | "/" | "%"
          | "==" | "!=" | "<" | "<=" | ">" | ">=" | "<<" | ">>"
          | "&" | "|" | "^" | "&&" | "||" | "="
```

Operator precedences and associativities come from
[`token.infix_precedence`](../token.md#infix_precedence) and
[`token.is_right_assoc`](../token.md#is_right_assoc); the only
right-associative operator is `=`.

### Prefix dispatch

[`parse_prefix`](#internal-helpers) consults the leading token and
delegates:

| Leading token kind                          | Prefix form                                |
|---------------------------------------------|--------------------------------------------|
| `KIND_LIT_INT`                              | [`parse_lit_int`](#internal-helpers)       |
| `KIND_LIT_FLOAT`                            | [`parse_lit_float`](#internal-helpers)     |
| `KIND_LIT_CHAR`                             | [`parse_lit_char`](#internal-helpers)      |
| `KIND_LIT_STR`                              | [`parse_lit_str`](#internal-helpers)       |
| `KIND_LIT_ZSTR`                             | [`parse_lit_zstr`](#internal-helpers)      |
| `KIND_MINUS`                                | [`parse_unary`](#internal-helpers) (`UN_NEG`) |
| `KIND_BANG`                                 | [`parse_unary`](#internal-helpers) (`UN_NOT`) |
| `KIND_TILDE`                                | [`parse_unary`](#internal-helpers) (`UN_BIT_NOT`) |
| `KIND_QUESTION`                             | [`parse_unary`](#internal-helpers) (`UN_ADDR`) |
| `KIND_AT`                                   | [`parse_unary`](#internal-helpers) (`UN_DEREF`) |
| `KIND_LPAREN`                               | [`parse_paren`](#internal-helpers)         |
| `KIND_DOLLAR`                               | [`parse_comptime_ident`](#internal-helpers)|
| `KIND_LBRACKET`                             | [`parse_array_literal`](#internal-helpers) |
| `KIND_IDENT` (text = `"nil"`)               | [`parse_lit_nil`](#internal-helpers)       |
| `KIND_IDENT` + lookahead matches type{}     | [`parse_typed_literal`](#internal-helpers) |
| `KIND_IDENT`                                | [`parse_ident`](#internal-helpers)         |

When no prefix matches, `parse_prefix` emits
[`error_at_current`](state.md#error_at_current) with `"expected an
expression"` and returns an [`EXPR_KIND_ERROR`](../ast/expr.md#exprkind)
node at the current span.

### Typed-literal lookahead

[`looks_like_typed_literal`](#internal-helpers) peeks past an
identifier (and any `[...]` generic arguments) for a `{` to distinguish
`Name { ... }` (a struct literal) from a bare ident followed by an
unrelated `{` (a block start). The peek does not consume tokens.

### Postfix dispatch

[`parse_postfix`](#internal-helpers) loops on the next token:

| Token kind          | Action                                       |
|---------------------|----------------------------------------------|
| `KIND_LPAREN`       | [`parse_call`](#internal-helpers)            |
| `KIND_LBRACKET`     | [`parse_index`](#internal-helpers)           |
| `KIND_DOT`          | [`parse_member`](#internal-helpers)          |
| `KIND_COLON_COLON`  | [`parse_cast`](#internal-helpers)            |
| anything else       | exit the loop                                |

### Infix climb

After the prefix + postfix chain is built, [`parse_expr_bp`](#parse_expr_bp)
queries [`token.infix_precedence`](../token.md#infix_precedence) for
the current token. If `precedence >= min_bp`, the climber:

1. Reads the operator with [`bin_op_from_kind`](#internal-helpers).
2. Consumes the operator token.
3. Recurses with `min_bp = precedence + 1` for left-associative ops,
   `min_bp = precedence` for right-associative ones (only `=`).
4. Wraps the left and right operands in an [`EXPR_KIND_BINARY`](../ast/expr.md#exprkind)
   node and continues the loop.

## Type grammar

```
type        := named_type
             | "*" type
             | "[" expr "]" type
             | "fun" "(" type_list? ")" type?
             | "rec" "{" typed_name (";" typed_name)* ";"? "}"
             | "uni" "{" typed_name (";" typed_name)* ";"? "}"
named_type  := IDENT ("." IDENT)* generic_args?
generic_args := "[" type ("," type)* ","? "]"
type_list   := type ("," type)*
typed_name  := IDENT ":" type
```

[`parse_type`](#parse_type) dispatches on the leading token:

| Leading token kind                              | Type form                              |
|-------------------------------------------------|----------------------------------------|
| `KIND_STAR`                                     | [`parse_ptr`](#internal-helpers)       |
| `KIND_LBRACKET`                                 | [`parse_array`](#internal-helpers)     |
| `KIND_IDENT` + text = `"fun"`                   | [`parse_fun`](#internal-helpers)       |
| `KIND_IDENT` + text = `"rec"`                   | [`parse_rec_or_uni`](#internal-helpers) (TYPE_KIND_REC) |
| `KIND_IDENT` + text = `"uni"`                   | [`parse_rec_or_uni`](#internal-helpers) (TYPE_KIND_UNI) |
| `KIND_IDENT` (other)                            | [`parse_named`](#internal-helpers)     |

When no rule matches, `parse_type` emits `"expected a type"` and
returns a [`TYPE_KIND_ERROR`](../ast/type.md#typekind) node.

[`at_type_boundary`](#internal-helpers) is a peek-only predicate used
by callers (e.g. function-type return slot detection) to determine
whether the cursor is positioned at the start of a type.

## Internal helpers

These are file-private; the table is for reference only.

| Function                                  | Role                                                                 |
|-------------------------------------------|----------------------------------------------------------------------|
| `parse_prefix`                            | Dispatch table for leading-token expression forms.                   |
| `parse_postfix`                           | Loop that applies postfix operators (call, index, member, cast).     |
| `parse_lit_int` / `_float` / `_char` / `_str` / `_zstr` / `_nil` | Literal-form constructors. |
| `parse_ident`                             | Bare identifier reference.                                           |
| `parse_comptime_ident`                    | `$`-prefixed identifier, including `$mach.*` dotted form.            |
| `parse_unary`                             | Builds an [`EXPR_KIND_UNARY`](../ast/expr.md#exprkind) over a prefix.|
| `parse_paren`                             | Recurses via `parse_expr` between matching parens; no wrapping node. |
| `parse_call` / `parse_index` / `parse_member` / `parse_cast` | Postfix-operator constructors.    |
| `parse_array_literal`                     | `[N]T{...}` form; expects a type after the `[N]`.                    |
| `parse_typed_literal`                     | `Name{...}` and `Name[T,U]{...}` form.                               |
| `parse_field_init`                        | `IDENT ":" expr` entry inside a struct literal body.                 |
| `looks_like_typed_literal`                | Two-token lookahead for ident-followed-by-`{`.                       |
| `bin_op_from_kind`                        | [`token.Kind`](../token.md#kind) → [`BinOp`](../ast/expr.md#binop) table. |
| `span_of_expr_pair`                       | `span_of` over two `ExprId`s' spans.                                 |
| `parse_named` / `parse_ptr` / `parse_array` / `parse_fun` / `parse_rec_or_uni` | Type-form constructors. |
| `at_type_boundary`                        | Peek-only predicate; `true` when the cursor begins a type.           |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.fe.token`](../token.md), [`mach.lang.fe.ast`](../ast.md),
[`mach.lang.fe.ast.id`](../ast/id.md), [`mach.lang.fe.ast.expr`](../ast/expr.md),
[`mach.lang.fe.ast.decl`](../ast/decl.md),
[`mach.lang.fe.ast.type`](../ast/type.md),
[`mach.lang.fe.parser.state`](state.md).
