# Lexical Structure

This document describes how Mach source text is broken into tokens: whitespace handling, comments, identifiers, keywords, literals, punctuation, and operators.

- See also: [Literals](literals.md), [Expressions and Operators](expressions-and-operators.md), [Types](types.md)

## Source text and whitespace

- Mach source is a sequence of Unicode code points; tokens are separated by ASCII whitespace (space, tab, newline, carriage return).
- Whitespace has no semantic meaning beyond separating tokens.
- Line endings do not terminate statements; the semicolon character `;` is the statement terminator.

## Comments

- Line comments begin with `#` and continue to the end of the line.
- Comments are ignored by the language (they do not produce tokens).
- Block/multi-line comment syntax is not defined; use consecutive line comments for multi-line notes.

Example:
- `# this is a comment`

## Identifiers

Identifiers name variables, constants, types, functions, methods, fields, modules, etc.

- Syntax:
  - The first character must be an ASCII letter (`A–Z` or `a–z`) or underscore (`_`).
  - Subsequent characters may be ASCII letters, ASCII digits (`0–9`), or underscores (`_`).
- Identifiers are case-sensitive.
- The dot `.` is not part of identifiers; it separates identifiers in qualified names (e.g., `alias.Type` or `module.submodule`).

Examples (valid):
- `x`, `value_2`, `_internal`, `HTTPHeader`

Examples (invalid):
- `2fast` (cannot start with a digit), `name-with-dash` (`-` not allowed), `a.b` (contains a dot)

### Keywords (reserved words)

The following tokens are reserved keywords and cannot be used as identifiers:

- Declarations and visibility: `use`, `ext`, `def`, `pub`, `rec`, `uni`, `val`, `var`, `fun`
- Flow control: `ret`, `if`, `or`, `for`, `cnt`, `brk`
- Assembly: `asm`
- Null literal: `nil`

## Tokens: punctuation and operators

Mach uses the following single- and multi-character tokens. Whitespace may separate tokens; when ambiguous, the longest valid token is formed.

- Delimiters: `(` `)` `[` `]` `{` `}`
- Separators: `,` `;` `.` `:` `::`
- Special symbols: `$` `@` `?` `_` `...`
- Arithmetic: `+` `-` `*` `/` `%`
- Bitwise: `~` `&` `|` `^` `<<` `>>`
- Comparison: `<` `<=` `>` `>=` `==` `!=`
- Logical: `&&` `||`
- Assignment: `=`

Notes:
- `::` is the cast operator in expressions.
- `?` is the address-of unary operator; `@` is the dereference unary operator.
- `...` (ellipsis) appears in function parameter lists to denote variadic functions and as an expression form in varargs contexts.
- `_` (underscore) is a standalone token used where the grammar allows it (for example, an unbounded array size), and it is also permitted within numeric literals as a digit separator.
- The character `/` is the division operator. In source, a backslash `\` is treated equivalently to `/` by the lexer; use `/` in source code for clarity and portability.

## Literals overview

Mach has the following literal kinds:

- Integer literals
- Floating-point literals
- Character literals
- String literals
- The null literal `nil`

Literal tokens carry their textual form; their exact types are determined by context during semantic analysis. This section summarizes their lexical form.

### Integer literals

- Decimal (no prefix): e.g., `0`, `42`, `10_000`
- Hexadecimal (prefix `0x` or `0X`): e.g., `0xff`, `0XDEAD_BEEF`
- Binary (prefix `0b` or `0B`): e.g., `0b1010_0011`
- Octal (prefix `0o` or `0O`): e.g., `0o755`

Rules:
- Only digits appropriate to the base are allowed after the prefix.
- Underscores (`_`) are permitted between digits as visual separators and have no semantic meaning.
- Prefixed integers (hex/bin/oct) are always integers; they cannot include a decimal point.

Examples:
- `123`, `0x1f`, `0b0101_1100`, `0o644`, `1_000_000`

### Floating-point literals

- Decimal notation with a single dot: e.g., `3.14`, `0.0`, `10_000.5`
- At least one digit must appear before or after the dot.
- Underscores (`_`) are permitted between digits.
- Exponential notation (e.g., `1e6`) is not part of the lexical syntax.
- Base prefixes (`0x`, `0b`, `0o`) are not valid for floats.

Examples:
- `1.0`, `0.25`, `12_345.678_9`, `.5` is not valid; use `0.5`.

### Character literals

- Enclosed in single quotes: `'a'`
- May contain a single character or one of the supported escape sequences.
- Supported escapes: `\'` `\"` `\\` `\n` `\t` `\r` `\0`

Examples:
- `'x'`, `'\n'`, `'\"'`

### String literals

- Enclosed in double quotes: `"text"`
- May contain any character; strings can span lines.
- Supported escapes: `\'` `\"` `\\` `\n` `\t` `\r` `\0`
- There is no raw-string literal syntax.

Examples:
- `"hello"`, `"line1\nline2"`, `"quote: \""`

### Null literal

- `nil` denotes the null literal.

## Tokenization summary

- The lexer skips whitespace and line comments, then emits the next token.
- When multiple tokenizations are possible, multi-character tokens take precedence (e.g., `==` over `=` followed by `=`).
- Numeric literals are recognized greedily:
  - If a number starts with `0x`, `0b`, or `0o`, it is parsed as an integer in the corresponding base.
  - Otherwise, a sequence of digits may optionally be followed by a single dot `.` and further digits to form a float; without the dot, it is an integer.
- Identifiers are recognized when the first character is a letter or underscore; otherwise, punctuation and operators are recognized per the lists above.

For operator precedence and associativity, see [Expressions and Operators](expressions-and-operators.md). For the precise grammar forms that use `_`, `...`, `$`, and `::`, see [Grammar](grammar.md) and the topic-specific pages linked from the [language tour](language-tour.md).