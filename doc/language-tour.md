# Mach Language Tour

This tour introduces the core Mach language features with small, focused examples. It links to detailed sections for each topic.

- If you prefer a deep dive, see: [Lexical Structure](lexical-structure.md), [Types](types.md), [Literals](literals.md), [Variables and Constants](variables.md), [Functions and Methods](functions-and-methods.md), [Expressions and Operators](expressions-and-operators.md), [Control Flow](control-flow.md), [Modules and Visibility](modules-and-visibility.md), [Records and Unions](records-and-unions.md), [Arrays and Slices](arrays-and-slices.md), [Pointers and Memory](pointers-and-memory.md), [Generics](generics.md), [Compile-time Features](compile-time.md), [Interoperability](interoperability.md), [Inline Assembly](inline-assembly.md), [Keywords](keywords-and-symbols.md), and the [Grammar](grammar.md).

## A minimal program

Top-level declarations define types, values, variables, functions, records, and unions. Use `pub` to export a declaration, and `use` to import modules.

```/dev/null/tour.mach#L1-20
use math: mylib.math;   # import with alias
use mylib.io;           # import by path

def Index: u64;         # type alias

pub fun add(a: u64, b: u64) u64 {
    ret a + b;
}

fun main() i64 {
    var x: u64 = 10;
    val y: u64 = 20;
    ret (add(x, y)) :: i64;  # explicit cast
}
```

- See [Modules and Visibility](modules-and-visibility.md), [Variables and Constants](variables.md), [Functions and Methods](functions-and-methods.md), and [Types](types.md).

## Values, variables, assignment

`val` defines an initialized constant; `var` defines a variable, optionally with an initializer. Statements end with `;`.

```/dev/null/tour.mach#L1-18
val max_items: u64 = 1024;

var count: u64;         # uninitialized
count = 1;

var total: u64 = 0;
total = total + count;

# address-of and dereference
var p: *u64 = ?total;   # take address
@p = @p + 41;           # write through pointer
```

- See [Pointers and Memory](pointers-and-memory.md) and [Expressions and Operators](expressions-and-operators.md).

## Records and unions

Define product types with `rec` and sum-like storage with `uni`. Fields are separated by `;` in declarations (trailing `;` optional).

```/dev/null/tour.mach#L1-30
pub rec Point {
    x: f64;
    y: f64;
}

pub uni Value {
    u: u64;
    f: f64;
}

fun length(p: Point) f64 {
    ret (p.x * p.x + p.y * p.y);
}

# Struct literal with named fields
val origin: Point = Point{ x: 0.0, y: 0.0 };


# Anonymous composite literals

val tmp_rec: rec { x: f64; y: f64; } = rec { x: 1.0, y: 2.0 };

val tmp_uni: uni { f: f64; } = uni { f: 3.14 };

```

- See [Records and Unions](records-and-unions.md) and [Literals](literals.md).

## Arrays and slices

Fixed-size arrays: `[N]T`. Slices (runtime-sized fat pointers) use `[]T`. Index with `[]`. Slices expose `.data` and `.len` fields.

```/dev/null/tour.mach#L1-20
val a: [3]u8  = [3]u8{ 1, 2, 3 };
var s: []u8   = []u8{ 10, 20, 30, 40 };
val first: u8 = a[0];
var i: u64 = 0;

for (i < s.len) {
    # iterate by index
    i = i + 1;
}
```

# Take a pointer to the first element (decay via field)
```
var data_ptr: *u8 = s.data;
```

- See [Arrays and Slices](arrays-and-slices.md) and [Control Flow](control-flow.md).

## Control flow

Mach provides `if` with `or` chains, `for` loops with optional conditions, `brk`, `cnt`, and `ret`.

```/dev/null/tour.mach#L1-28
fun classify(n: u64) u64 {
    if (n == 0) {
        ret 0;
    }
    or (n < 10) {
        ret 1;
    }
    or {
        ret 2;
    }
}

fun sum_to(n: u64) u64 {
    var acc: u64 = 0;
    var i:   u64 = 0;

    for (i < n) {
        acc = acc + i;
        i = i + 1;
    }

    ret acc;
}
```

- See [Control Flow](control-flow.md).

## Functions, methods, and calls

Functions use `fun name(params) ReturnType { ... }`. Variadics use `...` as the last parameter. Methods are declared with a receiver `(name: Type)` before the function name and are called as `value.method(...)`.

