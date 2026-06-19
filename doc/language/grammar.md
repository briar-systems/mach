# Mach grammar (EBNF)

A formal grammar for the **implemented** Mach dialect, derived from the
parser (`src/lang/fe/lexer.mach`, `src/lang/fe/token.mach`, and
`src/lang/fe/parser/`) and cross-checked against the per-element docs in
this directory. Where the live parser diverges from a doc, the divergence
is called out inline. Productions that could not be fully pinned to the
parser are marked `(* approximate, verify *)`.

This is a reference grammar, not the parser's exact control flow. The
parser is a hybrid recursive-descent / Pratt climber that is
malformed-input tolerant (it recovers and continues rather than failing),
so some ambiguities the grammar leaves open are resolved operationally —
those are noted.


## Meta-notation

| Notation        | Meaning                                              |
|-----------------|------------------------------------------------------|
| `::=`           | production definition                                |
| `\|`            | alternation                                          |
| `{ x }`         | zero or more repetitions of `x`                      |
| `[ x ]`         | optional `x` (zero or one)                           |
| `( x )`         | grouping                                             |
| `"abc"`         | a literal terminal (exact source characters)         |
| `'a'`           | a single literal character                           |
| `UPPER`         | a lexical token class (terminal produced by the lexer) |
| `lower`         | a grammar nonterminal                                |
| `(* ... *)`     | a note / annotation                                  |

