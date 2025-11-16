# Mach Language Quirks

> Why is it done like *that*?

Mach has several idiosyncrasies that may seem unusual to programmers coming from other languages. This document explains some of these quirks and the rationale behind them.

- [Mach Language Quirks](#mach-language-quirks)
  - [Boolean Type as u8](#boolean-type-as-u8)
  - [`if` and `or`](#if-and-or)
  - ["Record" (`rec`) Instead of "Struct" (`struct`)](#record-rec-instead-of-struct-struct)
  - [No Implicit Type Coercion](#no-implicit-type-coercion)
  - [Vendoring of the Standard Library](#vendoring-of-the-standard-library)


## Boolean Type as u8

Mach uses `u8` as the boolean type for conditions, where zero represents false and one represents true.
This can seem unusual compared to languages with a dedicated boolean type, or those that use a `u1` style representation. 
The choice to use `u8` was made to reflect the underlying hardware representation and to simplify interoperability with other integer types.

The [standard library](https://github.com/octalide/mach-std/blob/main/src/types/bool.mach) provides a `bool` alias as well as `true` and `false` constants for convenience in `std.types.bool`.


## `if` and `or`

Mach's conditional statements use `if` and `or` keywords instead of the more common `else if` or `elif`.

This design choice has garnered mixed reactions and warrants explanation. There are a few reasons why this syntax was chosen:
- Symmetry with `if`: The `or` keyword is the same length as `if`, creating a visually balanced structure, allowing for cleaner alignment in lieu of a `switch` statement:

```mach
if (condition1) { ret 1; }
or (condition2) { ret 2; }
or (condition3) { ret 3; }
or              { ret 0; }
```

- Avoiding the `else if` parsing problem: In some languages (C in particular), `else if` can lead to ambiguity in parsing, especially when nested. Using `or` eliminates this issue entirely.

The intended visual parsing of `if`/`or` chains is phonetically, "if condition, then block; or condition, then block; or block;".


## "Record" (`rec`) Instead of "Struct" (`struct`)

Mach uses the keyword `rec` to define record types instead of the more common `struct`. 
This choice was made to reduce ambiguity between the shortened form of the common `struct` keyword (`str`) and common identifiers for string types (particularly `str`).

Since all of Mach's major keywords are 3 letters or less in length, `struct` was deemed "out of alignment" with the rest of the language's visual design and `rec` was chosen as a concise alternative.


## No Implicit Type Coercion

Mach does not perform implicit type coercion between different types. This means that you must explicitly convert values when assigning or passing them between different types. This design choice helps prevent unexpected behavior and makes type conversions explicit and intentional.

The only exception to this rule is the use of integer literals, which can be assigned to any integer type without an explicit cast, as long as the value fits within the target type's range.


## Vendoring of the Standard Library

Mach's standard library is designed to be vendored directly into projects rather than being linked as an external dependency or even shipped with the compiler itself.

This approach was chosen to ensure that projects have full control over the version of the standard library they use.
It also decouples the compiler itself from the standard library, allowing for independent updates and modifications.

Projects can include the standard library by adding it as a submodule or copying the source files directly into their project structure. This allows for easy customization and ensures that the standard library is always in sync with the project's needs.
