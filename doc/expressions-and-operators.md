# Expressions and Operators

This document defines the Mach expression model: primary, postfix, unary, binary, and assignment expressions; precedence and associativity; calls, indexing, field access, and casts. Cross‑references:
- [Lexical Structure](lexical-structure.md)
- [Types](types.md)
- [Variables and Constants](variables.md)
- [Functions and Methods](functions-and-methods.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Records and Unions](records-and-unions.md)
- [Generics](generics.md)

Mach is expression-oriented: many constructs yield values, and expressions can appear where values are required (e.g., in initializers, arguments, return statements).

## Primary expressions

- Identifiers: `name`
- Literals: integer, float, char, string, `nil` (see [Literals](literals.md))
- Parenthesized expressions: `(expr)` — groups and determines evaluation order

Examples:
- `x`, `42`, `"hi"`, `nil`, `(a + b)`

## Postfix expressions

Postfix operators and constructs bind tightly and apply after a primary (or another postfix expression). They associate left-to-right as written.

- Call: `expr(args...)`
  - Example: `f(a, b)`, `obj.method(x)`
  - Generic call: `expr[TypeArgs](args...)` (see [Generics](generics.md))
- Index: `expr[index]`
  - Works for fixed arrays `[N]T` and slices `[]T`
  - For slices, indexing is bounds-checked at runtime; for fixed arrays, indexing computes an element address without a runtime bounds check
- Field access: `expr.field`
  - Works with records and unions; for slices, fields `data` (pointer) and `len` (length) are available
- Cast: `expr :: Type`
  - Converts the value to the target type if allowed (see Casts below)
- Typed composite literal:
  - Array/slice: `Type{ elem0, elem1, ... }`, e.g., `[]u8{ 1, 2, 3 }`, `[3]u8{ 1, 2, 3 }`
  - Record/union: `Type{ name0: value0, name1: value1, ... }`, e.g., `Point{ x: 0.0, y: 1.0 }`
  - Generic forms: `GenericType[Args]{ ... }`, e.g., `Box[u64]{ value: 42 }`

Method calls
- `value.method(args...)` is sugar for a function call where the receiver is passed as a first argument.
- When a receiver’s declared type differs by one level of indirection, the language automatically takes the address or dereferences as needed:
  - If the method expects `*T` but you call on `T`, the address of the receiver is taken.
  - If the method expects `T` but you call on `*T`, the receiver is dereferenced.

## Unary expressions

Unary operators apply to a single operand and bind tighter than any binary operator. They associate right-to-left.

- Logical not: `!expr`
  - Yields a `u8` (0 for false, nonzero for true). Operands are typically `u8`.
- Unary plus/minus: `+expr`, `-expr`
  - Numeric-only; preserves or negates the operand
- Bitwise not: `~expr`
  - Bitwise complement (integer types only); inverts all bits
- Address-of: `?expr`
  - Requires an assignable operand (lvalue), yields `*T` for operand type `T`
- Dereference: `@expr`
  - If `expr` is `*T`, yields a `T`

Examples:
- `!(x == 0)`, `-n`, `~flags`, `?var`, `@ptr`

## Binary expressions

Binary operators combine two operands. Except for assignment, operators associate left-to-right. The result types depend on the operator and operands as noted.

Arithmetic
- `+`, `-`, `*`, `/`, `%`
- Apply to numeric types (`i*`, `u*`, `f*`)
- Mixed-type arithmetic requires explicitly matching types via `::` casts

Bitwise (integers only)
- `&` (and), `|` (or), `^` (xor), `<<` (shift left), `>>` (shift right)

Comparisons (yield `u8`)
- Equality: `==`, `!=`
- Relational: `<`, `>`, `<=`, `>=`
- Comparison results are `u8` (0/1). Comparisons require type-compatible operands.

Logical (short-circuit, `u8` logic)
- `&&` (and), `||` (or)
- Short-circuit evaluation: the right-hand side is evaluated only if needed
- Operands are typically `u8`; the result is `u8`

Assignment
- `=` (right-associative)
- LHS must be assignable (lvalue). RHS is evaluated and assigned to LHS if type-compatible.
- Assignable LHS forms include:
  - A `var` variable
  - Pointer dereference: `@ptr`
  - Field of an assignable object: `obj.field`
  - Indexed element: `array[i]`, `slice[i]`

Examples:
- `a = b + 1`
- `@p = 7`
- `v.x = v.x + 1`
- `buf[i] = 0`

## Casts (`::`)

The cast operator converts a value to a target type when permitted by the type system.

Syntax:
- `expr :: Type`
- Type may be qualified (`alias.Type`) and/or generic (`Name[Args]`)

Rules (high level):
- Numeric casts convert among integer and floating types as allowed.
- Pointer casts follow pointer compatibility rules.
- Arrays and slices:
  - A slice value `[]T` can decay to a data pointer `*T` when explicitly cast via `::`.
- In calls, argument passing from slices to pointer parameters may decay implicitly as described below.

