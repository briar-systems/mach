# Values and expressions

A binding names a value and states its type. Expressions build values from literals, names, operators and casts.

## Bindings

```mach
val limit: i64 = 100;
var count: i64;

fun step() {
    if (count < limit) { count = count + 1; }
}
```

| Form | Meaning |
|---|---|
| `val name: T = expr;` | Immutable, and the initializer is required |
| `var name: T;` | Mutable, starting at zero |
| `var name: T = expr;` | Mutable, starting at `expr` |

- Every binding states its type. `val n = 42;` is refused, and so is `val n = 42i64;`, since a suffix types the literal and not the binding.
- Zero is the default, in every byte: numbers are 0, pointers are `nil`, and every field and element is zero.
- A `val` is never assigned, and neither are its fields or elements. Through a `val p: *T` the pointee is still writable.

## Module-level values

Outside a function, a `val` is a constant and a `var` is a global, and [`pub`](modules.md) exports either. The initializer must be a compile-time value: literals, operators over constants, other `val`s and addresses of globals. A call is refused.

```mach
val KB:    u64    = 1024;
val MB:    u64    = KB * KB;
val SIZES: [2]u64 = [2]u64{KB, MB};
```

## Literals

| Literal | Examples |
|---|---|
| Integer | `42`, `1_000_000`, `0xFF`, `0b1010`, `0o755` |
| Float | `2.5`, `1.5e10` |
| Suffixed | `255u8`, `7i64`, `2.5f32` |
| Character, a `u8` | `'A'`, `'\n'`, `'\x1b'` |
| String, a `*u8` | `"hello\n"` |
| Null address | `nil` |
| Array, record | `[3]i64{1, 2, 3}`, `Point{x: 1, y: 2}` |

- An unsuffixed literal takes the type of its context: the binding it initializes, or the operand beside it. With no context an integer is `i64` and a float is `f64`.
- A suffixed literal has its suffix's type and is checked against that range, so `256u8` is refused. A float suffix needs a float literal: `1.0f32`, not `1f32`.
- The escapes are `\n` `\t` `\r` `\\` `\'` `\"` `\0` and `\xHH`.

Strings and vector literals are under [Types](types.md).

## Operators

| Operators, tightest first | Meaning |
|---|---|
| `f(x)` `a[i]` `a.b` `x::T` `x:~T` `x:>T` | Call, index, field and the casts |
| `-x` `!x` `~x` `?x` `@p` | Negate, logical not, bitwise not, address, dereference |
| `*` `/` `%` | Multiply, divide, remainder |
| `+` `-` | Add, subtract |
| `<<` `>>` | Shift |
| `<` `<=` `>` `>=` | Order |
| `==` `!=` | Equality |
| `&` | Bitwise and |
| `^` | Bitwise xor |
| `\|` | Bitwise or |
| `&&` | Logical and, short-circuit |
| `\|\|` | Logical or, short-circuit |
| `=` | Assignment |

```mach
fun low_nibble_set(flags: u32) u1 {
    ret (flags & 0x0f) != 0;
}
```

- Comparison binds tighter than `&`, `^` and `|`, so a bit test needs its parentheses.
- Assignment is `=` only. There is no `+=` and no `++`, so a counter is `i = i + 1;`.
- A prefix operator takes the whole postfix chain after it: `-x::i64` is `-(x::i64)`, and `@p.x` is `@(p.x)`.
- Comparisons give a `u1`, and `&&`, `||` and `!` take one. Integers of any two types compare by value. An integer against a float is refused, and so is `==` on a whole record, union or tag.
- The operands of an arithmetic or bitwise operator have one type. Nothing widens, so mixing is refused:

```text
error[operator.operand_mismatch] operands `i32` and `i64` have different types. cast one operand with `::` so both have one type
```

## Casts

```mach
fun casts(n: i32, f: f64) u64 {
    val wide:  i64 = n::i64;
    val whole: i64 = f::i64;
    val bits:  u64 = f:~u64;
    ret bits + (wide + whole)::u64;
}
```

| Cast | Meaning |
|---|---|
| `x::T` | Converts the value: resizes an integer, converts between integer and float, retypes a pointer |
| `x:~T` | Reads the same bits as a `T` of the same size |
| `x:>T` | Makes a secret `^T` public. See [Secrecy](secrecy.md) |

A float cast to an integer drops its fraction, and at run time a value out of range saturates.

The full rules are in the reference for [`val` and `var`](../language/val-var.md), [literals](../language/literals.md) and [operators](../language/operators.md).
