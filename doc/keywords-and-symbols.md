# Keywords and Symbols

This reference lists the reserved keywords and token symbols in the Mach language, with brief descriptions and links to relevant sections of the language reference.

Related topics:
- [Lexical Structure](lexical-structure.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Types](types.md)
- [Variables and Constants](variables.md)
- [Functions and Methods](functions-and-methods.md)
- [Control Flow](control-flow.md)
- [Modules and Visibility](modules-and-visibility.md)
- [Records and Unions](records-and-unions.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Compile-time Features](compile-time.md)
- [Inline Assembly](inline-assembly.md)

---

## Reserved Keywords

These identifiers are reserved and cannot be used as names.

- `use` — Import another module’s public symbols. See [Modules and Visibility](modules-and-visibility.md).
- `ext` — Declare an external (foreign) symbol binding. See [Interoperability](interoperability.md).
- `def` — Create a type alias. See [Types](types.md).
- `pub` — Make a top-level declaration public to importers. See [Modules and Visibility](modules-and-visibility.md).
- `rec` — Define a record (struct-like) type or literal. See [Records and Unions](records-and-unions.md).
- `uni` — Define a union type or literal. See [Records and Unions](records-and-unions.md).
- `val` — Declare an initialized constant (immutable). See [Variables and Constants](variables.md).
- `var` — Declare a variable (mutable). See [Variables and Constants](variables.md).
- `fun` — Declare a function or method; also introduces function types. See [Functions and Methods](functions-and-methods.md).
- `ret` — Return from a function (with or without a value). See [Control Flow](control-flow.md).
- `if` — Start a conditional chain. See [Control Flow](control-flow.md).
- `or` — Continue a conditional chain (else-if or else when no condition). See [Control Flow](control-flow.md).
- `for` — Introduce a loop with an optional condition. See [Control Flow](control-flow.md).
- `cnt` — Continue the innermost loop. See [Control Flow](control-flow.md).
- `brk` — Break out of the innermost loop. See [Control Flow](control-flow.md).
- `asm` — Inline assembly statement. See [Inline Assembly](inline-assembly.md).
- `nil` — The null literal. See [Literals](literals.md).

---

## Delimiters and Separators

- `(` `)` — Parentheses; grouping and function calls. See [Expressions and Operators](expressions-and-operators.md).
- `[` `]` — Brackets; array/slice types and indexing. See [Arrays and Slices](arrays-and-slices.md).
- `{` `}` — Braces; blocks and composite (record/union/array) literals. See [Control Flow](control-flow.md) and [Records and Unions](records-and-unions.md).
- `,` — Comma; separates parameters, arguments, fields, and list elements.
- `;` — Semicolon; statement terminator. See [Variables and Constants](variables.md).
- `.` — Dot; field access and qualified names with module aliases. See [Expressions and Operators](expressions-and-operators.md) and [Modules and Visibility](modules-and-visibility.md).
- `:` — Colon; separates names from types (e.g., parameters, fields, aliases). See [Types](types.md).

---

## Special Symbols

- `$` — Introduces a compile-time expression or statement (e.g., `$if`, `$size_of(Type)`). See [Compile-time Features](compile-time.md).
- `::` — Cast operator (convert expression to a target type). See [Expressions and Operators](expressions-and-operators.md).
- `...` — Ellipsis; marks variadic function parameters and is used in varargs contexts. See [Functions and Methods](functions-and-methods.md).
- `_` — Underscore token; permitted inside numeric literals as a digit separator. See [Lexical Structure](lexical-structure.md).
- `?` — Address-of unary operator (`?expr` yields `*T` for assignable `expr: T`). See [Pointers and Memory](pointers-and-memory.md).
- `@` — Dereference unary operator (`@expr` yields `T` for `expr: *T`). See [Pointers and Memory](pointers-and-memory.md).

---

## Operators

Arithmetic
- `+` `-` `*` `/` `%` — Addition, subtraction, multiplication, division, remainder (numeric types). See [Expressions and Operators](expressions-and-operators.md).


Bitwise (integers)

- `~` — Bitwise NOT (unary); inverts all bits.
- `&` `|` `^` — AND, OR, XOR (binary).
- `<<` `>>` — Shift left, shift right.


Comparison (results are `u8`)
- `==` `!=` — Equality, inequality.
- `<` `>` `<=` `>=` — Relational comparisons.

Logical (short-circuit; operands/results are `u8`)
- `&&` `||` — Logical AND, logical OR.

Assignment
- `=` — Assign right-hand value to left-hand lvalue. See [Variables and Constants](variables.md) and [Expressions and Operators](expressions-and-operators.md).

Postfix and indexing
- `()` — Function/method call.
- `[]` — Index into arrays and slices.

Casts and typed literals
- `::` — Cast expression. Also appears in typed composite literal context (postfix chain). See [Expressions and Operators](expressions-and-operators.md) and [Types](types.md).

Notes:
- Operator precedence and associativity are defined in [Expressions and Operators](expressions-and-operators.md).
- Indexing a slice (`[]T`) is bounds-checked at runtime; fixed arrays (`[N]T`) are indexable by `0..N-1`. See [Arrays and Slices](arrays-and-slices.md).

---

## Composite and Type Syntax Tokens

- `rec` / `uni` with `{ ... }` — Record/union type definitions and literals with named fields. See [Records and Unions](records-and-unions.md).
- `[N]T` — Fixed-size array type; `N` is a compile-time integer. See [Arrays and Slices](arrays-and-slices.md).
- `[]T` — Slice (runtime-sized fat pointer: `.data`, `.len`). See [Arrays and Slices](arrays-and-slices.md).
- `*T` — Pointer to `T`. See [Pointers and Memory](pointers-and-memory.md).
- `fun(T1, ..., Tn) Ret` — Function type (return type optional). Variadic form ends with `...`. See [Functions and Methods](functions-and-methods.md).
- `alias.Type` — Qualified type name using a module alias. See [Modules and Visibility](modules-and-visibility.md).

---

## Control-Flow Tokens (Summary)

- `if (cond) { ... } or (cond) { ... } or { ... }` — Conditional chains. See [Control Flow](control-flow.md).
- `for (cond) { ... }` / `for { ... }` — Loop with optional condition. See [Control Flow](control-flow.md).
- `brk;` / `cnt;` / `ret;` or `ret expr;` — Break/continue/return statements. See [Control Flow](control-flow.md).

---

## Inline Assembly

- `asm { ... }` — Embed target-specific assembly as a statement; trailing `;` is optional. See [Inline Assembly](inline-assembly.md).

---

## Compile-time ($) Tokens (Summary)

- `$if (...) { ... } or { ... }` — Compile-time selection (one branch kept). See [Compile-time Features](compile-time.md).
- `$size_of(Type)` `$align_of(Type)` `$offset_of(Type, field)` `$type_of(X)` — Compile-time intrinsics. See [Compile-time Features](compile-time.md).
- `$Symbol.attribute = value` — Set symbol attributes (e.g., `$main.symbol = "main"`). See [Compile-time Features](compile-time.md).

---