Examples:
- `(a) :: i64`
- `?x :: *u8`
- `(buf) :: []u8`

See [Types](types.md) for categories and [Literals](literals.md) for literal cast examples.

## Function calls

Call forms:
- `f(a, b, c)`
- Generic: `f[T1, T2](args...)`

Arguments
- Arguments are evaluated left-to-right.
- Variadic functions end their type with `...`, e.g., `fun(*u8, ...) i32`. Call sites pass extra arguments as usual.

Implicit array/slice decay for calls
- When passing a value of slice type `[]T` to a parameter whose type is a pointer `*U` (or an untyped pointer), the data pointer from the slice may be used for the argument if compatible. This enables passing `slice.data` implicitly when the parameter expects a pointer.
- For varargs calls, a slice argument of type `[]T` decays to its data pointer for the corresponding vararg position when appropriate.

Return values
- Functions may return a value or no value (if the return type is omitted in the declaration). Use `ret;` or `ret expr;` accordingly.

See [Functions and Methods](functions-and-methods.md).

## Indexing

- `array[i]` or `slice[i]`
- For fixed arrays `[N]T`, indexing computes an element address; bounds are not checked at runtime.
- For slices `[]T`, indexing is bounds-checked at runtime; accessing out of range triggers a runtime failure.
- The result of indexing is assignable when the base is assignable.

Examples:
- `val x = a[0]`
- `s[i] = 0`

## Field access

- `expr.field`
- Records and unions expose declared fields by name.
- Slices expose:
  - `data`: pointer to first element
  - `len`: length of the slice (u64)

Examples:
- `p.x`, `u.payload`, `bytes.len`, `bytes.data`

## Lvalues (assignable expressions)

Assignable forms:
- A `var` variable
- `@ptr`
- `obj.field` if `obj` is assignable
- `arr[i]` / `slice[i]` if `arr`/`slice` is assignable

Non-assignable forms include `val` bindings, literals, and pure rvalues (e.g., `a + b`).

## Evaluation order

- Function arguments are evaluated left-to-right.
- Postfix chains evaluate left-to-right at the same precedence level.
- Logical `&&` and `||` short-circuit.

## Operator precedence and associativity

From lowest (parsed last) to highest (parsed first):

1. Assignment:
   - `=` (right-associative)
2. Logical OR:
   - `||`
3. Logical AND:
   - `&&`
4. Bitwise OR:
   - `|`
5. Bitwise XOR:
   - `^`
6. Bitwise AND:
   - `&`
7. Equality:
   - `==`, `!=`
8. Relational:
   - `<`, `>`, `<=`, `>=`
9. Shifts:
   - `<<`, `>>`
10. Additive:
    - `+`, `-`
11. Multiplicative:
    - `*`, `/`, `%`
12. Unary:
    - `!`, `+` (unary), `-` (unary), `~` (bitwise not), `?` (address-of), `@` (dereference)
    - Right-associative
13. Postfix:
    - Call `()`, Index `[]`, Field `.`, Cast `::`, Typed literal `{...}`
    - Left-to-right

Parentheses `( ... )` can be used to override precedence.

## Examples

Basic expressions
```
val a: u64 = 10;
val b: u64 = 20;
val c: u64 = a + b * 2;
```

Unary, pointers
```
var x: u64 = 1;
var p: *u64 = ?x;
@p = @p + 41;           # x becomes 42
```

Casts
```
val i: i64 = (a) :: i64;
val q: *u8 = (?x) :: *u8;
```

Calls, generics, and method sugar
```
fun add(a: u64, b: u64) u64 { ret a + b; }
val s: u64 = add(2, 3);

rec Counter { v: u64; }
fun (c: *Counter) inc() { c.v = c.v + 1; }

var c: Counter; c.v = 0;
c.inc();                 # auto address-of to match *Counter receiver
```

Indexing and fields
```
val a3: [3]u8 = [3]u8{ 1, 2, 3 };
val first: u8 = a3[0];

val s: []u8 = []u8{ 10, 20, 30 };
val n: u64  = s.len;
val d: *u8  = s.data;
```

Logical and comparisons
```
val ok: u8 = (a == b) || (c > 0);
if (ok) { /* ... */ } or { /* ... */ }
```

Assignment forms
```
var y: u64 = 0;
y = 1;

var ptr: *u64 = ?y;
@ptr = 2;

rec P { x: u64; }
var p: P;
p.x = 3;
```

## Summary

- Use postfix constructs for calls, indexing, field access, casts, and typed literals; they bind tightly.
- Unary operators include logical not, unary plus/minus, address-of, and dereference.
- Binary operators cover arithmetic, bitwise, comparisons, and logical connectives; assignment is right-associative.
- Casts are explicit with `::`; slices offer `.data` and `.len` and can decay to pointers in casts and calls when appropriate.
- Method calls are syntactic sugar over functions with a receiver; auto address-of/deref is applied to match the receiver’s parameter type.