# Variables, Constants, Assignment, Scope, and Semicolons

This document explains how to declare and use variables and constants, how assignment works, how scope is structured, and when semicolons are required.

Related topics:
- [Types](types.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Control Flow](control-flow.md)
- [Modules and Visibility](modules-and-visibility.md)
- [Literals](literals.md)

## Overview

- Use `val` to declare an initialized, immutable binding (a constant).
- Use `var` to declare a mutable variable, optionally with an initializer.
- All declarations require explicit type annotations.
- Assignment uses `=`, and target expressions must be assignable (lvalues).
- Scopes are introduced by blocks `{ ... }`; inner declarations shadow outer ones.
- Statements end with a semicolon `;`.

## Declarations

### Constants: `val`

- Syntax: `val name: Type = expression;`
- `val` must have an initializer and cannot be reassigned.

```/dev/null/variables.mach#L1-10
val max_items: u64 = 1024;
val greeting: []u8 = []u8{ 'h', 'i' };
# greeting = []u8{ 'x' };   # error: cannot assign to val
```

### Variables: `var`

- Syntax: `var name: Type;` or `var name: Type = expression;`
- `var` may be declared without an initializer and can be reassigned.

```/dev/null/variables.mach#L1-18
var count: u64;
count = 1;

var total: u64 = 0;
total = total + count;

# type annotations are required; there is no type inference in declarations
var p: *u8;
p = nil;
```

### Top-level visibility

- Top-level `val` and `var` may be exported with `pub`.
- Module users see only `pub` declarations.

```/dev/null/variables.mach#L1-8
pub val version_major: u64 = 1;
pub var global_counter: u64 = 0;
```

See [Modules and Visibility](modules-and-visibility.md) for import/export rules.

## Assignment

- Operator: `=`
- Assignment requires the left-hand side to be assignable (an lvalue) and the right-hand side to be type-compatible with the left-hand side’s type.

Assignable (lvalue) examples:
- A `var` variable
- A pointer dereference `@ptr`
- A field access of an assignable object `obj.field`
- An indexed element `array[i]` or `slice[i]`

```/dev/null/variables.mach#L1-28
# variable assignment
var n: u64 = 0;
n = n + 1;

# pointer assignment via dereference
var x: u64 = 42;
var px: *u64 = ?x;   # address-of
@px = @px + 1;       # write through pointer

# record field assignment
rec Point { x: f64; y: f64; }
var p: Point;
p.x = 1.5;
p.y = 2.5;

# array and slice element assignment
var a: [3]u8 = [3]u8{ 1, 2, 3 };
a[0] = 7;

var s: []u8 = []u8{ 10, 20, 30 };
s[1] = 25;          # bounds behavior is target-dependent
```

Use explicit casts when needed:

```/dev/null/variables.mach#L1-8
var i: i64;
val u: u64 = 10;
i = (u) :: i64;     # cast u64 -> i64 before assignment
```

See [Expressions and Operators](expressions-and-operators.md) for cast (`::`) and lvalue rules.

## Scope

- Each block `{ ... }` introduces a new scope for declarations.
- Inner declarations shadow outer ones of the same name.
- Variables and constants declared in a block are not visible outside that block.

```/dev/null/variables.mach#L1-34
var x: u64 = 1;

fun demo() {
    var x: u64 = 2;   # shadows outer x
    {
        var x: u64 = 3;   # shadows function-scope x
        # x == 3 here
    }
    # x == 2 here
}

# outer x remains 1 here
```

Top-level scope contains module declarations; use `pub` to make them visible to importers. See [Modules and Visibility](modules-and-visibility.md).

## Semicolons

- Semicolons terminate statements.
- All declaration statements (`use`, `ext`, `def`, `val`, `var`) and expression statements end with `;`.
- Block statements end with `}` and are not followed by `;`.
- Control-flow statements (`ret`, `brk`, `cnt`) end with `;`.

```/dev/null/variables.mach#L1-36
use mylib.os;

def Index: u64;

val threshold: u64 = 10;
var value: u64 = 0;

fun f() {
    value = value + 1;    # expression statement
    if (value < threshold) {
        ret;              # return statement
    }
    for (value < 100) {
        value = value + 1;
        if (value == 50) {
            cnt;          # continue
        }
        if (value == 75) {
            brk;          # break
        }
    }
}
```

Note: Inline assembly blocks `asm { ... }` accept an optional trailing `;`.

## Immutability and reassignment

- `val` is immutable: reassigning is not allowed.
- `var` is mutable: reassign as needed, respecting type rules.

```/dev/null/variables.mach#L1-16
val name: []u8 = []u8{ 'a', 'b' };
# name = []u8{ 'x' };    # error

var counter: u64 = 0;
counter = counter + 1;   # ok
```

## Initialization patterns

- Use typed literals to initialize arrays, slices, records, and unions.
- Use address-of `?` and dereference `@` for pointer-related initialization and mutation.

```/dev/null/variables.mach#L1-24
rec Pair { a: u64; b: u64; }
val p0: Pair = Pair{ a: 1, b: 2 };

var buf: [4]u8 = [4]u8{ 1, 2, 3, 4 };
var view: []u8 = []u8{ 5, 6, 7 };

var q: u64 = 100;
var pq: *u64 = ?q;    # pointer to q
@pq = 101;            # mutate q through pointer
```

For more on composite initialization and pointer operations, see:
- [Arrays and Slices](arrays-and-slices.md)
- [Records and Unions](records-and-unions.md)
- [Pointers and Memory](pointers-and-memory.md)

## Summary

- Choose `val` for immutable constants (initializer required) and `var` for mutable variables (initializer optional).
- Assignment uses `=`, requires an assignable target, and enforces type compatibility; cast explicitly with `::` when needed.
- Scopes are defined by blocks; inner scopes shadow outer names.
- End statements with semicolons; block endings `}` are not followed by semicolons.

Continue with [Expressions and Operators](expressions-and-operators.md) to learn how assignments and other expressions interact with precedence and evaluation.