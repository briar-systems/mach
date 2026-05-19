# mach.lang.fe.comptime

Bounded compile-time evaluator. Consumed by [`resolve`](resolve.md) and
[`driver`](../driver.md) to evaluate `$if` predicates against
compile-time constants (`$mach.*` target parameters plus user-
registered `pub val` decls). Scope is intentionally narrow: literals,
references to comptime-known constants, and arithmetic / comparison /
logical / bitwise / shift operators over them. Out of scope: function
calls, loops, type-introspection intrinsics, casts.

## Types

### `CTKind`

```mach
pub def CTKind: u8;
```

Discriminator for [`CTValue.data`](#ctvalue). See
[Constants](#constants) for the enumerated values.

### `CTValue`

```mach
pub rec CTValue {
    kind: CTKind;
    data: uni {
        i: i64;
        f: f64;
        b: bool;
        s: intern.StrId;
    };
}
```

A tagged comptime-evaluated value.

| Field | Type                                  | Description                                  |
|-------|---------------------------------------|----------------------------------------------|
| kind  | [`CTKind`](#ctkind)                   | Which [`CT_KIND_*`](#constants) variant is active. |
| data  | `uni { … }`                           | Kind-specific payload.                       |

| `data` variant | Type                              | Active when `kind` is …  |
|----------------|-----------------------------------|--------------------------|
| i              | `i64`                             | `CT_KIND_INT`            |
| f              | `f64`                             | `CT_KIND_FLOAT`          |
| b              | `bool`                            | `CT_KIND_BOOL`           |
| s              | [`intern.StrId`](../intern.md#strid) | `CT_KIND_STR`         |

### `NamedConst`

```mach
pub rec NamedConst {
    name:  intern.StrId;
    value: CTValue;
}
```

A named comptime constant. Produced when a comptime-evaluable val decl
is [`register`](#register)ed with a [`ComptimeCtx`](#comptimectx) for
later reference by its bare identifier.

| Field | Type                                          | Description                          |
|-------|-----------------------------------------------|--------------------------------------|
| name  | [`intern.StrId`](../intern.md#strid)          | Interned identifier.                 |
| value | [`CTValue`](#ctvalue)                         | Pre-evaluated constant.              |

### `ComptimeCtx`

```mach
pub rec ComptimeCtx {
    alloc: *Allocator;

    target_os_id:    u32;
    target_arch_id:  u32;
    pointer_width:   u32;
    compiler_name:   intern.StrId;
    compiler_ver:    intern.StrId;

    user_consts:     *NamedConst;
    user_const_len:  u32;
    user_const_cap:  u32;
}
```

Per-module evaluation context. Target fields are seeded once at
context creation by the [driver](../driver.md). `user_consts` grows as
[`register`](#register) is called.

| Field           | Type                                              | Description                                                |
|-----------------|---------------------------------------------------|------------------------------------------------------------|
| alloc           | `*Allocator`                                      | Backs the `user_consts` array.                             |
| target_os_id    | `u32`                                             | OS id for `$mach.build.target.os.id`.                      |
| target_arch_id  | `u32`                                             | Arch id for `$mach.build.target.arch.id`.                  |
| pointer_width   | `u32`                                             | Pointer width in bytes for `$mach.build.target.pointer_width`.|
| compiler_name   | [`intern.StrId`](../intern.md#strid)              | Compiler name for `$mach.compiler.name`.                   |
| compiler_ver    | [`intern.StrId`](../intern.md#strid)              | Compiler version for `$mach.compiler.version`.             |
| user_consts     | [`*NamedConst`](#namedconst)                      | Registered comptime constants.                             |
| user_const_len  | `u32`                                             | Number of constants stored.                                |
| user_const_cap  | `u32`                                             | Allocated slots in `user_consts`.                          |

## Constants

```mach
pub val CT_KIND_INT:   CTKind = 0;
pub val CT_KIND_FLOAT: CTKind = 1;
pub val CT_KIND_BOOL:  CTKind = 2;
pub val CT_KIND_STR:   CTKind = 3;
```

[`CTKind`](#ctkind) values.

| Constant         | Value | Payload     | Notes                            |
|------------------|-------|-------------|----------------------------------|
| `CT_KIND_INT`    | 0     | `i: i64`    | Integer literal or operator result. |
| `CT_KIND_FLOAT`  | 1     | `f: f64`    | Float literal or operator result.|
| `CT_KIND_BOOL`   | 2     | `b: bool`   | Logical or comparison result.    |
| `CT_KIND_STR`    | 3     | `s: StrId`  | String literal (interned).       |

```mach
val INITIAL_CONST_CAP: u32 = 8;
```

Starting capacity for [`ComptimeCtx.user_consts`](#comptimectx). The
array doubles on overflow.

## Functions

### `init`

```mach
pub fun init(
    alloc:           *Allocator,
    target_os_id:    u32,
    target_arch_id:  u32,
    pointer_width:   u32,
    compiler_name:   intern.StrId,
    compiler_ver:    intern.StrId,
) ComptimeCtx
```

Constructs an empty [`ComptimeCtx`](#comptimectx) with target
parameters seeded. `user_consts` starts nil; grows on first
[`register`](#register).

| Param           | Type                                  | Description                                  |
|-----------------|---------------------------------------|----------------------------------------------|
| alloc           | `*Allocator`                          | Allocator backing the constants array.       |
| target_os_id    | `u32`                                 | OS id (see [target.os](../target/os.md)).    |
| target_arch_id  | `u32`                                 | Arch id (see [target.isa](../target/isa.md)).|
| pointer_width   | `u32`                                 | Pointer width in bytes.                      |
| compiler_name   | [`intern.StrId`](../intern.md#strid)  | Interned compiler name string.               |
| compiler_ver    | [`intern.StrId`](../intern.md#strid)  | Interned compiler version string.            |

Returns the populated [`ComptimeCtx`](#comptimectx).

### `dnit`

```mach
pub fun dnit(c: *ComptimeCtx)
```

Releases the constants array and clears state. `nil` is a no-op.

| Param | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| c     | [`*ComptimeCtx`](#comptimectx)        | Context to tear down. `nil` is a no-op. |

### `register`

```mach
pub fun register(c: *ComptimeCtx, name: intern.StrId, value: CTValue) Result[bool, str]
```

Appends a [`NamedConst`](#namedconst) `{ name, value }` to
[`c.user_consts`](#comptimectx). Errors on allocation failure.

| Param | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| c     | [`*ComptimeCtx`](#comptimectx)        | Target context.                      |
| name  | [`intern.StrId`](../intern.md#strid)  | Interned identifier of the constant. |
| value | [`CTValue`](#ctvalue)                 | Pre-evaluated value.                 |

Returns `true` on success, or an allocation error.

### `lookup`

```mach
pub fun lookup(c: *ComptimeCtx, name: intern.StrId) Option[*NamedConst]
```

Linear-scans [`c.user_consts`](#comptimectx) for a matching `name`.

| Param | Type                                  | Description                          |
|-------|---------------------------------------|--------------------------------------|
| c     | [`*ComptimeCtx`](#comptimectx)        | Context to search.                   |
| name  | [`intern.StrId`](../intern.md#strid)  | Interned identifier to find.         |

Returns `some(*NamedConst)` when registered, `none` otherwise.

### `eval`

```mach
pub fun eval(
    c:        *ComptimeCtx,
    a:        *ast.Ast,
    source:   str,
    e:        id.ExprId,
    interner: *intern.Interner,
) Result[CTValue, str]
```

Evaluates a comptime expression in the given context. Dispatches on
the expression's [`ExprKind`](../ast/expr.md#exprkind); unsupported
kinds (call, index, cast, array literal, struct literal, `nil`, error)
return an explicit error describing the rejection.

| Param    | Type                                          | Description                                              |
|----------|-----------------------------------------------|----------------------------------------------------------|
| c        | [`*ComptimeCtx`](#comptimectx)                | The comptime context.                                    |
| a        | [`*ast.Ast`](../ast.md#ast)                   | The ast that owns `e`.                                   |
| source   | `str`                                         | Source text covering `e` (used for span lookup).         |
| e        | [`id.ExprId`](../ast/id.md#exprid)            | Expression id to evaluate.                               |
| interner | [`*intern.Interner`](../intern.md#interner)   | Shared string interner; used to compare identifier text via [`StrId`](../intern.md#strid). |

Returns the evaluated [`CTValue`](#ctvalue), or an error message
describing why evaluation failed.

## Evaluation rules

### Supported expression kinds

| `EXPR_KIND_*`          | Behaviour                                                                  |
|------------------------|----------------------------------------------------------------------------|
| `LIT_INT`              | Decode the literal's source text per [lexer numeric rules](lexer.md#numeric-literals); emit `CT_KIND_INT`. |
| `LIT_FLOAT`            | Decode and emit `CT_KIND_FLOAT`.                                           |
| `LIT_CHAR`             | Decode escape sequences; emit `CT_KIND_INT` (i64 of the byte).             |
| `LIT_STR` / `LIT_ZSTR` | Decode escapes; intern the contents; emit `CT_KIND_STR`.                   |
| `BINARY`               | Recurse on operands; dispatch on [`BinOp`](../ast/expr.md#binop) via [operator table](#operators). |
| `UNARY`                | Recurse on operand; apply unary op via [operator table](#operators).       |
| `IDENT`                | Intern the identifier text; [`lookup`](#lookup) in `c.user_consts`; return its value. |
| `COMPTIME_IDENT`       | Treat as a leaf segment of `$mach.*`; see [`$mach.*` path resolution](#mach-path-resolution). |
| `MEMBER`               | Walk a leftmost chain of MEMBER → ... → COMPTIME_IDENT, collect segments, resolve via [`$mach.*` resolution](#mach-path-resolution). |

### Rejected expression kinds

| `EXPR_KIND_*`          | Returned error                                              |
|------------------------|-------------------------------------------------------------|
| `CALL`                 | `"function call not allowed in comptime context"`           |
| `INDEX`                | `"indexing not allowed in comptime context"`                |
| `CAST`                 | `"cast not allowed in comptime context"`                    |
| `ARRAY_LIT`            | `"array literal not allowed in comptime context"`           |
| `STRUCT_LIT`           | `"struct literal not allowed in comptime context"`          |
| `LIT_NIL`              | `"nil is not a comptime value"`                             |
| `ERROR`                | `"comptime evaluation of parse-error node"`                 |

### Operators

| Category   | [`BinOp`](../ast/expr.md#binop)                          | Result kind          | Notes                             |
|------------|----------------------------------------------------------|----------------------|-----------------------------------|
| Arithmetic | `BIN_ADD`, `BIN_SUB`, `BIN_MUL`, `BIN_DIV`, `BIN_MOD`    | i64 or f64           | Operands must share kind; INT/FLOAT only. Integer wraps; float follows IEEE. |
| Equality   | `BIN_EQ`, `BIN_NEQ`                                      | bool                 | Operands must share kind. Strings compare by [`StrId`](../intern.md#strid). |
| Compare    | `BIN_LT`, `BIN_LEQ`, `BIN_GT`, `BIN_GEQ`                 | bool                 | INT or FLOAT only.                |
| Logical    | `BIN_AND`, `BIN_OR`                                      | bool                 | Both operands must be `CT_KIND_BOOL`. Short-circuits before evaluating RHS. |
| Bitwise    | `BIN_BIT_AND`, `BIN_BIT_OR`, `BIN_BIT_XOR`               | i64                  | INT only.                         |
| Shift      | `BIN_SHL`, `BIN_SHR`                                     | i64                  | INT only; `BIN_SHR` is arithmetic. |
| Assign     | `BIN_ASSIGN`                                             | (error)              | Assignment is not a comptime expression. |

Unary operators:

| [`UnOp`](../ast/expr.md#unop) | Result kind | Notes                                          |
|-------------------------------|-------------|------------------------------------------------|
| `UN_NEG`                      | i64 or f64  | INT or FLOAT.                                  |
| `UN_NOT`                      | bool        | BOOL only.                                     |
| `UN_BIT_NOT`                  | i64         | INT only.                                      |
| `UN_ADDR` / `UN_DEREF`        | (error)     | Pointer operations are not comptime.           |

### `$mach.*` path resolution

A `COMPTIME_IDENT` (possibly the leftmost root of a MEMBER chain)
identifies a target attribute. The chain is collected as a stack of
[`Span`](../token.md#span)s (leaf first) up to depth 16, then matched
against the supported paths:

| Path                                            | Returns                                                   |
|-------------------------------------------------|-----------------------------------------------------------|
| `$mach.build.target.os.id`                      | `CT_KIND_INT` of [`target_os_id`](#comptimectx).          |
| `$mach.build.target.arch.id`                    | `CT_KIND_INT` of [`target_arch_id`](#comptimectx).        |
| `$mach.build.target.pointer_width`              | `CT_KIND_INT` of [`pointer_width`](#comptimectx).         |
| `$mach.compiler.name`                           | `CT_KIND_STR` of [`compiler_name`](#comptimectx).         |
| `$mach.compiler.version`                        | `CT_KIND_STR` of [`compiler_ver`](#comptimectx).          |
| `$mach.os.linux.id` / `$mach.os.darwin.id` / `$mach.os.windows.id` | `CT_KIND_INT` per [target.os](../target/os.md) ids. |
| `$mach.arch.x86_64.id` / `$mach.arch.aarch64.id`| `CT_KIND_INT` per [target.isa](../target/isa.md) ids.     |
| any other path                                  | error `"unknown $mach.* identifier"`.                     |

Single-segment `$IDENT` (no leading `$mach`) is left for user-level
comptime constants: the leaf segment is interned and
[`lookup`](#lookup)ed against `c.user_consts`. Failure to find the
identifier returns the same error as for a missing IDENT.

### Numeric literal decoding

[`eval_lit_int`](#internal-helpers) handles the four integer bases
matched by the [lexer](lexer.md#numeric-literals): decimal, `0x`, `0b`,
`0o`. Underscores are stripped before parsing. A trailing `u<bits>` or
`i<bits>` type suffix is consumed but does not change the comptime
representation (always `i64`).

[`eval_lit_float`](#internal-helpers) handles decimal floats with an
optional `f<bits>` type suffix; representation is always `f64`.

### String and char escapes

[`decode_escape`](#internal-helpers) recognises `\n`, `\t`, `\r`,
`\\`, `\"`, `\'`, `` \` ``, `\0`, and `\xNN`. Unrecognised escapes
return an error. Unicode escapes (`\u{...}`) are intentionally not
supported by the comptime evaluator.

## Internal helpers

File-private; listed for reference.

| Function                | Role                                                                       |
|-------------------------|----------------------------------------------------------------------------|
| `span_text`             | Build a fat-string view over `source[span.offset..span.offset+span.len]`. |
| `eval_lit_int`          | Decode and box an integer literal.                                         |
| `eval_lit_float`        | Decode and box a float literal.                                            |
| `eval_lit_char`         | Decode escape sequences within a char literal; box as `CT_KIND_INT`.       |
| `eval_lit_str`          | Decode escape sequences within a string literal; intern; box as `CT_KIND_STR`. |
| `intern_decoded`        | Allocate a temporary buffer for an escape-decoded string and intern it.    |
| `decode_char`           | Decode the contents of a single-character literal to a `u8`.               |
| `decode_escape`         | Decode a single `\…` escape sequence at the given pointer.                 |
| `hex_digit`             | ASCII hex character → 0..15, or negative on failure.                       |
| `contains_byte`         | Predicate: does `s` contain the byte `b`?                                  |
| `eval_binary`           | Recurse on operands and dispatch on `BinOp`.                               |
| `eval_unary`            | Recurse on operand and dispatch on `UnOp`.                                 |
| `eval_logical`          | Short-circuit `BIN_AND` / `BIN_OR`.                                        |
| `apply_equality`        | Implements `BIN_EQ` / `BIN_NEQ`.                                           |
| `apply_compare`         | Implements `<`, `<=`, `>`, `>=`.                                           |
| `apply_arith`           | Implements `+`, `-`, `*`, `/`, `%` for INT and FLOAT.                      |
| `eval_ident`            | Interns the source text and `lookup`s the matching `NamedConst`.           |
| `eval_path`             | Walks a MEMBER chain to a root `COMPTIME_IDENT`, then calls `resolve_mach_path`. |
| `comptime_ident_name`   | Strips the leading `$` from a comptime-ident span.                         |
| `resolve_mach_path`     | Table-driven match of the segment stack against the supported `$mach.*` paths. |
| `seg_is`                | Predicate: does segment N of the stack match the given string?             |
| `os_id_for` / `arch_id_for` | Maps a name span to its `target.os` / `target.isa` id constant.        |
| `int_value` / `float_value` / `bool_value` / `str_value` | `CTValue` constructors.                  |

## Dependencies

`std.types.bool`, `std.types.option`, `std.types.size`,
`std.types.string`, `std.types.zstr`, `std.types.result`,
`std.allocator`, [`mach.lang.intern`](../intern.md),
[`mach.lang.fe.ast`](../ast.md),
[`mach.lang.fe.ast.id`](../ast/id.md),
[`mach.lang.fe.ast.expr`](../ast/expr.md),
[`mach.lang.fe.token`](../token.md),
[`mach.lang.target.os`](../target/os.md),
[`mach.lang.target.isa`](../target/isa.md).