```/dev/null/tour.mach#L1-38
rec Counter {
    value: u64;
}

# Method with pointer receiver
fun (c: *Counter) inc() {
    c.value = c.value + 1;
}

# Method with value receiver
fun (c: Counter) get() u64 {
    ret c.value;
}


# Variadic function (external) declared at top level
ext "C:printf" printf: fun(*u8, ...) i32;

fun demo() {

    var c: Counter;
    c.value = 0;

    c.inc();             # auto-takes address to match *Counter receiver
    val v: u64 = c.get();


    # printf usage would depend on a string source; see Interoperability

}
```

- See [Functions and Methods](functions-and-methods.md) and [Interoperability](interoperability.md).

## Generics

Functions, records, and unions support type parameters in `[...]`. Instantiate generics by supplying type arguments.

```/dev/null/tour.mach#L1-28
rec Box[T] {
    value: T;
}

fun id[T](x: T) T {
    ret x;
}

fun use_generics() {
    val b_u64: Box[u64] = Box[u64]{ value: 42 };
    val forty_two: u64   = id[u64](b_u64.value);
}
```

- See [Generics](generics.md).

## Type syntax and casts

Type names may be qualified with a module alias (`alias.Type`). Pointers use `*T`. Function types use `fun(T1, T2, ...) Ret`. Cast with `expr :: Type`.

```/dev/null/tour.mach#L1-20
use net: mylib.net;

var p: *u8;
var f: fun(u64, u64) u64;

val t: u64  = 10;
val s: i64  = t :: i64;

val addr: *u8 = ?(@p);   # address-of and deref compose
```

- See [Types](types.md) and [Expressions and Operators](expressions-and-operators.md).

## Compile-time features ($)

The `$` prefix introduces compile-time expressions and statements:
- `$if (const_condition) { ... } or { ... }` includes one branch at compile time.
- `$size_of(T)`, `$align_of(T)`, `$offset_of(T, field)`, `$type_of(x)` return `u64` values known at compile time.
- `$category.field` accesses target/build constants (e.g., `$target.os`, `$build.debug`, `$mach.version`).
- `$symbol.attribute = value` sets attributes on declarations (e.g., override an emitted symbol name).

```/dev/null/tour.mach#L1-28
$if ($build.debug) {
    val dbg: u64 = 1;
}
or {
    val dbg: u64 = 0;
}

val sz_u64: u64 = $size_of(u64);

# Override emitted symbol name
$main.symbol = "main";
```

- See [Compile-time Features](compile-time.md).

## Interoperability (ext)

Declare external symbols with `ext`. The optional leading string specifies the calling convention and optional symbol name as `"Convention[:symbol]"`. The type on the right is a type, commonly a function type.

```/dev/null/tour.mach#L1-18
# Convention only
ext "C" puts: fun(*u8) i32;

# Convention and explicit symbol name
ext "C:puts" print_c: fun(*u8) i32;

# External constants or data can also be declared with types other than functions
ext "C:errno" errno: *i32;
```

- See [Interoperability](interoperability.md).

## Inline assembly

Embed inline assembly blocks with `asm { ... }`. A trailing `;` after the block is allowed.

```/dev/null/tour.mach#L1-12
fun fence() {
    asm {
        # target-specific barrier or instruction sequence
        # (content is target-dependent)
    };
}
```

- See [Inline Assembly](inline-assembly.md).

## Operators and precedence

Mach defines:
- Postfix: call `()`, index `[]`, field `.`, cast `::`
- Unary: logical not `!`, unary plus/minus `+`/`-`, bitwise not `~`, address-of `?`, dereference `@`
- Binary (low to high precedence): `=`; `||`; `&&`; `|`; `^`; `&`; `== !=`; `< > <= >=`; `<< >>`; `+ -`; `* / %`

- See [Expressions and Operators](expressions-and-operators.md) for details.

## Comments and literals

- Line comments start with `#` and run to the end of the line.
- Literals:
  - Integers: decimal, `0x` hex, `0b` binary, `0o` octal; underscores `_` allowed as digit separators
  - Floats: digits with a single `.`; underscores allowed
  - Chars: `'a'`, with escapes `\' \" \\ \n \t \r \0`
  - Strings: `"..."`, same escapes as chars
  - Null: `nil`

- See [Lexical Structure](lexical-structure.md) and [Literals](literals.md).

---

Continue exploring:
- [Modules and Visibility](modules-and-visibility.md)
- [Records and Unions](records-and-unions.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Compile-time Features](compile-time.md)
- [Interoperability](interoperability.md)
