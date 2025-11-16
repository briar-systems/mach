# Expressions

This document describes the expression system in Mach, including operators, operator precedence, expression forms, and the semantics enforced by the canonical compiler.

- [Expressions](#expressions)
	- [Expression Categories](#expression-categories)
	- [Operators](#operators)
		- [Binary Operators](#binary-operators)
		- [Unary Operators](#unary-operators)
		- [Precedence and Associativity](#precedence-and-associativity)
	- [Primary Expressions](#primary-expressions)
		- [Literals](#literals)
		- [Identifiers](#identifiers)
		- [Parenthesized Expressions](#parenthesized-expressions)
		- [Null Literal](#null-literal)
		- [Varargs Pack](#varargs-pack)
	- [Postfix Expressions](#postfix-expressions)
		- [Function Calls](#function-calls)
		- [Array Initialization](#array-initialization)
		- [Slice Initialization](#slice-initialization)
		- [Array Indexing](#array-indexing)
		- [Field Access](#field-access)
		- [Type Casting](#type-casting)
		- [Typed Literals](#typed-literals)
	- [Declaration Expressions](#declaration-expressions)
		- [Anonymous Record Literals](#anonymous-record-literals)
		- [Anonymous Union Literals](#anonymous-union-literals)
	- [Compile-time Expressions](#compile-time-expressions)
	- [Assignment](#assignment)


## Expression Categories

Expressions in Mach are organized into several categories based on their evaluation order and binding strength:

1. **Primary expressions** - atomic expressions like literals and identifiers
2. **Postfix expressions** - operations that follow their operand (calls, indexing, field access)
3. **Unary expressions** - operations with a single operand
4. **Binary expressions** - operations with two operands
5. **Assignment** - variable assignment (lowest precedence)

Expression parsing uses precedence climbing to correctly handle operator priority and associativity.


## Operators

### Binary Operators

Binary operators take two operands and combine them to produce a result. All binary operators are left-associative.

| Operator | Description              | Precedence Level |
| -------- | ------------------------ | ---------------- |
| `=`      | Assignment               | Assignment       |
| `\|\|`   | Logical OR               | Or               |
| `&&`     | Logical AND              | And              |
| `\|`     | Bitwise OR               | Bit Or           |
| `^`      | Bitwise XOR              | Bit Xor          |
| `&`      | Bitwise AND              | Bit And          |
| `==`     | Equal                    | Equality         |
| `!=`     | Not equal                | Equality         |
| `<`      | Less than                | Comparison       |
| `>`      | Greater than             | Comparison       |
| `<=`     | Less than or equal       | Comparison       |
| `>=`     | Greater than or equal    | Comparison       |
| `<<`     | Left shift               | Shift            |
| `>>`     | Right shift              | Shift            |
| `+`      | Addition                 | Term             |
| `-`      | Subtraction              | Term             |
| `*`      | Multiplication           | Factor           |
| `/`      | Division                 | Factor           |
| `%`      | Modulo/Remainder         | Factor           |

Note: The assignment operator `=` is used for variable mutation. See the [Keywords](keywords.md#var) documentation for details on variable declarations.


### Unary Operators

Unary operators take a single operand and operate on it. They bind more tightly than binary operators but less tightly than postfix operations.

| Operator | Description                          |
| -------- | ------------------------------------ |
| `!`      | Logical NOT                          |
| `-`      | Numeric negation                     |
| `~`      | Bitwise NOT                          |
| `?`      | Address-of (pointer creation)        |
| `@`      | Pointer dereference                  |

The `?` operator creates a pointer to a value, while `@` dereferences a pointer to access its value. See the [Memory](memory.md) documentation for details on pointer semantics.


### Precedence and Associativity

Operators are evaluated according to their precedence level, from lowest to highest:

1. Assignment (`=`)
2. Logical OR (`||`)
3. Logical AND (`&&`)
4. Bitwise OR (`|`)
5. Bitwise XOR (`^`)
6. Bitwise AND (`&`)
7. Equality (`==`, `!=`)
8. Comparison (`<`, `>`, `<=`, `>=`)
9. Shift (`<<`, `>>`)
10. Term (`+`, `-`)
11. Factor (`*`, `/`, `%`)
12. Unary (`!`, `-`, `~`, `?`, `@`)
13. Postfix (calls, indexing, field access, casts)
14. Primary (literals, identifiers)

All binary operators are **left-associative**, meaning `a - b - c` is evaluated as `(a - b) - c`.

Use parentheses to override default precedence:

```mach
val x: i32 = 2 + 3 * 4;     # 14 (multiplication first)
val y: i32 = (2 + 3) * 4;   # 20 (parentheses override)
```


## Primary Expressions

Primary expressions are the atomic building blocks of all expressions.


### Literals

Literal values represent fixed data directly in source code. See the [Literals](literals.md) documentation for comprehensive details on all literal forms.

```mach
val n: i32 = 42;
val x: f64 = 3.14159;
val c: u8  = 'A';
val s: *u8 = "Hello";
```


### Identifiers

Identifiers refer to named entities such as variables, functions, and types.

```mach
val x: i32 = 10;
val y: i32 = x;  # identifier refers to variable x
```


### Parenthesized Expressions

Parentheses group expressions to control evaluation order or improve readability.

```mach
val result: i32 = (a + b) * (c + d);
```


### Null Literal

The `nil` keyword represents a null pointer value. See the [Keywords](keywords.md) documentation for usage details.

```mach
val p: *u8 = nil;
```


### Varargs Pack

The `...` token represents a varargs pack expression. This is used in variadic function contexts to represent the collection of variable arguments.

```mach
fun variadic_fn(fmt: *u8, ...) {
    # `...` represents the varargs pack here
}
```


## Postfix Expressions

Postfix expressions extend a base expression with additional operations that follow it.


### Function Calls

Function call expressions invoke a function with zero or more arguments.

```mach
fun add(a: i32, b: i32) i32 {
    ret a + b;
}

val result: i32 = add(10, 20);
```

Generic functions can be called with explicit type arguments:

```mach
val result: i32 = generic_fn[i32, u64](arg1, arg2);
```

See the [Types](types.md#generics) documentation for details on generic types.


### Array Initialization

Arrays can be initialized using a typed literal syntax:

```mach
val arr:  [3]i32 = [3]i32{ 1, 2, 3 }; # initialize array
```


### Slice Initialization

Slices must be initialized using a typed literal syntax with both `data` and `len` components:

```mach
var arr:   [3]i32 = [3]i32{ 1, 2, 3 };           # initialize array
val slice: []i32  = []i32{ data: ?arr, len: 3 }; # initialize slice from array
```

- `data` is a pointer to the first element and must be of type `*T` where `T` is the element type.
- `len` is the number of elements in the slice and is not managed automatically.


### Array Indexing

Array indexing accesses an element at a specific position.

```mach
val arr:  [3]i32 = [3]i32{ 1, 2, 3 }; # initialize array
val elem: i32    = arr[1];            # access second element
```

See the [Types](types.md#arrays) documentation for details on arrays and indexing.


### Field Access

Field access retrieves a named field from a record or union.

```mach
rec Point {
    x: f32;
    y: f32;
}

val p:       Point = Point{x: 1.0, y: 2.0};
val x_coord: f32   = p.x;
```

Field access is also used to call methods on types:

```mach
fun (this: *Point) distance() f32 {
    ret 0.0;
}

val dist: f32 = p.distance();
```

See the [Types](types.md#methods) documentation for details on method syntax.


### Type Casting

The `::` operator performs an explicit type cast.

```mach
val x: i32 = 42;
val y: i64 = x::i64;  # cast i32 to i64
```

See the [Types](types.md#type-casting) documentation for details on casting rules and semantics.


### Typed Literals

Composite literals can be created by providing a type expression followed by a brace-enclosed initializer:

```mach
val p:   Point =  Point{x: 1.0, y: 2.0};
val arr: [3]i32 = [3]i32{1, 2, 3};
```

See the [Literals](literals.md#composite-literals) documentation for comprehensive details on composite literal forms.


## Declaration Expressions

Declaration expressions create values of composite types inline without requiring a named type definition.


### Anonymous Record Literals

The `rec` keyword can be used as an expression to create an anonymous record literal:

```mach
val point: rec { x: f32; y: f32; } = rec { x: 1.0, y: 2.0 };
```

See the [Types](types.md#anonymous-rec-and-uni) documentation for details on anonymous composite types.


### Anonymous Union Literals

The `uni` keyword can be used as an expression to create an anonymous union literal:

```mach
val result: uni { ok: i32; err: *u8; } = uni { ok: 42 };
```

See the [Types](types.md#anonymous-rec-and-uni) documentation for details on anonymous composite types.


## Compile-time Expressions

The `$` prefix marks an expression for compile-time evaluation. This is used for compile-time intrinsics, target information, and other compile-time operations.

```mach
val size:     u64  = $size_of(i32);
val is_linux: bool = $OS_LINUX;
```

See the [Compiletime Systems](compiletime.md) documentation for details on compile-time evaluation and available intrinsics.


## Assignment

Assignment updates the value of a mutable variable. The left-hand side must be a mutable location (e.g., a `var` declaration or dereferenced pointer).

```mach
var x: i32 = 10; # declare and initialize x
x = 20;          # update x

var p: *i32 = ?x; # create pointer to x
@p = 30;          # update through pointer
```

Assignment has the lowest precedence of all operators. The right-hand side is fully evaluated before assignment occurs.

```mach
var a: i32 = 5; # declare and initialize a
a = a + 10;     # evaluates to 15
```

Only variables declared with `var` can be assigned to. Variables declared with `val` are immutable after initialization. See the [Keywords](keywords.md#var) and [Keywords](keywords.md#val) documentation for details.

