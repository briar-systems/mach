# Expressions


## Operators

### Binary Operators

All binary operators are left-associative, listed from lowest to highest precedence:

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 1 | `=` | Assignment |
| 2 | `\|\|` | Logical OR |
| 3 | `&&` | Logical AND |
| 4 | `\|` | Bitwise OR |
| 5 | `^` | Bitwise XOR |
| 6 | `&` | Bitwise AND |
| 7 | `==` `!=` | Equality |
| 8 | `<` `>` `<=` `>=` | Comparison |
| 9 | `<<` `>>` | Shift |
| 10 | `+` `-` | Addition, subtraction |
| 11 | `*` `/` `%` | Multiplication, division, modulo |

### Unary Operators

Unary operators bind tighter than binary operators:

| Operator | Description |
|----------|-------------|
| `!` | Logical NOT |
| `-` | Numeric negation |
| `~` | Bitwise NOT |
| `?` | Address-of (creates pointer to value) |
| `@` | Dereference (reads/writes through pointer) |

### Postfix Operators

Postfix operators bind tightest:

| Syntax | Description |
|--------|-------------|
| `f(args)` | Function call |
| `a[i]` | Array/pointer indexing |
| `x.field` | Field access |
| `x::T` | Type cast (bit reinterpretation) |


## Literals

### Integer Literals

```mach
val dec: i32 = 42;
val hex: i32 = 0xff;
val bin: i32 = 0b1010;
val oct: i32 = 0o755;
val big: i64 = 1_000_000;   # underscores for readability
```

Integer literals can have a type suffix: `42u8`, `100i64`, `0xffu32`.

### Float Literals

```mach
val pi: f64 = 3.14159;
val sci: f64 = 1.5e-10;
```

Float literals can have a type suffix: `3.14f32`.

### Character Literals

```mach
val c: u8 = 'A';
val nl: u8 = '\n';
val hex: u8 = '\x41';   # 'A'
```

Escape sequences: `\n` `\t` `\r` `\\` `\"` `\'` `\0` `\xNN`.

### String Literals

```mach
val s: str = "hello, world";
```

String literals are null-terminated and placed in read-only memory. The type is `str` (alias for `&u8`).

### Other Literals

```mach
val n: *u8 = nil;       # null pointer
val t: bool = true;     # boolean (from std.types.bool)
val f: bool = false;
```


## Composite Literals

### Record Literals

```mach
rec Point { x: f64; y: f64; }
val p: Point = Point{x: 1.0, y: 2.0};
```

### Array Literals

```mach
val a: [3]i32 = [3]i32{10, 20, 30};
```

### Generic Typed Literals

```mach
val pair: Pair[i32, str] = Pair[i32, str]{first: 42, second: "hello"};
```


## Assignment

Only `var` bindings and dereferenced mutable pointers can be assigned to:

```mach
var x: i32 = 10;
x = 20;

var p: *i32 = ?x;
@p = 30;
```


## Parentheses

Parentheses override default precedence:

```mach
val x: i32 = (2 + 3) * 4;   # 20
```
