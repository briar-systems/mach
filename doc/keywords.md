# Keywords and Symbols

This reference lists the reserved keywords and token symbols in the Mach language, with brief descriptions and links to relevant sections of the language reference.

- [Keywords and Symbols](#keywords-and-symbols)
- [Reserved Keywords](#reserved-keywords)
  - [`use`](#use)
  - [`ext`](#ext)
  - [`def`](#def)
  - [`pub`](#pub)
  - [`rec`](#rec)
  - [`uni`](#uni)
  - [`val`](#val)
  - [`var`](#var)
  - [`fun`](#fun)
    - [Method Syntax](#method-syntax)
  - [`ret`](#ret)
  - [`if`](#if)
  - [`or`](#or)
  - [`for`](#for)
  - [`cnt`](#cnt)
  - [`brk`](#brk)
  - [`asm`](#asm)


# Reserved Keywords

The identifiers listed in this file are reserved keywords and cannot be used as names for variables, functions, types, or other user-defined entities in Mach code.

The only other reserved keywords in Mach are the primitive types, listed in the [Types](types.md) documentation.


## `use`

> `use [<alias>:] <module-path>;`

The `use` keyword is used to import public symbols from another Mach module.
It allows for an alias to be specified which can be used to "contain" the imported symbols, preventing name collisions.

```mach
use std.types.bool;

use console: std.io.console;
```


## `ext`

> `[pub] ext "<ABI>:<linkage-name>" <symbol-name>: <type>;`

The `ext` keyword is used to declare an external (foreign) symbol binding.
It can optionally be marked as `pub`.

- `<ABI>` specifies the calling convention of the external symbol (e.g., `C`).
- `<linkage-name>` specifies the name of the symbol in the foreign code, which may differ from the Mach identifier.
- `<symbol-name>` is the identifier used to refer to the external symbol within Mach code.
- `<type>` specifies the type of the external symbol.

```mach
ext "C:printf" printf: fun(fmt: *u8, ...) i32;
```


## `def`

> `[pub] def <alias>: <type>;`

The `def` keyword is used to create a type alias.
It can optionally be marked as `pub`.
It associates the `<alias>` with the specified `<type>`, allowing the type to be referred to by the alias elsewhere in the code.

```mach
def Age: u32;
```


## `pub`

> `pub <declaration>;`

The `pub` keyword is used to make a top-level declaration public to importers of the module.
It can be applied to functions, types, variables, and aliases.

```mach
pub def ID: u64;
```


## `rec`

> `[pub] rec <identifier>[[<type_id>[, ...]] { [<field-name>: <field-type>; ...] };`

The `rec` keyword is used to define a record (struct-like) type or literal.
It can optionally be marked as `pub`.
A record consists of named fields, each with a specified type.

```mach
rec Point {
    x: f32;
    y: f32;
}
```

## `uni`

> `[pub] uni <identifier>[[<type_id>[, ...]] { [<variant-name>: <variant-type>; ...] };`

The `uni` keyword is used to define a union type or literal.
It can optionally be marked as `pub`.
A union consists of named variants, each with a specified type.

```mach
uni Result {
    ok:  u32;
    err: u32;
}
```


## `val`

> `[pub] val <identifier>: <type> = <expression>;`

The `val` keyword is used to declare an initialized, immutable constant.
It can optionally be marked as `pub` when used in the global scope.
Once assigned, the value of a `val` cannot be changed.

> NOTE: `val` declarations are not guaranteed to be compile-time constants unless the initializer expression itself is a compile-time constant.
> 
> `val` simply guarantees immutability after initialization.

```mach
val pi: f32 = 3.14;
```


## `var`

> `[pub] var <identifier>: <type> [= <expression>];`

The `var` keyword is used to declare a variable (mutable).
It can optionally be marked as `pub` when used in the global scope.
The value of a `var` can be changed after initialization.

```mach
var foo: i32;
var bar: f32 = 0.0;
```


## `fun`

> `[pub] fun [(<method_identifier>: <method_type>)] <identifier>[[<type_id>[, ...]]([<parameter-name>: <parameter-type>, ...]) [<return-type>] { <body> }`

The `fun` keyword is used to declare a function or method.
It can optionally be marked as `pub`.
It defines a callable block of code with parameters and an optional return type.

```mach
pub fun foo() {
    ret;
}

fun add(a: i32, b: i32) i32 {
    ret a + b;
}
```


### Method Syntax

Methods are functions associated with a specific type, allowing them to operate on instances of that type.
They are defined using the same `fun` keyword, but include a special first parameter that represents the instance the method is called on.

The instance itself can be specified as either a named value or a pointer to a named value.
The instance parameter is conventionally named `this`, but any valid identifier can be used.

```mach
rec Point {
    x: f32;
    y: f32;
}

fun (this: *Point) move(dx: f32, dy: f32) {
    this.x = this.x + dx;
    this.y = this.y + dy;
}
```


## `ret`

> `ret [<expression>];`

The `ret` keyword is used to return from a function, optionally with a value.
If the function has a return type, an expression must be provided.

```mach
fun get_answer() i32 {
    ret 42;
}
```


## `if`

> `if (<condition>) { <body> }`

The `if` keyword is used to introduce a conditional block.
If the `<condition>` evaluates to true (non-zero), the `<body>` is executed.

> See this section on [`if` and `or`](quirks.md#if-and-or) for more details on the design rationale behind Mach's conditional constructs.

```mach
if (x > 0) {
    ret 1;
}
```


## `or`

> `or [(<condition>)] { <body> }`

The `or` keyword is used to continue a conditional chain, similar to `else if` or `else` in other languages.
If a `<condition>` is provided and evaluates to true (non-zero), the `<body>` is executed.
If no condition is provided, the `<body>` is executed if all previous conditions were false (zero).
An `or` statement with an empty condition is only valid as the last clause in an `if`/`or` chain.

`or` is not valid as an independent statement; it must follow an `if` or another `or`.

```mach
if (x > 0) {
    ret 1;
}
or (x < 0) {
    ret -1;
}
or {
    ret 0;
}

if (condition1) { ret 1; }
or (condition2) { ret 2; }
or (condition3) { ret 3; }

```

## `for`

> `for [(<condition>)] { <body> }`

The `for` keyword is used to introduce a loop with an optional condition.
If a `<condition>` is provided, the loop continues as long as the condition evaluates to true (non-zero).
If no condition is provided, the loop continues indefinitely until a `brk` (or `ret`) is encountered.

```mach
var i: i32 = 0;
for (i < 10) {
    i = i + 1;
}

for {
    # infinite loop
}
```

> See this section on [`for` loops](quirks.md#for-loops) for more details on the design rationale behind Mach's loop constructs.


## `cnt`

> `cnt;`

The `cnt` keyword is used to continue to the next iteration of the innermost loop.
It must be used within a `for` loop to skip the remaining statements in the current iteration and proceed with the next iteration.

`cnt` is not valid outside of a loop context.

```mach
var i: i32 = 0;
for (i < 10) {
    i = i + 1;
    if (i % 2 == 0) {
        cnt;
    }

    # this code runs only for odd values of i
}
```


## `brk`

> `brk;`

The `brk` keyword is used to break out of the innermost loop.
It must be used within a `for` loop to exit the loop immediately.

`brk` is not valid outside of a loop context.

```mach
var i: i32 = 0;
for {
    i = i + 1;
    if (i == 5) {
        brk;
    }
}
```


## `asm`

> `asm { <assembly-instructions> }`

The `asm` keyword is used to introduce an inline assembly statement.
It allows embedding low-level assembly instructions directly within Mach code for performance-critical or hardware-specific operations.

```mach
asm {
    mov eax, 1;
    int 0x80;
}
```

`asm` is allowed at both global and function scope.

> At the time of writing, inline assembly blocks do not support interpolation of Mach variables into the assembly code or "clobber" declarations.
