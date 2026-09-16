---
name: mach
description: Authoring guide for Mach (.mach) source files. Covers syntax, explicit typing, control flow, tags and guards, stdlib idioms, comptime constructs, decorators, and inline assembly.
---

# Mach Language Reference

Mach is a low-level, explicitly typed systems programming language without type inference, garbage collection, or hidden control flow. Source files use the `.mach` extension. This skill provides a dense reference for authoring correct Mach code. The authoritative language specification lives under [`doc/language/`](https://github.com/briar-systems/mach/tree/dev/doc/language).

## Critical Rules

Keep these core principles in mind when writing Mach to avoid common pitfalls:

- **No type inference**: Every `val` and `var` must declare its type explicitly. `val x: i64 = 42;` is valid, while `val x = 42;` is an error. Initializers are mandatory for `val`.
- **Zero-initialization guarantee**: Declaring `var x: T;` without an initializer safely zeroes all bytes: numeric types start at `0`, pointers at `nil`, arrays have zeroed elements, and tags default to their first declared variant.
- **No compiler-known type aliases**: `bool`, `usize`, `str`, and `char` are standard library definitions rather than built-in primitives. Always import them before use, such as `use std.types.bool.bool;` or `use std.types.size.usize;`. The literals `true` and `false` are imported values representing `1` and `0` of type `u8`.
- **Strings are raw pointers, not objects**: The type `str` is `*u8` pointing to null-terminated bytes. It has no `.len` or methods. The `==` operator compares pointer addresses rather than string contents. To inspect byte lengths or compare contents, use `std.types.string.str_len` and `std.types.string.str_equals`.
- **Conditionals use `if` and `or`**: Mach has no `else` or `else if` keyword. Use `or (condition)` for chained checks and a bare `or` block for the fallback branch.
- **Loops use `for`**: Mach has no `while` keyword. Use `for (condition) { ... }` for conditional loops and a bare `for { ... }` for infinite loops.
- **Blocks require braces**: Control flow bodies must always be enclosed in curly braces. Single-statement unbraced bodies are invalid.
- **Statements end in semicolons**: Every statement ends with `;` unless it terminates with a block.
- **No compound assignment**: Operators like `+=`, `-=`, `*=`, and `++` do not exist. Write assignments explicitly, such as `x = x + 1;`.
- **Pointers, auto-dereference, and precedence**: The address-of operator is `?x`. The dereference operator is `@p`. Member access on a pointer auto-dereferences once: write `p.x` when `p: *Point`. Never write `@p.x`, because `@` has lower precedence than `.` and parses as `@(p.x)`, which fails to type-check on non-pointer fields. Write through a pointer using `@p = value;` or `p.x = value;`.
- **No methods or UFCS**: Mach has no method syntax or uniform function call syntax. All operations are free functions called directly when in scope or imported with `use`, or reached through module paths and aliases, taking an explicit receiver: `push[T](?v, item)` or `vector.push[T](?v, item)` rather than `v.push(item)`.
- **Tagged values use `sel` and lexical guards**: Unions with an active variant are declared as `tag Name: u8 { case1; case2: T; }`. Construct variants using `Type.case{payload}` or `Type.empty{}`. Inspect the active case with `sel place.case`, which evaluates to `bool` (`u8`) and can be used in expressions, stored, or combined with `!`, `&&`, and `||`. Variant payloads are ordinary storage accessible wherever guarded: inside an `if (sel place.case)` block, in expressions on the right-hand side of `&&` (such as `if (sel r.ok && r.ok > 0)`), or after all alternative variant branches exit via `ret`, `brk`, or `cnt`. Whole tags can be assigned, passed, and returned, but cannot be compared with `==` or `!=`.
- **No error bubbling sugar**: Mach has no `try` keyword or `?` error propagation operator. Explicitly test and return errors: `if (sel outcome.err) { ret res[T, E].err{outcome.err}; }`.
- **Explicit generic call sites**: Generic function and type instantiations must always provide explicit type parameters at call sites, such as `identity[i64](42)`.
- **Decorators use `#[...]`**: Placed on the line or lines immediately preceding the declaration. Multiple decorators can be stacked one per line (such as `#[inline]` followed by `#[symbol("name")]`) or written space-separated on the same line.

## Project and Module Structure

A project is defined by a root `mach.toml` containing a `[project] id` string that roots all module paths. A source file at `src/net/tcp.mach` in project `id = "myproj"` defines the module `myproj.net.tcp`. Full module paths are always required, including when referencing sibling modules within the same project.

### Imports with `use`

Imports are private to the file and are placed at the top of the file following the module docstring:

```mach
use std.types.size;
use sz: std.types.size;
use std.types.size.usize;
```

Wildcard imports and grouped braces are unsupported. Each import occupies its own line.

### Re-exports with `fwd`

The `fwd` keyword re-exports symbols publicly with the same path syntax as `use`. It is always public and does not take the `pub` keyword:

```mach
fwd impl.Point;
fwd Pt: impl.Point;
fwd impl.helpers;
```

### Shadow Module Pattern

A surface module file `foo.mach` often coexists with a directory `foo/` containing modular implementation files. The surface file imports the splits and re-exports public items, allowing external consumers to write `use myproj.foo;` without coupling to internal file splits:

```mach
$if ($mach.build.os == $mach.os.linux) {
    use impl: myproj.os.linux;
}
$or ($mach.build.os == $mach.os.windows) {
    use impl: myproj.os.windows;
}
$or {
    $error("unsupported operating system");
}

fwd impl.page_size;
```

## Program Entrypoint and Standard Output

Executable targets link the standard library runtime to supply the platform `_start` symbol. Every entry file must include `use std.runtime;` even if no symbol is called directly. Export the program entry function as `main` using the `#[symbol("main")]` decorator:

```mach
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.println("hello, world");
    ret 0;
}
```

The `std.print` module provides output functions:
- `print.print("text")` and `print.println("text")` write to standard output.
- `print.eprint("text")` and `print.eprintln("text")` write to standard error.
- `print.printf("count: {}\n", count)` and `print.printlnf("hex: {:x}", value)` provide formatted output. Holes use `{}` and accept specifiers such as `{:x}`, `{:X}`, `{:5}`, `{:<5}`, and `{:08x}`. Literal braces are escaped as `{{` and `}}`.

## Declarations

The `pub` modifier exports a declaration. Without `pub`, declarations are private to the file.

### Type Aliases with `def`

```mach
pub def Age:      i64;
pub def BinaryOp: fun(i64, i64) i64;
```

### Records with `rec`

Records define aggregate data with named fields:

```mach
pub rec Point {
    x: i64;
    y: i64;
}

pub rec Pair[T, U] {
    left:  T;
    right: U;
}

fun make_records() {
    val pt:   Point         = Point{x: 10, y: 20};
    val pair: Pair[i64, u8] = Pair[i64, u8]{left: 1, right: 2u8};
}
```

### Raw Unions with `uni`

Unions provide untracked overlapping storage where all fields share the same offset:

```mach
pub uni Number {
    i: i64;
    f: f64;
}
```

### Tagged Unions with `tag`

A `tag` is a discriminated union representing exactly one active case at runtime. Declarations require an explicit unsigned integer discriminator type (`u8`, `u16`, `u32`, `u64`):

```mach
pub tag Status: u8 {
    pending;
    failed: i32;
    done:   i64;
}

pub tag Result[T, E]: u8 {
    err: E;
    ok:  T;
}
```

Construct variants using `Type.case{payload}` or `Type.empty{}` for cases without payloads:

```mach
pub tag Status: u8 {
    pending;
    failed: i32;
    done:   i64;
}

fun make_tags() {
    val s0: Status = Status.pending{};
    val s1: Status = Status.done{100};
    var s2: Status;
}
```

### Tag Inspection, Operations, and Guards

The active variant is inspected with `sel place.case`. Because `sel` evaluates to a boolean `u8`, it can be used directly in any expression, assigned to a variable, negated, or combined with logical operators:

```mach
pub tag Status: u8 {
    pending;
    failed: i32;
    done:   i64;
}

fun inspect(s: Status) i64 {
    if (sel s.done) {
        ret s.done;
    }
    or (sel s.failed) {
        ret -1;
    }
    ret 0;
}
```

Lexical guards govern access to variant payloads. Once guarded, a payload is ordinary mutable storage that can be read, compared, mutated (`place.case = val;`), or addressed (`?place.case`). Guards are established in three ways:

1. **Arm conditions**: Inside the body of an `if (sel place.case)` block.
2. **Conditional expressions**: On the right-hand side of `&&` when `sel place.case` is on the left, allowing direct comparisons and operations right in the condition.
3. **Exiting chains**: When preceding branches check other cases and exit with `ret`, `brk`, or `cnt`, the remaining block code is guarded for the remaining variant.

```mach
use std.types.bool.bool;
use std.types.bool.false;
use std.types.bool.true;

pub tag Status: u8 {
    pending;
    failed: i32;
    done:   i64;
}

fun check_status(s: Status) bool {
    # sel evaluates directly to a bool in any expression
    val is_pending: bool = sel s.pending;
    if (is_pending || (sel s.failed && s.failed < 0)) {
        ret false;
    }
    # s.done payload is guarded right here after sel on the left of &&
    if (sel s.done && s.done > 100) {
        ret true;
    }
    ret false;
}
```

Whole tags can be assigned, passed to functions, and returned, but whole tags cannot be compared with `==` or `!=`. There is no `match` or `switch` keyword.

#### Standard Result, Option, and Error Tags

The standard library provides canonical tags in `std.types`:
- `use std.types.result.res;` defines `pub tag res[T, E]: u8 { err: E; ok: T; }`. Case `err` is 0 so zero-initialized results default to failure.
- `use std.types.option.opt;` defines `pub tag opt[T]: u8 { none; some: T; }`.
- `use std.types.error.err;` defines `pub tag err[E]: u8 { err: E; ok; }` for unit return values that can fail.

Handling fallible outcomes follows an early exit pattern:

```mach
use std.types.result.res;

tag Error: u8 {
    io;
}

fun perform_work() res[i64, Error] {
    ret res[i64, Error].ok{42};
}

fun handle() res[i64, Error] {
    val outcome: res[i64, Error] = perform_work();
    if (sel outcome.err) {
        ret res[i64, Error].err{outcome.err};
    }
    # outcome.ok is unguarded and valid here because the error check returned early
    val value: i64 = outcome.ok;
    ret res[i64, Error].ok{value};
}
```

### Functions with `fun`

```mach
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}

pub fun identity[T](val: T) T {
    ret val;
}

pub fun log_message(msg: *u8) {
    # functions returning void omit the return type
    ret;
}
```

Generic functions must be instantiated with concrete types at call sites: `identity[i64](5)`.

### External Functions with `ext fun`

External functions declare foreign C-ABI signatures and end in a semicolon without a body:

```mach
#[symbol("write")]
pub ext fun libc_write(fd: i32, buf: *u8, count: u64) i64;

pub ext fun open(path: *u8, flags: i32, ...) i32;
```

### Variables with `val` and `var`

```mach
fun init_vars() {
    val max_retries:     i64 = 5;
    var current_attempt: i64 = 0;
    var scratch:         [128]u8;
}
```

### Tests with `test`

Tests are first-class declarations placed directly in source files alongside implementation code:

```mach
fun add(a: i64, b: i64) i64 {
    ret a + b;
}

test "math.add: basic sum" {
    if (add(2, 3) != 5) {
        ret 1;
    }
    ret 0;
}
```

The test body returns an integer exit code. Returning `0` or falling off the end denotes success. Any non-zero return value fails the test.

## Types and Literals

### Built-in Types

- **Integers**: `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`.
- **Floating point**: `f32`, `f64`.
- **Untyped pointer**: `ptr`.
- **Typed pointers**: `*T`. Pointers support array indexing `p[i]` and dereferencing `@p`. Member access on a pointer auto-dereferences once, so accessing a field through a pointer is written `p.x`.
- **Arrays**: `[N]T` where `N` is a compile-time constant expression, such as `[4]u32`. Array literals are written as `[4]u32{1, 2, 3, 4}`. Fixed arrays are value types copied on assignment and function calls. To pass array data without copying, pass a pointer `*[N]T` or pass pointer and length pairs. The standard library also defines `std.types.view.View` as one common slice record representation. The inferred array syntax `[_]u8` is permitted only on `val` bindings annotated with `#[embed("path")]`.
- **SIMD vectors**: Formatted as `<scalar>x<lanes>` such as `f32x4`, `i32x8`, and `u8x16`. Literals are positional expressions like `f32x4{1.0, 2.0, 3.0, 4.0}`. Lane access uses compile-time indexing `vec[0]`.
- **Function pointers**: `fun(ParamTypes) ReturnType`, such as `fun(i64, *u8) i64`.
- **Secret qualifier**: `^T` marks values containing sensitive data subject to constant-time restrictions. Secret values cannot be branched on, looped on, or used as array indices or memory addresses. The only way to remove the secret qualifier is an explicit downgrade cast `val:>T`.

```mach
pub rec Point {
    x: i64;
    y: i64;
}

fun update_point(p: *Point) {
    # member access auto-dereferences p once: do not write @p.x
    p.x = 10;
    p.y = 20;
}
```

### Literals and Strings

- **Integers**: Decimal `42`, hexadecimal `0x2A`, binary `0b101010`, octal `0o52`, with optional digit separators `1_000_000`. Typed integer suffixes include `42u8`, `100i32`, `500u64`.
- **Floats**: `3.14`, `1.0e-5`, with optional typed suffixes `2.5f32`, `0.1f64`.
- **Characters**: `'A'` represents a `u8` byte value. Standard escape sequences include `\n`, `\t`, `\r`, `\\`, `\'`, `\0`, `\xHH`.
- **Strings**: `"hello\n"` represents a null-terminated `*u8` pointer to static memory. Multi-line strings are not supported directly, so embed newline escapes instead. Strings do not have a `.len` property, and `==` compares pointer addresses. Use standard library functions to measure length or compare contents:

```mach
use std.types.bool.bool;
use std.types.size.usize;
use std.types.string.str;
use std.types.string.str_equals;
use std.types.string.str_len;

fun demo_strings(a: str, b: str) bool {
    val len: usize = str_len(a);
    ret str_equals(a, b);
}
```

- **Null address**: `nil` represents a null pointer or nil function pointer and coerces to any pointer or function type.

## Operators and Casts

- **Arithmetic**: `+`, `-`, `*`, `/`, `%` operate on integers and floats. `%` computes truncated remainder.
- **Bitwise**: `&`, `|`, `^` (xor), `~` (bitwise not), `<<`, `>>`. Bitwise operators have lower precedence than comparison operators, so always parenthesize bit checks: `(flags & 1u32) != 0`.
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=` return `u8` (1 or 0). Mixed signedness and width comparisons compare true mathematical values without surprise wraps. Records, unions, and tags cannot be compared with `==` or `!=`.
- **Logical**: `&&`, `||`, `!` short-circuit and operate on `u8` values.
- **Pointer operators**: `?x` produces the address of variable `x`. `@p` dereferences pointer `p`. Write through a pointer using `@p = value;`.
- **Casts**:
  - `expr::T` performs value conversion (resizing integer widths, conversions between integer and float).
  - `expr:~T` performs bitwise reinterpretation between types of identical byte sizes.
  - `expr:>T` strips the `^` secret qualifier from a secret value, returning public type `T`.
- **Assignment**: `=` is right-associative and functions as an expression in statement positions.

## Control Flow Statements

Statements terminate with a semicolon unless ending with a curly-braced block.

### If and Or

```mach
fun classify(score: i64) i64 {
    var grade: i64 = 0;
    if (score > 90) {
        grade = 1;
    }
    or (score > 75) {
        grade = 2;
    }
    or {
        grade = 3;
    }
    ret grade;
}
```

### For Loops

Mach uses `for` exclusively for looping:

```mach
fun loop_demo() {
    var i: i64 = 0;
    for (i < 10) {
        i = i + 1;
    }

    for {
        # infinite loop
        if (i >= 20) {
            brk;
        }
        cnt;
    }
}
```

### Fin Cleanups

The `fin` block acts as a deferred cleanup. It executes upon exit of the enclosing lexical block in reverse order of declaration:

```mach
fun acquire_resource() *u8 {
    ret nil;
}

fun release_resource(res: *u8) {}

fun process_data() {
    val resource: *u8 = acquire_resource();
    fin {
        release_resource(resource);
    }
}
```

### Jump Statements

- `ret;` returns from a void function.
- `ret expr;` returns a value.
- `brk;` breaks out of the innermost enclosing loop.
- `cnt;` jumps to the next iteration of the innermost loop.

## Docstrings

Document declarations using `#` comments placed immediately above the item. The first line is a lowercase summary without a trailing period. Detailed parameter, field, and return documentation follows an optional `# ---` separator with description columns aligned:

```mach
# calculate the sum of two integers
# ---
# a:   first integer operand
# b:   second integer operand
# ret: sum of both operands
pub fun add(a: i64, b: i64) i64 {
    ret a + b;
}
```

## Comptime Channel

The `$` prefix accesses compile-time features, target inspection, reflection, and conditional compilation. Comptime constructs select and expand code at compile time rather than executing runtime code.

### Target and Compiler State with `$mach.*`

Compile-time constants inspection keys:
- `$mach.build.os`: matches `$mach.os.linux`, `$mach.os.darwin`, `$mach.os.windows`, `$mach.os.freestanding`
- `$mach.build.arch`: matches `$mach.arch.x86_64`, `$mach.arch.aarch64`, `$mach.arch.riscv64`
- `$mach.build.mode`: matches `$mach.mode.debug`, `$mach.mode.release`
- `$mach.build.pointer_width`: pointer size in bytes (8 on 64-bit systems)

### Manifest State with `$project.*` and `$bin.*`

- `$project.id`: project identifier from `mach.toml`
- `$project.version`: version string
- `$project.version.major`: integer version component
- `$bin.name`: current artifact name

### Conditional Compilation with `$if` and `$or`

Code within unselected branches is discarded before type-checking and name resolution, making cross-platform code completely safe:

```mach
$if ($mach.build.os == $mach.os.linux) {
    pub val PLATFORM: *u8 = "linux";
}
$or ($mach.build.os == $mach.os.windows) {
    pub val PLATFORM: *u8 = "windows";
}
$or {
    $error("unsupported operating system");
}
```

### Comptime Function Parameters

Prefixing a function parameter with `$` forces arguments to be evaluated at compile time. The compiler monomorphizes the function for each distinct value. The parameter is referenced bare within the function body:

```mach
pub fun shift_left($count: u8, val: u64) u64 {
    $if (count >= 64) {
        $error("shift count exceeds integer width");
    }
    ret val << count;
}
```

### Variadic Packs

A trailing parameter declared as `va: ...` collects arbitrary arguments into a compile-time sequence. Iterate over the arguments with `$each`, check the count with `va.len`, or forward the entire pack with `callee(va...)`:

```mach
use std.print;
use std.types.string.str;

pub fun print_all(va: ...) {
    $each arg in va {
        $if ($type_of(arg) == i64) {
            print.printlnf("int: {}", arg);
        }
        $or ($type_of(arg) == str) {
            print.println(arg);
        }
    }
}
```

### Intrinsics

- `$size_of(T)`: byte size of type `T`.
- `$align_of(T)`: byte alignment of type `T`.
- `$length_of(T)`: element count of array or vector type `T`.
- `$offset_of(T, field)`: byte offset of a record field.
- `$type_of(expr)`: compile-time type value for comparison inside `$if` conditions.
- `$fields(T)`: sequence of record or union field descriptors, consumed with `$each f in $fields(T) { val field_val = instance.[f]; }`.
- `$cases(T)`: sequence of tag case descriptors, consumed with `$each c in $cases(T)`.
- `$is_tag(T)`, `$is_record(T)`, `$is_union(T)`, `$is_pointer(T)`, `$is_secret(T)`: type reflection predicates.
- `$error("message")`: produces a compile error when encountered in an active branch.

## Decorators

Decorators attach compiler metadata to declarations. Each directive is written in its own `#[name]` or `#[name(arg)]` clause. Multiple decorators can be stacked across several lines immediately preceding the declaration or written space-separated on one line:

```mach
#[inline]
#[symbol("fast_calc")]
pub fun fast_calc(x: i64) i64 {
    ret x * 2;
}
```

- `#[symbol("name")]`: overrides the exported or imported linker symbol name.
- `#[inline]`: forces the compiler to inline a function.
- `#[align(N)]`: overrides alignment on a type, record field, or variable.
- `#[packed]`: removes alignment padding from a record, union, or tag.
- `#[section(".name")]`: places a function or global variable into a specific object file section.
- `#[embed("path")]`: embeds the raw bytes of an external file at compile time into an uninitialized `val name: [_]u8;` array.
- `#[library("name")]`: specifies the dynamic library name required for an external import.
- `#[deprecated]` or `#[deprecated("message")]`: emits a compile warning whenever the decorated symbol is used.
- `#[scalar]`: opts a function out of automatic vectorization.
- `#[naked]`: emits a function without prologue or epilogue code, intended for functions written entirely with inline assembly.
- `#[oblivious]`: marks a function as constant-time, verifying that no secret-dependent branches or variable-latency operations are emitted.

## Inline Assembly

Inline assembly uses an ISA-tagged block with native instruction text:

```mach
pub fun pause() {
    $if ($mach.build.arch == $mach.arch.x86_64) {
        asm x86_64 {
            pause
        }
    }
    $or ($mach.build.arch == $mach.arch.aarch64) {
        asm aarch64 {
            isb
        }
    }
    $or ($mach.build.arch == $mach.arch.riscv64) {
        asm riscv64 {
            pause
        }
    }
}
```

Rules for inline assembly:
- The architecture tag (`x86_64`, `aarch64`, `riscv64`) is mandatory.
- Operands reference local variables via `{name}` substitution. The compiler resolves substitutions to registers or stack slots based on instruction semantics.
- Never write double indirections like `[{ptr}]`. Always load the address into a register first, then dereference the register.
- The compiler automatically infers clobber sets and assumes a memory clobber. Manual clobber lists do not exist.
