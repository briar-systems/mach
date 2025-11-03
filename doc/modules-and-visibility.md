# Modules and Visibility

This document explains modules, imports with `use`, symbol visibility with `pub`, aliases, and qualified names. It shows how to structure code across modules and how to access public declarations.

Related topics:
- [Functions and Methods](functions-and-methods.md)
- [Types](types.md)
- [Variables and Constants](variables.md)
- [Records and Unions](records-and-unions.md)
- [Generics](generics.md)

## Overview

- A module is a compilation unit that groups declarations (types, values, variables, functions, records, unions, external bindings).
- Use `use` to import a module’s public symbols.
- Use `pub` on a top‑level declaration to export it from the current module.
- Import styles:
  - Unaliased: bring public symbols into the current module’s scope for direct use.
  - Aliased: bind the module to a short name and refer to its symbols via `alias.Symbol`.
- Qualified names allow you to reference symbols and types from an imported module using a module alias.

## Module names

- Module names are dot‑separated identifiers: `std.io.console`, `std.types.string`, `myapp.core`.
- A single identifier is also a valid module name: `commands`, `math`.
- Dots express hierarchy only for naming; import always occurs by the full module path you specify.

## Importing modules: `use`

The `use` statement imports another module. It must appear at top level and ends with `;`.

Forms:
- Unaliased:
  ```
  use std.types.string;
  use commands;
  ```
  Public symbols from the imported module are available directly in the current module’s scope:
  ```
  fun main(args: []string) i64 {  # 'string' came from std.types.string
      ret dispatch(args);         # 'dispatch' came from commands
  }
  ```

- Aliased:
  ```
  use console: std.io.console;
  use mem:     std.system.memory;
  ```
  Refer to imported symbols through the alias:
  ```
  console.print("hello\n");
  ```

Rules:
- `use` is allowed only at the top level.
- `use` ends with `;`.
- No wildcard or selective imports; import the module as a whole (with or without alias).
- An alias is a single identifier before a `:` followed by the module path.
- `pub` cannot be applied to `use`.

## Visibility and `pub`

By default, top‑level declarations are private to their defining module. Prepend `pub` to export a declaration so it can be imported by other modules.

`pub` may be used with:
- Type aliases: `pub def Name: Type;`
- Constants: `pub val NAME: Type = expr;`
- Variables: `pub var name: Type = expr;` (initializer optional)
- Functions: `pub fun name(params) ReturnType { ... }`
- Methods: `pub fun (recv: Type) name(params) ReturnType { ... }`
- Records: `pub rec Name { field: Type; ... }`
- Unions: `pub uni Name { field: Type; ... }`
- Externals: `pub ext "C[:symbol]" name: fun(...) Ret;` (or other types)

Not allowed with `pub`:
- `use`
- `asm { ... }`
- Compile‑time directives/statements (`$...`)

Examples:
```
pub def Index: u64;

pub rec Point {
    x: f64;
    y: f64;
}

pub fun distance(p: Point) f64 {
    ret (p.x * p.x + p.y * p.y);
}

pub ext "C:puts" puts: fun(*u8) i32;
```

## Qualified names

When a module is imported with an alias, refer to its public symbols via `alias.Symbol`. Types can also be qualified in type positions.

- Values/functions:
  ```
  use console: std.io.console;
  console.print("hi\n");
  ```

- Types:
  ```
  use string: std.types.string;
  val msg: string = "ok";
  ```

- Qualified type names in type positions:
  ```
  use net: mylib.net;
  var s: net.SocketState;
  ```

- Generics with qualified names:
  ```
  use coll: mylib.collections;
  rec Box[T] { value: T; }
  val bx: coll.Box[u64] = coll.Box[u64]{ value: 42 };
  ```

Notes:
- The `.` in qualified names is the same token used for record/union field access. In a qualified reference like `alias.Name`, the left side is a module alias; in `expr.field`, the left side is a value expression. The language resolves these forms by context.
- If both a directly imported symbol and an alias‑qualified symbol share a name, the unqualified name refers to the directly imported one; use `alias.Name` to disambiguate.

## Import styles and name resolution

- Unaliased `use` introduces the imported module’s public identifiers directly into the current module’s scope.
- Aliased `use` does not inject the module’s symbols into the unqualified scope; access them with `alias.Symbol`.
- You may mix both styles:
  ```
  use std.types.string;            # unaliased; brings 'string'
  use console: std.io.console;     # aliased; use 'console.print'
  ```

- Name collisions:
  - If two unaliased `use` statements bring in the same identifier, it is a compile‑time error due to ambiguity.
  - Prefer aliasing to avoid collisions:
    ```
    use json: vendor.json;
    use json2: other.json;
    ```

## Examples

Unaliased import and direct usage:
```
use std.types.string;
use commands;

fun main(args: []string) i64 {
    ret dispatch(args);
}
```

Aliased import and qualified usage:
```
use console: std.io.console;

pub fun log_ok() {
    console.print("ok\n");
}
```

Combining both:
```
use std.types.string;
use console: std.io.console;

pub fun greet(name: string) {
    console.print("hello, %s\n", name);
}
```

Exporting declarations:
```
pub def Bytes: []u8;

pub rec Pair {
    a: u64;
    b: u64;
}

pub fun make_pair(a: u64, b: u64) Pair {
    ret Pair{ a: a, b: b };
}
```

## Best practices

- Export only what consumers need; keep internal helpers private by omitting `pub`.
- Use aliases to:
  - Avoid name collisions.
  - Improve readability for frequently used modules (`console`, `mem`, etc.).
- Prefer stable, descriptive module paths (`std.types.string`, `std.io.console`) and short, meaningful aliases.

For more on declarations and use sites:
- [Functions and Methods](functions-and-methods.md)
- [Types](types.md)
- [Records and Unions](records-and-unions.md)
- [Generics](generics.md)