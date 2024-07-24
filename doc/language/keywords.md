- [Mach Keywords](#mach-keywords)
  - [Operational](#operational)
  - [Declaration](#declaration)
  - [Flow Control](#flow-control)
  - [Type Keywords](#type-keywords)
- [Detailed Keyword Descriptions](#detailed-keyword-descriptions)
  - [Declaration Keyword Usage](#declaration-keyword-usage)
    - [`use`](#use)
      - [A note about the standard library](#a-note-about-the-standard-library)
    - [`val`](#val)
    - [`var`](#var)
    - [`fun`](#fun)
    - [`str`](#str)
    - [`uni`](#uni)
    - [`def`](#def)
  - [Flow Control Keyword Usage](#flow-control-keyword-usage)
    - [`if`](#if)
    - [`or`](#or)
    - [`for`](#for)
    - [`brk`](#brk)
    - [`cnt`](#cnt)
    - [`ret`](#ret)


# Mach Keywords

Below is a complete list of keywords available in the mach language.

Mach's keywords were specifically chosen to minimize both the number of reserved keywords as well as the number of characters per keyword.
This is to make the language as easy to read as possible.
Mach has very few non-type keywords, and nearly all of them are 3 or fewer characters long, with the preference being exactly 3 characters for symmetry.

The built-in types only include integers and floating point numbers, whose keywords all follow a predictable pattern:
`<format><bits>` where `format` is one of `u` (unsigned integer), `i` (signed integer), or `f` (floating point), and `bits` is the number of bits in the type (e.g., `i32` is a signed 32-bit integer).

For more information on types, refer to the [types documentation](types.md).


## Operational

| keyword       | description                                           |
| ------------- | ----------------------------------------------------- |
| [`use`](#use) | import a module or symbol for use in the current file |

## Declaration

| keyword       | description                              |
| ------------- | ---------------------------------------- |
| [`val`](#val) | value definition                         |
| [`var`](#var) | variable definition                      |
| [`fun`](#fun) | function declaration                     |
| [`str`](#str) | struct declaration                       |
| [`uni`](#uni) | union declaration                        |
| [`def`](#def) | type definition                          |


## Flow Control

| keyword       | description        |
| ------------- | ------------------ |
| [`if`](#if)   | if statement       |
| [`or`](#or)   | or statement       |
| [`for`](#for) | for statement      |
| [`cnt`](#cnt) | continue statement |
| [`brk`](#brk) | break statement    |
| [`ret`](#ret) | return statement   |


## Type Keywords

Basic types:

| keyword | description                  |
| ------- | ---------------------------- |
| `i8`    | signed 8-bit integer         |
| `i16`   | signed 16-bit integer        |
| `i32`   | signed 32-bit integer        |
| `i64`   | signed 64-bit integer        |
| `u8`    | unsigned 8-bit integer       |
| `u16`   | unsigned 16-bit integer      |
| `u32`   | unsigned 32-bit integer      |
| `u64`   | unsigned 64-bit integer      |
| `f32`   | 32-bit floating point number |
| `f64`   | 64-bit floating point number |
| `ptr`   | pointer to a value           |

Complex types:

| keyword       | description |
| ------------- | ----------- |
| [`map`](#map) | map         |

For more information about types, see the [types documentation](types.md).


# Detailed Keyword Descriptions


## Declaration Keyword Usage


### `use`

Imports symbols from a mach file.
Paths can be either relative to the current file or relative to the `dep` directory specified in the project configuration.
The latter option is selected by the compiler when a local path lookup fails.

Aliases can be set by providing an identifier before the path.

A `use` statement that does not provide an alias will import the symbols from the target file into the scope of the current file.
A good example of this behaviour can be seen when importing "std/bool" unaliased.
This imports three symbols: `bool`, a `u8` type, and two values -- `true` and `false`.
These symbols would be directly accessable without qualifiers in the current file if no alias is provided.

#### A note about the standard library

Note that the "standard library" is only usable when added as a project dependancy.
It *is* packaged with an installation of the language.
This will copy the standard library into the `dep` folder, at which point it will be referenced by the compiler.

This decision may seem clunky at first, but the choice was made so that the standard library is entirely separated from the rest of the language.
This allows for extremely fine grained control over language versioning and allows the language to be used entirely standalone if desired.

Syntax:
```
use [<alias>] "<path>";
```

Examples:
```
use "foo";
use "foo/bar/baz";
use foo "foo";
use bar "foo/bar";
```


### `val`

Declares an immutable variable

Syntax:
```
val <name>: <type> = <value>;
```

Examples:
```
val foo: u32 = 0;
```


### `var`

Declares a mutable variable

Sample:
```
var <name>: <type> [= <value>];
```

Examples:
```
var foo: u32;
var foo: u32 = 0;
```


### `fun`

Declares a function.

Syntax:
```
fun <name>(<[arg]...>): <[return_type]> { ... }
```

Examples:
```
fun foo() { ... }
fun foo() u32 { ... }
fun foo(bar: u32) { ... }
fun foo(bar: u32): u32 { ... }
fun foo(bar: u32, baz: u32): u32 { ... }
```


### `str`

Declares a struct

Syntax:
```
str <name> {
    <field_name>: <field_type>;
    ...
}
```

Examples:
```
str foo {
    bar: u32;
    baz: #foo;
}
```


### `uni`

Declares a union

Syntax:
```
uni <name> {
    <field_name>: <field_type>;
    ...
}
```

Examples:
```
uni foo {
    bar: u32;
    baz: #foo;
}
```


### `def`

Declares a type alias.

`def` can be used to make one of five types of aliases:
- A type alias for builtin type
- A type alias for another type
- A type alias for a struct
- A type alias for a union
- A type alias for a function

The only special cases from those above are struct, union, and function, which each require the use of their own respective keywords and special syntax.

Syntax:
```
def <name>: <type>;
```

Examples:
```
def foo: u32;     // alias for u32 (builtin type)
def foo: bar;     // alias for bar (another type, builtin or not)
def foo: [2]#bar; // complex, but still valid type alias

def foo: fun(u32): u32; // alias for a function type

// alias for a struct type
def foo: str {
    bar: u32; 
    baz: u32;
}

// alias for a union type
def foo: uni {
    bar: u32;
    baz: u32;
}
```


## Flow Control Keyword Usage


### `if`

Declares an if statement

Syntax:

```
if (<condition>) { ... }
```

Examples:
```
if (foo == 0) { ... }
```


### `or`

Declares an or statement.
An or statement works identically to `else if` and `else` in many other languages.
Branches defined by an or statement are taken if the preceding if statement's condition has resolved to `false`.

Or statements can only be used following an if statement and, like the if statement, may have a condition that needs to be met before proceeding.
Only one or statement in each chain is allowed to *not* have a condition, in which case the statement operates like `else`. 

Syntax:

```
or [(<condition>)] { ... }
```

Examples:
```
or (foo == 0) { ... }
or { ... }
```

```
if (foo == 0) { ... }
or (bar == 0) { ... }
or            { ... }
```


### `for`

Declares a for statement.

For loops work similarly to other languages, except in that they only take a boolean condition as their single parameter.
Traditional C-style for loops (initializer; condition; increment) are not supported.

The condition is optional and may be ommitted if the desired functionality is similar to that of a `while` loop in other languages.
The condition will autoresolve to `true` and the loop will run indefinitely.

Syntax:
```
for [(<condition>)] { ... }
```

Examples:
```
for { ... }
for (foo == bar) { ... }
```


### `brk`

Declares a break statement.

Break statements may only be used inside of `for` loops and signify that the current scope should be exited.
This will halt the execution of a for loop and continue with the next statement after the loop.

Syntax:
```
brk;
```

Examples:
```
for {
    if (foo == 0) { brk; }
}
```


### `cnt`

Declares a continue statement.

Continue statements may only be used inside of `for` loops and signify that the current iteration should be skipped.
This will halt the execution of the current iteration and continue with the next iteration.

Syntax:
```
cnt;
```

Examples:
```
for {
    if (foo == 0) { cnt; }
}
```


### `ret`

Declares a return statement.

The value returned must match the function's return type.

Syntax:
```
ret <value>;
```

Examples:
```
ret 0;
```
