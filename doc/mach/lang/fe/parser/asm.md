# mach.lang.fe.parser.asm

Inline assembly. Parses the optional ISA tag, the operand list
(`name: in`, `name: out`, `name: clobber`, with optional `= source`
rename), and collects the raw body text by tracking brace depth through
the already-lexed token stream.

## Public surface

### `parse_asm`

```mach
pub fun parse_asm(p: *parser.Parser) id.StmtId
```

Parses a complete `asm [ISA]? (operand_list)? { body }` form starting
at the `asm` keyword. Builds a [`StmtAsm`](../ast/stmt.md#stmtasm) and
appends it via [`parser.push_stmt`](state.md#push_stmt).

| Param | Type                                          | Description                                          |
|-------|-----------------------------------------------|------------------------------------------------------|
| p     | [`*parser.Parser`](state.md#parser)           | Parser state; current token must be the IDENT `asm`. |

Returns the [`StmtId`](../ast/id.md#stmtid) of the resulting
[`STMT_KIND_ASM`](../ast/stmt.md#constants) node.

## Grammar

```
asm_stmt    := "asm" isa? operand_list? "{" raw_body "}"
isa         := IDENT                              # e.g. x86_64, arm64
operand_list:= "(" (operand ("," operand)* ","?)? ")"
operand     := IDENT ":" role ("=" IDENT)?
role        := "in" | "out" | "clobber"           # "=" rename rejected on clobber
raw_body    := <bytes between matched braces; not tokenised further>
```

`isa` is optional. When absent the body is parsed by the middle-end
as portable MASM; when present it scopes the body to the named target
ISA.

Operands have three roles:

| Role     | Meaning                                                                  |
|----------|--------------------------------------------------------------------------|
| `in`     | Input operand. May be renamed via `=` from a mach-level identifier.      |
| `out`    | Output operand. May be renamed via `=` from a mach-level identifier.     |
| `clobber`| Clobbered register. Rejects the `=` rename.                              |

When the optional rename is present, the body sees `name` while the
surrounding mach code reads/writes via the renamed identifier. When
absent, the body-name *is* the mach identifier.

## Body collection

The `{ ... }` body is *not* parsed as mach syntax. The recogniser:

1. Consumes the leading `{` and records `body.offset = byte_after_lbrace`.
2. Tracks `depth = 1` and walks tokens.
3. Increments `depth` on every `KIND_LBRACE`, decrements on every `KIND_RBRACE`.
4. When `depth == 0` after a `}`, stops; `body.len = closing.offset - body.offset` and the closing `}` becomes `end_span`.
5. If EOF is reached first, emits `"unterminated asm body"` and sets the body span to whatever was collected so far.

This means the body **must** contain matched braces. Internal `{...}`
pairs are passed through to the assembly layer untouched; their
parsing is the middle-end's responsibility.

## Internal helpers

File-private; listed for reference.

| Function              | Role                                                                     |
|-----------------------|--------------------------------------------------------------------------|
| `parse_operand_list`  | Reads `( operand , ... )`. Returns the operand count appended to [`p.ast.asm_operands`](../ast.md#ast). |
| `parse_operand`       | Reads a single `name : role [ = source ]` entry; appends an [`AsmOperand`](../ast/stmt.md#asmoperand) on success. |
| `collect_body`        | Brace-tracked raw-text scan; out-parameters fill the body and closing-brace spans. |

## Error recovery

`parse_operand_list` calls [`sync_to`](state.md#sync_to) on
`KIND_RPAREN` / `KIND_LBRACE` / `KIND_EOF` when an operand parse fails,
so the rest of the form (or the body) can still be reached.

`collect_body` does not recover from EOF — once the input is exhausted
without a closing brace, the body span is whatever was scanned and the
parse continues at EOF.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.string`,
`std.types.result`, [`mach.lang.fe.token`](../token.md),
[`mach.lang.fe.ast`](../ast.md),
[`mach.lang.fe.ast.id`](../ast/id.md),
[`mach.lang.fe.ast.stmt`](../ast/stmt.md),
[`mach.lang.fe.parser.state`](state.md).
