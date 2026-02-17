# Cheatsheet


## Toolchain

| Task | Command |
|------|---------|
| Build default target | `mach build .` |
| Build specific target | `mach build . --target linux` |
| Run the last build | `mach run .` |
| Run all tests | `mach test .` |
| Init a new project | `mach init my-app` |
| List dependencies | `mach dep list` |
| Add remote dependency | `mach dep add https://github.com/org/pkg --version branch/main` |
| Pull/update dependencies | `mach dep pull` |


## Types

| Type | Description |
|------|-------------|
| `u8` `u16` `u32` `u64` | Unsigned integers |
| `i8` `i16` `i32` `i64` | Signed integers |
| `f32` `f64` | Floating-point |
| `ptr` | Untyped pointer (`void*`) |
| `*T` | Mutable pointer to T |
| `&T` | Read-only pointer to T |
| `[N]T` | Fixed-size array of N elements of type T |


## Keywords

| Keyword | Purpose |
|---------|---------|
| `val` | Immutable binding |
| `var` | Mutable binding |
| `fun` | Function/method declaration |
| `rec` | Record (struct) type |
| `uni` | Union type |
| `def` | Type alias |
| `use` | Module import |
| `ext` | External symbol (FFI) |
| `pub` | Public visibility |
| `if` / `or` | Conditional (`or` = else-if / else) |
| `for` | Loop (condition optional) |
| `ret` | Return |
| `brk` | Break |
| `cnt` | Continue |
| `fin` | Defer (block-scoped, LIFO) |
| `asm` | Inline assembly |
| `test` | Test block |


## Operators

| Operator | Description |
|----------|-------------|
| `?x` | Address-of |
| `@x` | Dereference |
| `x::T` | Type cast |
| `x.f` | Field access (auto-deref through pointers) |
| `x[i]` | Indexing |
| `!` `-` `~` | Logical NOT, negation, bitwise NOT |
| `+ - * / %` | Arithmetic |
| `== != < > <= >=` | Comparison |
| `&& \|\|` | Logical AND/OR |
| `& \| ^ << >>` | Bitwise |
| `=` | Assignment |


## Declarations

```mach
val x: i32 = 42;                # immutable
var y: i32 = 0;                  # mutable
def Seconds: i64;                # type alias

rec Point { x: f32; y: f32; }   # record
uni Value { i: i64; f: f64; }   # union

fun add(a: i32, b: i32) i32 {   # function
    ret a + b;
}

fun (this: *Point) move(dx: f32, dy: f32) {  # method
    this.x = this.x + dx;
}

pub fun exported() { }           # public function
```


## Imports

```mach
use std.types.bool;              # bare: symbols available directly
use print: std.print;            # aliased: use as print.println(...)
```


## Control Flow

```mach
if (x > 0) { ret 1; }
or (x < 0) { ret -1; }
or          { ret 0; }

var i: i32 = 0;
for (i < 10) {
    if (i == 5) { brk; }
    i = i + 1;
}

for { brk; }                     # infinite loop
```


## Pointers and Memory

```mach
var x: i32 = 42;
val p: *i32 = ?x;               # address-of
val v: i32 = @p;                 # dereference
@p = 100;                        # write through pointer
val n: *i32 = nil;               # null pointer
```


## Entry Point

```mach
use std.runtime;
use print: std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.printlnf("hello %s", "world");
    ret 0;
}
```


## Compile-Time

```mach
val size: u64 = $size_of(MyType);
val align: u64 = $align_of(MyType);
val off: u64 = $offset_of(MyType, field);

$if ($mach.build.target.os.id == $mach.os.linux.id) {
    # linux-specific code
} or {
    # fallback
}

$my_func.symbol = "custom_name";  # override linker symbol
```


## Generics

```mach
rec Pair[T, U] { first: T; second: U; }
fun identity[T](x: T) T { ret x; }

val p: Pair[i32, str] = Pair[i32, str]{first: 42, second: "hello"};
val n: i32 = identity[i32](42);
```
