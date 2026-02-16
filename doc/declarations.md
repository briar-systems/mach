# Declarations

Mach has a small set of declaration forms. All declarations exist at module scope or inside function bodies (where applicable).


## Variables

### val (immutable binding)

```mach
val x: i32 = 42;
val name: str = "hello";
```

A `val` binding cannot be reassigned after initialization. Taking its address with `?` yields a read-only pointer (`&T`).

### var (mutable binding)

```mach
var count: i32 = 0;
count = count + 1;
```

A `var` binding can be reassigned. Taking its address with `?` yields a mutable pointer (`*T`).


## Functions

Functions are declared with `fun`:

```mach
fun add(a: i32, b: i32) i32 {
    ret a + b;
}
```

Functions with no return value omit the return type:

```mach
fun greet(name: str) {
    print.printf("hello %s\n", name);
}
```

### Variadic Functions

The `...` token after the last named parameter makes a function variadic:

```mach
fun printf(format: str, ...) {
    var ap: va_list;
    va_start(?ap);
    # use va_arg[T](?ap) to extract arguments
    va_end(?ap);
}
```

See [variadic.md](variadic.md) for details.

### Methods

A receiver parameter before the function name associates it with a type:

```mach
fun (this: *Point) translate(dx: f32, dy: f32) {
    this.x = this.x + dx;
    this.y = this.y + dy;
}
```

See [types.md](types.md#methods) for method semantics.


## Records

Records group named fields:

```mach
rec Point {
    x: f64;
    y: f64;
}
```

Generic records:

```mach
rec Pair[T, U] {
    first: T;
    second: U;
}
```


## Unions

Unions share memory between named fields:

```mach
uni Value {
    i: i64;
    f: f64;
    p: ptr;
}
```

The size of a union equals its largest field.


## Type Aliases

```mach
def Byte = u8;
def NodePtr = *Node;
```

Aliases are fully interchangeable with the underlying type.


## External Declarations

The `ext` keyword declares a function defined elsewhere (foreign/FFI):

```mach
ext fun memcpy(dst: ptr, src: ptr, n: usize) ptr;
```

External declarations have no body. The linker resolves them at link time.


## Visibility

The `pub` keyword makes any declaration visible outside its module:

```mach
pub rec Point { x: f64; y: f64; }
pub fun create_point(x: f64, y: f64) Point { ... }
pub val MAX_SIZE: i32 = 1024;
```

Without `pub`, declarations are private to their module.


## Test Blocks

Test blocks declare inline tests:

```mach
test "addition works" {
    val result: i32 = 1 + 1;
    if (result != 2) { ret 0; }
    ret 1;
}
```

See [testing.md](testing.md) for the test framework.


## Comments

Line comments begin with `#`:

```mach
# this is a comment
val x: i32 = 42;   # inline comment
```

There are no block comments.