Whitespace and comments may appear between any two tokens (see
[Lexical grammar](#lexical-grammar)); they are not shown in the
productions below.


## Lexical grammar

### Tokens

The lexer (`lexer.mach`) emits these token kinds (`token.mach`):

```ebnf
token ::= IDENT
        | LIT_INT | LIT_FLOAT | LIT_CHAR | LIT_STR
        | punctuation
        | operator
        | EOF
        | ERROR    (* emitted for an unexpected character, see below *)

punctuation ::= "(" | ")" | "{" | "}" | "[" | "]"
              | ";" | ":" | "," | "." | "?" | "@" | "$" | "`"
              | "::" | ":~" | "..."

operator ::= "+" | "-" | "*" | "/" | "%"
           | "&" | "|" | "^" | "~"
           | "=" | "==" | "!" | "!="
           | "<" | "<=" | "<<"
           | ">" | ">=" | ">>"
           | "&&" | "||"
```

Notes:
- There is **no dedicated keyword token**. Keywords are ordinary `IDENT`
  tokens; the parser recognizes them contextually by their text
  (`at_kw` / `eat_kw` in `parser/state.mach`). The same applies to the
  primitive type names and `nil`.
- The maximal-munch multi-character operators are `::`, `:~`, `...`, `==`,
  `!=`, `<=`, `<<`, `>=`, `>>`, `&&`, `||`. A leading `:` lexes as `::` or
  `:~` before a bare `:`, so `expr:~Type` is one cast, never `:` then `~`.
- The lexer recognizes the standalone characters `=`, `!`, `<`, `>`, `&`,
  `|` and the two-character forms above. There is no `+=`, `-=`, etc. —
  compound assignment does not exist.
- `ERROR` (`KIND_ERROR`) is a real token kind the lexer emits for an
  unexpected character: the input is recorded as a `LEX_ERR_UNEXPECTED_CHAR`
  in the sibling error buffer, a one-byte `ERROR` token is pushed, and
  lexing continues. `EOF` (`KIND_EOF`) terminates every stream. Neither
  appears in a well-formed production; they are listed here for completeness.

### Identifiers and keywords

```ebnf
IDENT       ::= ident-start { ident-char }
ident-start ::= 'a'..'z' | 'A'..'Z' | '_'
ident-char  ::= ident-start | '0'..'9'
```

The reserved keywords (matched as `IDENT` text by the parser) are:

```
asm  brk  cnt  def  ext  fin  for  fun  fwd  if
nil  or   pub  rec  ret  test uni  use  val  var
```

`nil` is an expression literal; the rest are statement/declaration/type
introducers. Note these are *contextual*: nothing in the lexer prevents a
binding or field from being named after one, but the parser will treat the
keyword in its keyword position. The operand-less statement keywords `brk`
and `cnt` are keywords only in their bare form (`brk;` / `cnt;`); the same
word followed by anything else is an ordinary identifier, so `cnt = x;` is an
assignment to a variable named `cnt`. The primitive type names (`u8`, `u16`,
`u32`, `u64`, `i8`, `i16`, `i32`, `i64`, `f32`, `f64`, `ptr`) are *not*
keywords — they are ordinary identifiers resolved by name later.

### Integer literals

```ebnf
LIT_INT  ::= dec-int | hex-int | bin-int | oct-int

dec-int  ::= digit { digit | "_" } [ int-suffix ]
hex-int  ::= "0x" hex-digit { hex-digit | "_" } [ int-suffix ]
bin-int  ::= "0b" ( "0" | "1" ) { "0" | "1" | "_" } [ int-suffix ]
oct-int  ::= "0o" oct-digit { oct-digit | "_" } [ int-suffix ]

int-suffix ::= ( "u" | "i" ) { digit }   (* e.g. u8, i64 — width digits *)

digit     ::= '0'..'9'
hex-digit ::= digit | 'a'..'f' | 'A'..'F'
oct-digit ::= '0'..'7'
```

A leading `0x` / `0b` / `0o` selects the base; the `u`/`i` width suffix is
scanned on any non-float integer literal regardless of base (the lexer
scans the digit run, then — when the token is not a float — accepts a
trailing `u`/`i` followed by width digits). Underscores are permitted as
digit separators in every base.

### Float literals

```ebnf
LIT_FLOAT ::= digit { digit | "_" }
              ( frac [ exponent ] | exponent )
              [ float-suffix ]

frac        ::= "." digit { digit | "_" }
exponent    ::= ( "e" | "E" ) [ "+" | "-" ] digit { digit | "_" }
float-suffix ::= "f" { digit }              (* e.g. f32, f64 *)
```

A token is a float when it has a fractional part (`.` followed by a digit)
and/or a scientific exponent. The `.` must be immediately followed by a
digit, otherwise it lexes as a separate `.` token (so `1.field` is
`1` `.` `field`, not a float).

### Char and string literals

```ebnf
LIT_CHAR ::= "'" char-body "'"
LIT_STR  ::= '"' { str-char } '"'
```

The lexer captures the raw span between the quotes and treats `\` as an
escape that consumes the next character (so an escaped quote does not
terminate the literal). Escape decoding happens later
(`comptime.eval_lit_char`); the recognized escapes are:

```
char escapes:   \n  \t  \r  \\  \'  \0  \xHH
string escapes: (char escapes) + \"
```

A string literal is single-line (see [literals.md](literals.md)): the lexer
scans from the opening `"` to the next unescaped `"`, stopping at an
unescaped newline or EOF. A char literal scans likewise to the next `'`. An
unterminated `'` or `"` (the scan hits a newline or EOF first) is a lexer
error but still produces a token span covering the rest of that line.

### Comments and whitespace

```ebnf
comment    ::= "#" { any-char-except-newline }
whitespace ::= " " | "\t" | "\n" | "\r"
```

`#` begins a line comment that runs to (but not including) the next
newline. There is no block-comment form. Whitespace and comments separate
tokens and are otherwise discarded.

### Unexpected characters

The lexer has no fall-through quote or reserved class: any byte that begins
none of the token forms above is an **unexpected character**. The lexer
records a `LEX_ERR_UNEXPECTED_CHAR`, emits a one-byte `ERROR` token
(`KIND_ERROR`) for it, and continues.

The backtick `` ` `` is a real punctuation token (`KIND_BACKTICK`) used to
delimit backtick decorators (see [Decorators](#decorators) below).


## Module

A source file is a single module: a sequence of declarations.

```ebnf
module ::= { decl }
```


## Decorators

Zero or more leading backtick decorator clauses may appear before any
declaration. Each clause is a comptime directive name, optionally followed
by a parenthesized argument list of comptime expressions.

```ebnf
decorator     ::= "`" IDENT [ "(" [ expr { "," expr } ] ")" ] "`"
decorated-decl ::= { decorator } decl
```

- Decorators attach to the **immediately following** declaration and do not
  bleed across declarations.
- Multiple decorators may appear on the same line (space-separated) or one
  per line.
- The argument list is optional; a bare `` `inline` `` carries no arguments.
- Arguments are comptime expressions (not types): `$size_of(T)` is a valid
  argument; `T` as a raw type name is not.
- The closed directive set is `symbol`, `section`, `inline`, `align`,
  `library`. See [decorators.md](decorators.md).


## Declarations

```ebnf
decl ::= comptime-decl
       | flags ( use-decl
               | fwd-decl
               | fun-decl
               | rec-decl
               | uni-decl
               | bind-decl
               | def-decl
               | test-decl )

flags ::= { "pub" | "ext" }          (* any order, any count; deduped to a bitfield *)
```

A leading `$` at declaration scope routes to `comptime-decl` *before*
flags are parsed (so `$if` / `$`-directives cannot carry `pub`/`ext`).

### `use` and `fwd`

```ebnf
use-decl ::= "use" [ IDENT ":" ] dotted-path ";"
fwd-decl ::= "fwd" [ IDENT ":" ] dotted-path ";"

dotted-path ::= IDENT { "." IDENT }
```

`use` imports; `fwd` re-exports. `fwd` always publishes, so `pub fwd` is
rejected by the parser (the `pub` flag is dropped and an error is emitted).

### `def` — type alias

```ebnf
def-decl ::= "def" IDENT ":" type ";"
```

### `rec` and `uni`

```ebnf
rec-decl ::= "rec" IDENT [ generic-params ] field-block
uni-decl ::= "uni" IDENT [ generic-params ] field-block

field-block ::= "{" { typed-name ";" } "}"
```

`rec` is a struct (sequential layout); `uni` is a raw union (overlapping
layout). Both may be generic and both share the same field-block grammar.

### `fun` — function

```ebnf
fun-decl ::= "fun" IDENT [ generic-params ] param-list [ type ] ( block | ";" )
```

- A `{ ... }` block is a defined function; a bare `;` is a forward/external
  signature (used with `ext`, see [ext-fun.md](ext-fun.md)).
- The optional `type` between the parameter list and the body is the return
  type. Its absence (the next token is `{` or `;`) means no return value.

```ebnf
param-list ::= "(" [ params ] ")"

params ::= typed-name { "," typed-name } [ "," pack-param ]
         | pack-param

pack-param ::= IDENT ":" "..."

typed-name ::= [ "$" ] IDENT ":" type
```

- A trailing `name: ...` declares a **comptime variadic pack parameter**
  (`TYPE_KIND_PACK`); it must be the last parameter. The C-style bare `...`
  variadic-parameter marker was **removed in v2.0.0** — the parser rejects it
  with a migration diagnostic pointing to `name: ...`. (A function *pointer
  type* may still carry `...` for FFI; see `fun-type-params` below.)
- A leading `$` on a `typed-name` marks it a **comptime value parameter**.

> **Divergence (parser vs. doc).** `typed-name` is shared between function
> parameters and `rec`/`uni` fields, so the parser will *accept* a leading
> `$` on a named `rec`/`uni` field too. [fun.md](fun.md) states comptime
> value parameters apply to function parameters only; anonymous inline
> `rec {...}` / `uni {...}` field types hardcode the comptime flag off. A
> `$` on a *named-type* field parses but is not the documented surface.

### `val` / `var` — bindings

```ebnf
val-decl  ::= "val" IDENT ":" type "=" expr ";"
var-decl  ::= "var" IDENT ":" type [ "=" expr ] ";"
bind-decl ::= val-decl | var-decl
```

`val` is immutable, `var` mutable. The type annotation is mandatory for both
(Mach has no type inference — see [val-var.md](val-var.md)); the parser
rejects a binding with no `: type`. A `val` requires an initializer; a `var`
may omit it and is default-initialized. A `val`/`var` may also appear as a
local statement (see [Statements](#statements)).

### `test`

```ebnf
test-decl ::= "test" LIT_STR block
```

A `test` carries a string label and a block body.

### Generic parameters

```ebnf
generic-params ::= "[" [ IDENT { "," IDENT } [ "," ] ] "]"
```

A bracketed list of bare type-parameter names with no constraints. An empty
`[]` is accepted.


## Comptime declarations and directives

A `$` at declaration scope is either a comptime `$if` chain or a comptime
directive (`$intrinsic(args);`).

```ebnf
comptime-decl ::= comptime-if-decl | comptime-directive

comptime-if-decl ::= "$" "if" "(" expr ")" decl-branch-body
                     { "$" "or" "(" expr ")" decl-branch-body }
                     [ "$" "or" decl-branch-body ]

decl-branch-body ::= "{" { decl } "}"

comptime-directive ::= expr-no-assign ";"
```

- `comptime-directive` is a bare **comptime intrinsic / directive call**
  (`$error("msg");`). The legacy attribute-write setter (`$sym.attr = value;`)
  was removed in v2.0.0 — per-declaration codegen attributes are written as
  backtick decorators now (see [decorators.md](decorators.md)) — so a stray `=`
  after the target is a parse error. The target is parsed at a binding power
  above assignment so that `=` is detected rather than swallowed into the
  expression.
- `expr-no-assign` is `expr` parsed with the assignment operator excluded
  at the top level (binding power >= 2; see [Expressions](#expressions)). It
  still begins with the leading `$` because the first prefix atom is a
  `comptime-ident`.

The `$if` chain also exists as a **statement** form with the same shape but
a statement-list body (see [Statements](#statements)).


## Types

```ebnf
type ::= ptr-type
       | array-type
       | fun-type
       | rec-type
       | uni-type
       | named-type

ptr-type   ::= "*" type
array-type ::= "[" expr "]" type
named-type ::= dotted-path [ type-args ]
type-args  ::= "[" [ type { "," type } [ "," ] ] "]"

fun-type ::= "fun" "(" [ fun-type-params ] ")" [ type ]
fun-type-params ::= "..."
                  | type { "," type } [ "," "..." ]

rec-type ::= "rec" anon-field-block
uni-type ::= "uni" anon-field-block
anon-field-block ::= "{" { IDENT ":" type ";" } "}"
```

Notes:
- `*T` is a pointer; the untyped pointer type is the primitive name `ptr`
  (an ordinary `named-type`, not its own syntax).
- `[N]T` is a fixed-length array; `N` is a full expression (a comptime
  constant). Nesting (`[N][M]T`) falls out of the recursion.
- `named-type` covers both plain names (`i64`, `Point`) and generic
  instantiations (`Pair[i64, u8]`, `Map[str, u32]`). The dotted path allows
  module-qualified names (`core.Thing`).
- A `fun(...)` type's return type is optional; it is omitted when the next
  token cannot begin a type (`{`, `;`, `,`, `)`, `]`, `=`, EOF).
- `fun`-type parameter lists carry a `variadic` flag via a trailing `...`,
  mirroring `fun` declarations.
- Anonymous inline `rec {...}` / `uni {...}` types use a field block whose
  entries do **not** accept the leading `$` comptime marker.

There is **no `?T` option-type sugar and no Result sugar in the grammar.**
`?` is exclusively the prefix address-of operator (below). `Option` and
`Result` are ordinary stdlib generic types written `Option[T]` /
`Result[T, E]`.


## Expressions

Expressions are a Pratt climber over a primary (prefix + postfix) grammar.

```ebnf
expr ::= prefix { postfix } { binary-op prefix { postfix } }
```

(The braces above are illustrative; the real binding is precedence-driven —
see the precedence table.)

### Prefix (atoms and unary)

```ebnf
prefix ::= LIT_INT | LIT_FLOAT | LIT_CHAR | LIT_STR
         | "nil"
         | IDENT
         | comptime-ident
         | typed-literal
         | array-literal
         | unary-op prefix { postfix }
         | "(" expr ")"

comptime-ident ::= "$" IDENT

unary-op ::= "-"      (* numeric negation *)
           | "!"      (* logical not *)
           | "~"      (* bitwise not *)
           | "?"      (* address-of *)
           | "@"      (* dereference *)
```

- A unary operator binds its operand as `prefix` followed by any postfix
  chain, so `@p.field` and `?arr[i]` apply member/index *inside* the unary.
- `(expr)` is a plain grouping; there is no tuple form.

### Postfix

```ebnf
postfix ::= call-args
          | generic-call
          | index
          | member
          | project
          | cast

call-args    ::= "(" [ call-arg { "," call-arg } [ "," ] ] ")"
call-arg     ::= expr [ "..." ]     (* trailing "..." makes it a va... spread *)
generic-call ::= type-args call-args        (* callee[T, U](args) *)
index        ::= "[" expr "]"
member       ::= "." IDENT
project      ::= "." "[" expr "]"          (* v.[f]: comptime field projection *)
cast         ::= ( "::" | ":~" ) type
```

`::` is a value conversion and `:~` a same-size bit reinterpret; see
[operators.md](operators.md#cast). Both bind as postfix.

Disambiguating a postfix `[`: a bracket whose matching `]` is **not** followed
by `(` is always an index `obj[idx]`. When it **is** followed by `(`, the
bracket may open a generic call `callee[T, U](args)` or be the index of an
index-then-call `callee[idx](args)`. The two grammars overlap — a bare name is
both a type and a value — so the parser cannot decide locally. It probes the
payload against both grammars:
- reads cleanly **only** as a type list (`*T`, `[N]T`, a comma list, or a
  nested generic like `Result[bool, str]`) → a generic call;
- reads cleanly **only** as an expression (`i + 1`, `f(x)`, a literal) → an
  index, so `table[0]()` and `table[i + 1]()` are index-then-call;
- reads cleanly as **both** (a bare identifier, a dotted path, a generic
  application like `v[k]`) → the decision is deferred to name resolution, which
  picks by the callee's resolved kind: a value (or any non-name callee, e.g. an
  index/call result) makes `[x]` an index, while a function, type, or imported
  name takes `[x]` as a type argument. So `table[i]()` calls the indexed
  function pointer and `make[T]()` instantiates the generic — neither needs
  parentheses;
- reads cleanly as **neither** → the committing index parse reports the error
  (never a silent wrong parse).
- The struct-literal `Name[T]{...}` form is recognized at the **prefix**
  stage (`typed-literal`) and never reaches postfix.

### Literals with a type prefix

```ebnf
typed-literal ::= named-type "{" [ field-init { "," field-init } [ "," ] ] "}"
array-literal ::= array-type "{" [ expr { "," expr } [ "," ] ] "}"

field-init ::= IDENT ":" expr
```

- `typed-literal` is a record/union (struct) literal: a named type
  (optionally generic) followed by a brace-delimited list of `field: value`
  initializers (`Point{ x: 1, y: 2 }`, `Pair[i64, u8]{ left: 5, right: 6u8 }`).
  The parser commits to this form via a lookahead
  (`Name (.Name)* ([...])? {`).
- `array-literal` is `[N]T{ e0, e1, ... }` — an array type followed by a
  brace-delimited positional element list.

### Operator precedence

From `token.infix_precedence` / `token.is_right_assoc`. Higher binds
tighter; all binary operators are left-associative **except** `=`
(assignment), which is right-associative. Unary prefix operators and the
postfix chain (call/index/member/cast) bind tighter than every binary
operator.

| Prec | Operators              | Assoc  | Meaning                         |
|------|------------------------|--------|---------------------------------|
| 11   | `*` `/` `%`            | left   | multiply, divide, remainder     |
| 10   | `+` `-`               | left   | add, subtract                   |
| 9    | `<<` `>>`             | left   | shift left / right              |
| 8    | `<` `<=` `>` `>=`     | left   | relational                      |
| 7    | `==` `!=`             | left   | equality                        |
| 6    | `&`                   | left   | bitwise and                     |
| 5    | `^`                   | left   | bitwise xor                     |
| 4    | `\|`                  | left   | bitwise or                      |
| 3    | `&&`                  | left   | logical and (short-circuit)     |
| 2    | `\|\|`                | left   | logical or (short-circuit)      |
| 1    | `=`                   | right  | assignment                      |

```ebnf
binary-op ::= "*" | "/" | "%" | "+" | "-"
            | "<<" | ">>"
            | "<" | "<=" | ">" | ">="
            | "==" | "!="
            | "&" | "^" | "|"
            | "&&" | "||"
            | "="
```

Assignment is an expression (precedence 1), not a statement form; an
assignment statement is just an expression statement whose top operator is
`=`.


## Statements

```ebnf
stmt ::= block
       | if-stmt
       | for-stmt
       | ret-stmt
       | brk-stmt
       | cnt-stmt
       | fin-stmt
       | asm-stmt
       | local-decl-stmt
       | comptime-if-stmt
       | comptime-each-stmt
       | expr-stmt

block ::= "{" { stmt } "}"

if-stmt ::= "if" "(" expr ")" block { or-arm } [ or-else ]
or-arm  ::= "or" "(" expr ")" block
or-else ::= "or" block

for-stmt ::= "for" [ "(" expr ")" ] block      (* no condition => infinite loop *)

ret-stmt ::= "ret" [ expr ] ";"
brk-stmt ::= "brk" ";"
cnt-stmt ::= "cnt" ";"
fin-stmt ::= "fin" stmt                          (* defer: runs stmt at scope exit *)

local-decl-stmt ::= bind-decl                    (* a "val"/"var" used as a statement *)

expr-stmt ::= expr ";"

comptime-if-stmt ::= "$" "if" "(" expr ")" stmt-branch-body
                     { "$" "or" "(" expr ")" stmt-branch-body }
                     [ "$" "or" stmt-branch-body ]
stmt-branch-body ::= "{" { stmt } "}"

comptime-each-stmt ::= "$" "each" IDENT "in" expr stmt-branch-body
```

`$each` is a compile-time unroll: the body is duplicated once per element of
the sequence, which must be `$fields(T)` or a variadic pack identifier. `in`
is a contextual keyword.

Notes:
- `if` / `or`: the `or` chain models both `else if` (`or (cond) { ... }`)
  and `else` (`or { ... }`). Arms are parsed greedily; an `or` with a
  condition continues the chain, an `or` without one terminates it. Each
  arm body is a block.
- `fin S` registers `S` (typically a block) to run when the enclosing scope
  exits — Mach's defer.
- `comptime-if-stmt` is the statement-scope `$if`/`$or` chain (the
  declaration-scope variant is under [Comptime](#comptime-declarations-and-directives)).
  A `$` only begins this form when the next token is the keyword `if`;
  otherwise a leading `$` at statement position is parsed as an
  `expr-stmt` whose first atom is a `comptime-ident`.

### Inline assembly

```ebnf
asm-stmt ::= "asm" IDENT "{" asm-body "}"
```

- The `IDENT` after `asm` is the mandatory **ISA tag** (`x86_64`,
  `aarch64`, … — a closed set). Bare `asm { ... }` is rejected.
- `asm-body` is **raw text**, not a token grammar: the parser captures the
  source span between the opening `{` and its brace-matched `}` and hands it
  to the backend verbatim. Nested `{ }` are balanced by depth. Local
  substitution uses `{name}` references inside the body; operand direction
  and clobbers are inferred from the instruction stream, so there is no
  operand or clobber list. `#` introduces a line comment inside the body.

```ebnf
asm-body ::= (* raw source text, brace-balanced; not tokenized *)
```


## Comptime surface (syntactic forms)

These are not separate grammar productions — they reuse `comptime-ident`,
`call-args`, and `member` — but are listed here as the recognized comptime
shapes for reference. They are accepted syntactically; which ones the
compiler actually resolves is a semantic concern (several are documented
stubs).

```ebnf
comptime-ident ::= "$" IDENT                       (* $size_of, $mach, $type_of, ... *)

intrinsic-call ::= comptime-ident call-args         (* $size_of(T), $fields(T), $type_of(e) *)
mach-read      ::= comptime-ident { member }        (* $mach.build.os, $mach.arch.x86_64 *)
```

- Intrinsic calls (`$size_of(T)`, `$align_of(T)`, `$offset_of(T, field)`,
  `$type_of(e)`, `$fields(T)`, `$error("msg")`, `$assert(cond, "msg")`) are
  syntactically a `comptime-ident` callee with `call-args`. Their *arguments*
  are parsed as ordinary expressions — `$size_of(T)` and `$fields(T)` pass a
  type name that the expression grammar reads as an `IDENT`; the intrinsic
  reinterprets it as a type at evaluation time. `$type_of(e)` takes a value
  expression and produces a comptime type value.
- `$mach.*` reads are a `comptime-ident` followed by a `.`-member chain. They
  appear in `$if` conditions and as comptime initializers.
- A bare **directive** `$intrinsic(args);` (e.g. `$error("msg");`) is the
  `comptime-directive` declaration form above.

The `$mach.*` tag/path set is closed and documented in
[comptime-mach.md](comptime-mach.md); this grammar treats every `$ident`
and `.`-chain off it uniformly.

### Field projection (`v.[f]`)

`v.[f]` is a postfix form that projects the field described by a comptime
field descriptor `f` (bound by a `$each f in $fields(T)` loop) off an
instance `v`. Syntactically: `.` followed immediately by `[expr]`. It is
disambiguated from a regular member access `v.name` by the `[` lookahead:
`.` then `[` = projection; `.` then `IDENT` = member.


## Verification notes

Productions verified directly against the parser source:

- **Lexical grammar** — `lexer.mach` / `token.mach`: token set (incl.
  `KIND_BACKTICK`), operator maximal-munch, number/char/string scanning and
  escapes, comment and whitespace handling, the "keywords are `IDENT`s" model.
- **Precedence ladder** — `token.infix_precedence` / `token.is_right_assoc`
  (the table is a direct transcription; only `=` is right-associative).
- **Decorators** — `parser/decl.mach` `parse_decorators` / `parse_one_decorator`:
  leading `` `name` `` / `` `name(args)` `` clauses, closed directive set.
- **Declarations** — `parser/decl.mach`: `use`, `fwd` (incl. `pub fwd`
  rejection), `fun` (generics, params, variadic `...`, named pack `name: ...`,
  comptime `$` params, optional return type, block-or-`;` body), `rec`, `uni`,
  `val`/`var` (both annotations optional), `def`, `test`, `flags`
  (`pub`/`ext` any order/count), the decl-scope `$if`/`$or` chain, and the
  `comptime-directive` (attribute-write vs. bare directive) form.
- **Statements** — `parser/decl.mach`: `block`, `if`/`or` chain, `for`
  (optional condition), `ret`/`brk`/`cnt`/`fin`, local `val`/`var`, the
  stmt-scope `$if`/`$or` chain, `$each … in … { }`, and `expr-stmt`.
- **Expressions** — `parser/expr.mach`: prefix atoms, all five unary
  prefix operators (`-`, `!`, `~`, `?`, `@`), the postfix chain
  (call with optional `...` spread on arguments, generic-call, index,
  member, field projection `.[f]`, cast), struct/array literals and the
  typed-literal lookahead, the generic-call-vs-index `[` disambiguation,
  and `comptime-ident`.
- **Types** — `parser/expr.mach`: `*T`, `[N]T`, `fun(...) R` (with variadic,
  pack `name: ...`, and optional return), anonymous `rec {...}` / `uni {...}`,
  and named types with generic args / dotted paths.
- **Inline asm** — `parser/iasm.mach`: mandatory ISA tag, raw brace-balanced
  body, no operand/clobber list.

Doc-only (intended surface, not a distinct parser production):

- The concrete escape sets (`\n \t \r \\ \' \0 \xHH`, plus `\"` for
  strings) — the lexer only treats `\` as "consume next char"; the actual
  escape set is decoded in `comptime.eval_lit_char` / string lowering and
  documented in [literals.md](literals.md).
- The closed ISA-tag set (`x86_64`, `aarch64`) and the closed `$mach.*`
  tag/path set — the parser accepts any `IDENT` / `$`-chain; the closed
  sets are enforced later (see [asm.md](asm.md),
  [comptime-mach.md](comptime-mach.md)).
- The closed intrinsic set (`$size_of`, `$align_of`, `$offset_of`,
  `$type_of`, `$fields`, `$error`, `$assert`) — syntactically
  indistinguishable from any other `comptime-ident` call.
- The closed decorator directive set (`symbol`, `library`, `inline`, `align`,
  `section`) — the parser accepts any `IDENT` after `` ` ``; sema enforces
  the closed set.

Divergences flagged inline:

- `$` comptime marker is grammatically accepted on named `rec`/`uni` fields
  (shared `typed-name`), though [fun.md](fun.md) scopes comptime value
  parameters to functions only.

No production above is left unverified against the parser; nothing here is
invented. The only "approximate" surface is the asm body, which is
deliberately *not* a token grammar (it is raw text by design).


## See also

- [literals.md](literals.md) — literal forms and escapes
- [types.md](types.md) — type semantics
- [operators.md](operators.md) — operator semantics and precedence prose
- [expressions.md](expressions.md) — expression composition
- [statements.md](statements.md) — control-flow semantics
- [fun.md](fun.md) — function declarations, generics, variadics, comptime params
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
- [comptime-mach.md](comptime-mach.md) — `$mach.*` namespace
- [asm.md](asm.md) — inline assembly
