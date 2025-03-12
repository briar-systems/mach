- [Mach Keywords](#mach-keywords)
  - [Import](#import)
  - [Declaration](#declaration)
  - [Flow Control](#flow-control)
- [Detailed Keyword Descriptions](#detailed-keyword-descriptions)
  - [Declaration Keyword Usage](#declaration-keyword-usage)
    - [`use`](#use)
      - [A note about the standard library](#a-note-about-the-standard-library)
    - [`val`](#val)
    - [`var`](#var)
    - [`vol`](#vol)
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


## Import

| keyword       | description                                           |
| ------------- | ----------------------------------------------------- |
| [`use`](#use) | import a module or symbol for use in the current file |

## Declaration

| keyword       | description           |
| ------------- | --------------------- |
| [`val`](#val) | value definition      |
| [`var`](#var) | variable definition   |
| [`fun`](#fun) | function declaration  |
| [`str`](#str) | struct declaration    |
| [`uni`](#uni) | union declaration     |
| [`def`](#def) | type definition/alias |


## Flow Control

| keyword         | description        |
| --------------- | ------------------ |
| [`if`](#if)     | if statement       |
| [`or`](#or)     | or statement       |
| [`for`](#for)   | for statement      |
| [`cnt`](#cnt)   | continue statement |
| [`brk`](#brk)   | brk statement      |
| [`ret`](#ret)   | return statement   |
| [`try`](#try)   | try statement      |
| [`pass`](#pass) | pass statement     |
| [`fail`](#fail) | fail statement     |


# Detailed Keyword Descriptions


## Declaration Keyword Usage


### `use`

Imports symbols from a mach file.
Paths can be either relative to the current file or relative to the `dep` directory specified in the project configuration.
The latter option is selected by the compiler when a local path lookup fails.

Aliases can be set by providing an identifier before the path.

A `use` statement that does not provide an alias will import the symbols from the target file into the scope of the current file.
A good example of this behaviour can be seen when importing `std.types.bool` unaliased.
This imports three symbols: `bool`, a `u8` type alias, and two values -- `true` and `false`.
These symbols would be directly accessable without qualifiers in the current file if no alias is provided.


#### A note about the standard library

Note that the "standard library" is only usable when added as a project dependancy.
It *is* packaged with an installation of the language.
This will copy the standard library into the `dep` folder, at which point it will be referenced by the compiler.

This decision may seem clunky at first, but the choice was made so that the standard library is entirely separated from the rest of the language.
This allows for extremely fine grained control over language versioning and allows the language to be used entirely standalone if desired.

Syntax:
```
use [<alias>:] <path>
```

Examples:
```
use      foo
use      foo.bar.baz
use foo: foo
use bar: foo.bar
```


### `val`

Declares an immutable variable.

Syntax:
```
val <name>: <type> = <value>
```

Examples:
```
val foo: u32 = 0
```


### `var`

Declares a mutable variable.

Sample:
```
var <name>: <type> [= <value>]
```

Examples:
```
var foo: u32
var foo: u32 = 0
```

### `vol`

Declares a volatile variable.
This keyword functions similarly to `volatile` in C. The compiler will not optimize away reads or writes to a volatile variable.

Sample:
```
vol <name>: <type> [= <value>]
```

Examples:
```
vol foo: u32
vol foo: u32 = 0
```


### `fun`

Defines a function.

Functions have a few different ways in which they can be defined.
The basic syntax is as follows:
```
fun <name>([<arg>...]) <return_type | "void"> { <body> }
```

Examples:
```
fun foo() void { }
fun foo() u32 { }
fun foo(bar: u32) void { }
fun foo(bar: u32) u32 { }
fun foo(bar: u32, baz: u32) u32 { }
```

Functions support [generic](doc/language/generics.md) arguments (not shown here).

Functions support a special syntax for [error handling](doc/language/errors.md) (not shown here).

### `str`

Declares a structure.

Syntax:
```
str <name> {
    <field_name>: <field_type>
    ...
}
```

Examples:
```
str foo {
    bar: u32
    baz: *foo
}
```

Structures support [generic](doc/language/generics.md) arguments (not shown here).


### `uni`

Declares a union.

Syntax:
```
uni <name> {
    <field_name>: <field_type>
    ...
}
```

Examples:
```
uni foo {
    bar: u32
    baz: *foo
}
```

Unions support [generic](doc/language/generics.md) arguments (not shown here).


### `def`

Declares a type alias.

Syntax:
```
def <name>: <type>
```

Examples:
```
def foo: u32     # alias for u32 (builtin type)
def foo: bar     # alias for bar (another type, builtin or not)
def foo: [2]*bar # alias for a fixed size array of pointers to bar

def foo: fun (u32): u32 # alias for a function type

# alias for a struct type
def foo: str {
    bar: u32 
    baz: u32
}

# alias for a union type
def foo: uni {
    bar: u32
    baz: u32
}
```

The `def` keyword supports [generic](doc/language/generics.md) arguments (not shown here).


## Flow Control Keyword Usage


### `if`

Syntax:

```
if <condition> { <body> }
```

Examples:
```
if foo == 0 { <body> }
```

The statement body will be executed if the condition resolves to `true` (`1`).


### `or`

An or statement works identically to `else if` and `else` in many other languages.
Branches defined by an or statement are executed if the preceding if statement's condition has resolved to `false` (`0`).

Or statements can only be used following an if statement and, like the if statement, may have a condition that needs to be met before proceeding.
Only one or statement in each chain is allowed to *not* have a condition, in which case the statement operates like `else`. 

Syntax:

```
or [<condition>] { <body> }
```

Examples:
```
or foo == 0 { <body> }
or          { <body> }
```

```
if foo == 0 { <body> }
or bar == 0 { <body> }
or          { <body> }
```


### `for`

The body will be executed as long as the condition resolves to `true`.

Syntax:
```
for [<condition>] { <body> }
```

Note that `for` statements in mach operate similarly to `while` statements in other languages and traditional C-style `for` loops (with `inst cond inc` syntax) are not supported.

### `brk`

Declares a break statement.

Break statements may only be used inside of `for` loops and signify that the current scope should be exited, similar to `break` in other languages.
This will halt the execution of a loop and continue with the next statement after the loop.

Syntax:
```
brk
```

Examples:
```
for {
    if (foo == 0) { brk }
}
```


### `cnt`

Continue statements may only be used inside of `for` loops and signify that the remainder of the current iteration should be skipped.

Syntax:
```
cnt
```

Examples:
```
for {
    if (foo == 0) { cnt }
}
```


### `ret`

Declares a return statement.

The value returned must match the function's return type.

Syntax:
```
ret <value>
```

Examples:
```
fun main(): u32 {
    ret 0
}
```
