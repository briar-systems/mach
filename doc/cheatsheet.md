# Mach Cheatsheet

This page summarizes the syntax and tooling you are most likely to reach for when working in Mach.

- [Mach Cheatsheet](#mach-cheatsheet)
  - [Project and toolchain](#project-and-toolchain)
  - [Module basics](#module-basics)
  - [Keywords](#keywords)
  - [Builtin Types](#builtin-types)
  - [Builtin Values](#builtin-values)
  - [Declaration Examples](#declaration-examples)
  - ["Complex" types](#complex-types)
  - [Expressions and operators](#expressions-and-operators)
  - [Control flow snippets](#control-flow-snippets)
  - [Entry points](#entry-points)


## Project and toolchain

| Task                                         | Command                                                              |
| -------------------------------------------- | -------------------------------------------------------------------- |
| Scaffold a project                           | `cmach init my-app`                                                  |
| Build default target                         | `cmach build .`                                                      |
| Build specific target                        | `cmach build . --target linux`                                       |
| Run the most recent build (does not rebuild) | `cmach run .`                                                        |
| List dependencies                            | `cmach dep list`                                                     |
| Add remote dependency                        | `cmach dep add https://github.com/org/pkg.git --version branch/main` |
| Pull/update dependencies                     | `cmach dep pull`                                                     |

`mach.toml` is required for most situations.
Keep `[project]`, `[targets.*]`, and `[deps.*]` in sync with the layout on disk.
See [config.md](config.md) and [dependencies.md](dependencies.md) for details.


## Module basics

- Modules map one-to-one with `.mach` files under `[project].dir_src` or the dependency's source tree.
- `use project.module;` makes the module's public symbols available.
- `use alias: project.module;` creates a local alias for easier access.

Example project tree:

```
src/
    main.mach            # module: myapp.main
    driver/pipeline.mach # module: myapp.driver.pipeline
mach.toml                # project.id = "myapp"
```


## Keywords

| Keyword | Usage                                                                                                                                                    | Description                            |
| ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------- |
| `use`   | `use [<alias>:] <module-path>;`                                                                                                                          | Import a module into current namespace |
| `ext`   | `[pub] ext "<ABI>:<linkage-name>" <symbol-name>: <type>;`                                                                                                | Define an external symbol (FFI)        |
| `def`   | `[pub] def <alias>: <type>;`                                                                                                                             | Define a type alias                    |
| `pub`   | `pub <declaration>;`                                                                                                                                     | Make a symbol public                   |
| `rec`   | `[pub] rec <identifier>[[<type_id>[, ...]] { [<field-name>: <field-type>; ...] };`                                                                       | Define a record type                   |
| `uni`   | `[pub] uni <identifier>[[<type_id>[, ...]] { [<variant-name>: <variant-type>; ...] };`                                                                   | Define a union type                    |
| `val`   | `[pub] val <identifier>: <type> = <expression>;`                                                                                                         | Define an immutable binding            |
| `var`   | `[pub] var <identifier>: <type> [= <expression>];`                                                                                                       | Define a mutable binding               |
| `fun`   | `[pub] fun [(<method_identifier>: <method_type>)] <identifier>[[<type_id>[, ...]]([<parameter-name>: <parameter-type>, ...]) [<return-type>] { <body> }` | Define a function                      |
| `ret`   | `ret [<expression>];`                                                                                                                                    | Return a value from a function         |
| `if`    | `if (<condition>) { <body> } [<or-statement>]`                                                                                                           | Conditional execution                  |
| `or`    | `or [(<condition>)] { <body> } [<or-statement>]`                                                                                                         | Alternative branch                     |
| `for`   | `for [(<condition>)] { <body> }`                                                                                                                         | Loop construct                         |
| `cnt`   | `cnt;`                                                                                                                                                   | Continue to the next iteration         |
| `brk`   | `brk;`                                                                                                                                                   | Break out of a loop                    |
| `asm`   | `asm { <assembly-instructions> }`                                                                                                                        | Inline assembly                        |


## Builtin Types

| Type  | Description                        |
| ----- | ---------------------------------- |
| `u8`  | 8-bit unsigned integer             |
| `u16` | 16-bit unsigned integer            |
| `u32` | 32-bit unsigned integer            |
| `u64` | 64-bit unsigned integer            |
| `i8`  | 8-bit signed integer               |
| `i16` | 16-bit signed integer              |
| `i32` | 32-bit signed integer              |
| `i64` | 64-bit signed integer              |
| `f32` | 32-bit floating-point              |
| `f64` | 64-bit floating-point              |
| `ptr` | Untyped pointer (like C's `void*`) |
| `str` | String literal record `{ data: *u8, len: pointer-sized-uint }` |


## Builtin Values

| Value | Description                                   |
| ----- | --------------------------------------------- |
| `nil` | Null pointer (`ptr` type with a value of `0`) |

## Declaration Examples

```mach
# aliased import
use print: std.print;

val answer:  i32 = 42; # immutable binding
var counter: i32 = 0;  # mutable binding

# type alias
def Seconds: i64;

# record type
rec Point {
    x: f32;
    y: f32;
}

# union type (generic)
uni Result[T] {
    ok:  T;
    err: T;
}

# method with pointer receiver
fun (this: *Point) move(dx: f32, dy: f32) {
    this.x = this.x + dx;
    this.y = this.y + dy;
}

# simple function
fun add(a: i32, b: i32) i32 {
    ret a + b;
}
```


## "Complex" types

In addition to the builtin primitive types, Mach supports several native compound types:

| Category | Syntax |
| -------- | ------ |
| Pointer  | `*T`   |
| Array    | `[N]T` |

Records and unions can contain fields of any other type, including other generics.
Anonymous `rec { ... }` and `uni { ... }` blocks are valid anywhere a type expression is allowed.


## Expressions and operators

| Construct     | Example                                   | Notes                                        |
| ------------- | ----------------------------------------- | -------------------------------------------- |
| Field access  | `point.x`                                 | Works on records or module namespaces.       |
| Indexing      | `buffer[i]`                               | Arrays or pointer-like values.               |
| Call          | `func(arg1, arg2)`                        | Methods rewrite to calls with receivers.     |
| Generic call  | `func[i32](value)`                        | Supply type arguments before parentheses.    |
| Typed literal | `Pair[i32, f64]{ first: 1, second: 2.0 }` | Construct generic values directly.           |
| Cast          | `value::TargetType`                       | Bit reinterpretation, sizes must match.      |
| Address-of    | `?expr`                                   | Operand must be lvalue; result is a pointer. |
| Dereference   | `@expr`                                   | Operand must be pointer-like.                |

Literal coercion only happens at the point of declaration.
Aliases with identical underlying types can be used interchangeably without casts.
All other conversions require explicit casts.


## Control flow snippets

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
```

```mach
var counter: i32 = 0;
for (counter < 10) {
    counter = counter + 1;
    if (counter % 2 == 0) {
        cnt;
    }

    # odd-only work
}

for {
    # infinite loop
    brk; # exit manually
}
```

## Entry points

The runtime included in the Mach standard library looks for a function returning `i64` with a C-like signature: `argc` and `argv`.
This is not enforced by the compiler itself, but by convention outlined in `std.runtime`.

Because of this, a minimal Mach program must use the standard library as a dependency.
With that in place, a simple "Hello, World!" program looks like this:

```mach
use          std.runtime;
use print:   std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.println("Hello, World!");
    ret 0;
}
```

Please refer to the [standard library](https://github.com/octalide/mach-std) for specific implementation details.
