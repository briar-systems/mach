# mach.lang.fe.parser.decl

Declarations, statements, and the module-level parse entry point.
Decls and stmts are kept together because they are mutually recursive
(function bodies contain stmts; local `val`/`var` stmts wrap decls;
`$if` conditionals appear at both scopes). Also owns the orchestrators
[`parse_module`](#parse_module), [`parse_decl`](#parse_decl), and
[`parse_stmt`](#parse_stmt).

## Public surface

### `parse_module`

```mach
pub fun parse_module(p: *parser.Parser) id.ModuleId
```

Parses a whole source file as a single [`Module`](../ast/module.md#module),
sets [`p.ast.root_module`](../ast.md#ast) to the result, and returns
the new [`ModuleId`](../ast/id.md#moduleid). Loops [`parse_decl`](#parse_decl)
until [`KIND_EOF`](../token.md#constants); appended decl ids accumulate
in [`p.ast.decl_ids`](../ast.md#ast) and are referenced from
[`Module.decls_start` / `Module.decls_len`](../ast/module.md#module).

| Param | Type                                          | Description    |
|-------|-----------------------------------------------|----------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state.  |

### `parse_decl`

```mach
pub fun parse_decl(p: *parser.Parser) id.DeclId
```

Parses one declaration. Reads optional [flags](#flags) via
[`parse_flags`](#flags), then dispatches on the leading keyword. On
syntax error, [`sync_to`](state.md#sync_to) skips to the next
declaration boundary and an [`DECL_KIND_ERROR`](../ast/decl.md#declkind)
node is returned.

| Param | Type                                          | Description    |
|-------|-----------------------------------------------|----------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state.  |

### `parse_stmt`

```mach
pub fun parse_stmt(p: *parser.Parser) id.StmtId
```

Parses one statement. Dispatches on the leading token; falls back to
[`parse_expr_stmt`](#internal-helpers) when no statement keyword
matches.

| Param | Type                                          | Description    |
|-------|-----------------------------------------------|----------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state.  |

## Declaration grammar

```
decl              := flags? (use | fun | rec | bind | def | test | comptime)
flags             := ("pub" | "ext" | "fwd")*
use               := "use" (IDENT ":")? dotted_path ";"
fun               := "fun" IDENT generic_params? fun_params return_type? block
                   | "ext" "fun" IDENT generic_params? fun_params return_type? ";"
rec               := "rec" IDENT generic_params? rec_body
bind              := ("val" | "var") IDENT ":" type ("=" expr)? ";"
def               := "def" IDENT ":" type ";"
test              := "test" STR block
comptime          := "$if" "(" expr ")" decl_block ("$or" ("(" expr ")")? decl_block)*
                   | "$" comptime_ident "=" expr ";"
generic_params    := "[" IDENT ("," IDENT)* ","? "]"
fun_params        := "(" (typed_name ("," typed_name)* ","?)? ")"
rec_body          := "{" (typed_name ";")+ "}"
typed_name        := IDENT ":" type
return_type       := type            # parsed when at_type_boundary holds after fun_params
block             := "{" stmt* "}"
decl_block        := "{" decl* "}"
dotted_path       := IDENT ("." IDENT)*
```

### Flags

```mach
fun parse_flags(p: *parser.Parser) u8
```

Reads zero or more of `pub`, `ext`, `fwd` in any order, returning the
combined bitfield of [`DECL_FLAG_*`](../ast/decl.md#constants).

### Dispatch

Leading keyword (after optional flags):

| Keyword | Decl form                                                  |
|---------|------------------------------------------------------------|
| `use`   | [`parse_use_decl`](#internal-helpers)                      |
| `fun`   | [`parse_fun_decl`](#internal-helpers)                      |
| `rec`   | [`parse_rec_decl`](#internal-helpers)                      |
| `val`   | [`parse_bind_decl`](#internal-helpers) (`DECL_KIND_VAL`)   |
| `var`   | [`parse_bind_decl`](#internal-helpers) (`DECL_KIND_VAR`)   |
| `def`   | [`parse_def_decl`](#internal-helpers)                      |
| `test`  | [`parse_test_decl`](#internal-helpers)                     |
| `$if`   | [`parse_comptime_if_decl`](#internal-helpers)              |
| `$...`  | [`parse_comptime_attr_decl`](#internal-helpers)            |

When no rule matches, [`error_at_current`](state.md#error_at_current)
emits `"expected a declaration"` and [`sync_to`](state.md#sync_to)
skips to the next semicolon or closing brace.

### `pub fwd` re-exports

`pub fwd <module-prefixed-symbol>;` is parsed by
[`parse_use_decl`](#internal-helpers) — the dotted path identifies the
target. [`DECL_FLAG_FWD`](../ast/decl.md#constants) is set on the
emitted [`Decl`](../ast/decl.md#decl) so resolve can wire the alias
without re-importing the whole module.

## Statement grammar

```
stmt              := block
                   | if_stmt
                   | for_stmt
                   | ret_stmt
                   | brk_stmt
                   | cnt_stmt
                   | fin_stmt
                   | asm_stmt
                   | local_decl_stmt
                   | comptime_if_stmt
                   | expr_stmt
block             := "{" stmt* "}"
if_stmt           := "if" "(" expr ")" stmt or_chain?
or_chain          := "or" ("(" expr ")")? stmt or_chain?
for_stmt          := "for" ("(" expr ")")? stmt
ret_stmt          := "ret" expr? ";"
brk_stmt          := "brk" ";"
cnt_stmt          := "cnt" ";"
fin_stmt          := "fin" stmt
asm_stmt          := "asm" (IDENT)? ("(" asm_op ("," asm_op)* ","? ")")? "{" raw_text "}"
local_decl_stmt   := bind
comptime_if_stmt  := "$if" "(" expr ")" stmt_block ("$or" ("(" expr ")")? stmt_block)*
expr_stmt         := expr ";"
stmt_block        := "{" stmt* "}"
```

### Dispatch

Leading token / keyword in [`parse_stmt`](#parse_stmt):

| Leading                                  | Stmt form                                |
|------------------------------------------|------------------------------------------|
| `KIND_LBRACE`                            | [`parse_block_stmt`](#internal-helpers)  |
| `KIND_IDENT` text = `"if"`               | [`parse_if_stmt`](#internal-helpers)     |
| `KIND_IDENT` text = `"for"`              | [`parse_for_stmt`](#internal-helpers)    |
| `KIND_IDENT` text = `"ret"`              | [`parse_ret_stmt`](#internal-helpers)    |
| `KIND_IDENT` text = `"brk"`              | [`parse_brk_stmt`](#internal-helpers)    |
| `KIND_IDENT` text = `"cnt"`              | [`parse_cnt_stmt`](#internal-helpers)    |
| `KIND_IDENT` text = `"fin"`              | [`parse_fin_stmt`](#internal-helpers)    |
| `KIND_IDENT` text = `"asm"`              | [`asm.parse_asm`](asm.md#parse_asm)      |
| `KIND_IDENT` text = `"val"` / `"var"`    | [`parse_local_decl_stmt`](#internal-helpers) |
| `KIND_DOLLAR` + peek `"if"`              | [`parse_comptime_if_stmt`](#internal-helpers) |
| anything else                            | [`parse_expr_stmt`](#internal-helpers)   |

### `fin` body

[`parse_fin_stmt`](#internal-helpers) parses a statement (via
[`parse_stmt`](#parse_stmt)) for the body. This covers both
single-statement forms (`fin close_file(fd);`) and block forms
(`fin { close_file(fd); }`) because both are valid statements.

### `if` / `or` chain

[`parse_if_stmt`](#internal-helpers) parses the leading `if (cond)
stmt`, then [`parse_or_chain`](#internal-helpers) folds each
trailing `or (cond)? stmt` into the previous `if`'s `else_block`
recursively, building a right-linear chain of
[`StmtIf`](../ast/stmt.md#stmtif) nodes.

### Comptime branches

[`parse_comptime_if_decl`](#internal-helpers) and
[`parse_comptime_if_stmt`](#internal-helpers) both build a
[`ComptimeBranch`](../ast/decl.md#comptimebranch) array. The first arm
is keyed by `$if`; subsequent arms are `$or` with an optional
condition (final arm typically has none, acting as the `else`).

For decl-scope branches, branch bodies are sequences of decl ids
accumulated in [`p.ast.decl_ids`](../ast.md#ast). For stmt-scope
branches, bodies are sequences of stmt ids accumulated in
[`p.ast.stmt_ids`](../ast.md#ast). The owning node's kind
([`DECL_KIND_COMPTIME_IF`](../ast/decl.md#constants) vs
[`STMT_KIND_COMPTIME_IF`](../ast/stmt.md#constants)) is what
disambiguates which pool the `body_start`/`body_len` indexes.

## Internal helpers

File-private; listed for reference.

| Function                          | Role                                                                       |
|-----------------------------------|----------------------------------------------------------------------------|
| `parse_flags`                     | Reads `pub` / `ext` / `fwd` flag bitfield.                                 |
| `parse_dotted_path`               | Reads `IDENT ("." IDENT)*` as a single span.                               |
| `peek_is_kw`                      | Peek-only predicate: is the token at offset N an IDENT matching `kw`?      |
| `parse_use_decl`                  | `use [alias :] path ;` and `pub fwd path ;` forms.                         |
| `parse_fun_decl`                  | `fun name (params) ret { body }` and `ext fun ...` forms.                  |
| `parse_rec_decl`                  | `rec name [generics] { fields }`.                                          |
| `parse_bind_decl`                 | `val` / `var` declarations at decl scope.                                  |
| `parse_def_decl`                  | `def name : type ;` aliases.                                               |
| `parse_test_decl`                 | `test "label" { body }`.                                                   |
| `parse_comptime_decl`             | Dispatches between `$if`-decl and `$<attr> = value` forms.                 |
| `parse_comptime_if_decl`          | `$if` / `$or` chain at decl scope.                                         |
| `parse_comptime_attr_decl`        | `$<attr> = value ;` attribute directive.                                   |
| `parse_generic_params`            | `[T, U]` list of generic parameter names.                                  |
| `parse_generic_param`             | Single generic parameter identifier.                                       |
| `parse_fun_params`                | `(name: type, ...)` parameter list.                                        |
| `parse_rec_fields`                | `{ name: type; ... }` record-field list.                                   |
| `parse_typed_name`                | `IDENT ":" type` entry used by params and fields.                          |
| `parse_block_stmt`                | `{ stmt* }`.                                                               |
| `parse_if_stmt`                   | Leading `if (cond) stmt`, then delegates to `parse_or_chain`.              |
| `parse_or_chain`                  | Folds trailing `or` arms into a right-linear `StmtIf` chain.               |
| `parse_for_stmt`                  | `for (cond?) stmt`.                                                        |
| `parse_ret_stmt`                  | `ret expr? ;`.                                                             |
| `parse_brk_stmt` / `parse_cnt_stmt` | Bare control-flow statements.                                            |
| `parse_fin_stmt`                  | `fin stmt` (single statement or block).                                    |
| `parse_local_decl_stmt`           | Wraps a `val` / `var` decl as a [`STMT_KIND_DECL`](../ast/stmt.md#constants). |
| `parse_expr_stmt`                 | `expr ;`.                                                                  |
| `parse_comptime_if_stmt`          | `$if` / `$or` chain at stmt scope.                                         |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.result`,
[`mach.lang.fe.token`](../token.md), [`mach.lang.fe.ast`](../ast.md),
[`mach.lang.fe.ast.id`](../ast/id.md), [`mach.lang.fe.ast.expr`](../ast/expr.md),
[`mach.lang.fe.ast.stmt`](../ast/stmt.md),
[`mach.lang.fe.ast.decl`](../ast/decl.md),
[`mach.lang.fe.ast.type`](../ast/type.md),
[`mach.lang.fe.ast.module`](../ast/module.md),
[`mach.lang.fe.parser.state`](state.md),
[`mach.lang.fe.parser.expr`](expr.md),
[`mach.lang.fe.parser.asm`](asm.md).
