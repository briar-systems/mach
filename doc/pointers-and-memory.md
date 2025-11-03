# Pointers and Memory

This document explains how pointers work in Mach: pointer types, the address-of (`?`) and dereference (`@`) operators, what counts as an lvalue (assignable expression), and how pointers interact with casts, arrays/slices, fields, methods, and foreign calls.

Related topics:
- [Types](types.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Records and Unions](records-and-unions.md)
- [Functions and Methods](functions-and-methods.md)
- [Interoperability](interoperability.md)

## Pointer types

- A pointer to a type `T` is written `*T`.
- Pointers can be stored in variables, fields, passed as parameters, and returned from functions.
- The null literal for pointer-like values is `nil`.

Examples:
    
    var p: *u8 = nil;
    var q: *u64;

## Address-of operator `?`

The address-of operator produces a pointer to its operand.

- Syntax: `?expr`
- Type: if `expr` has type `T` and is an lvalue (assignable), then `?expr` has type `*T`.

Examples:
    
    var x: u64 = 42;
    var px: *u64 = ?x;  # pointer to x

    rec Pair { a: u64; b: u64; }
    var p: Pair;
    var pa: *u64 = ?p.a;   # pointer to field

    var buf: [4]u8 = [4]u8{ 1, 2, 3, 4 };
    var p0: *u8 = ?buf[0]; # pointer to first element

Rules:
- The operand must be assignable (an lvalue). See “Lvalues” below.

## Dereference operator `@`

The dereference operator accesses the value pointed to by a pointer.

- Syntax: `@expr`
- Type: if `expr` has type `*T`, then `@expr` has type `T`.
- As a statement target, `@expr` is assignable (lvalue), enabling writes through pointers.

Examples:
    
    var x: u64 = 1;
    var px: *u64 = ?x;
    @px = @px + 41;    # x becomes 42

    var s: []u8 = []u8{ 10, 20 };
    var d: *u8 = s.data;
    @d = 11;           # write first byte via pointer

Notes:
- Use indexing for arrays/slices; do not attempt to “step” pointers with arithmetic (see Best practices).

## Lvalues (assignable expressions)

Assignment requires the left-hand side to be an lvalue. The following are lvalues:
- A `var` variable
- A dereferenced pointer: `@ptr`
- A field of an assignable base: `obj.field`
- An indexed element of an assignable base: `array[i]`, `slice[i]`

Examples:
    
    var x: u64 = 0;
    x = 1;

    var px: *u64 = ?x;
    @px = 2;

    rec Point { x: f64; y: f64; }
    var p: Point;
    p.x = 1.5;

    var a: [3]u8 = [3]u8{ 1, 2, 3 };
    a[0] = 7;

    var s: []u8 = []u8{ 10, 20, 30 };
    s[1] = 25;

For full assignment rules, see [Expressions and Operators](expressions-and-operators.md).

## Pointer casts and conversions

Mach uses explicit casts with `::`. High-level rules:
- Between pointer types: `ptr :: *U` converts when allowed by the type system.
- Slice to pointer “decay”: `[]T` can yield its data pointer `*T` with an explicit cast, or implicitly in call contexts (see below).
- Pointer to slice: requires a length; there is no automatic construction of `[]T` from `*T`.

Examples:
    
    var px: *u8;
    var py: *u64;
    px = (?py) :: *u8;             # explicit pointer cast

    var s: []u8 = []u8{ 10, 20, 30 };
    var p: *u8  = (s) :: *u8;      # decay to data pointer via cast
    # often you can just use s.data

    # Passing to a function that expects *u8:
    ext "C:puts" puts: fun(*u8) i32;
    puts("Hello!\n");              # string literal provides *u8
    puts(s.data);                  # pass the slice’s data pointer
    puts((s) :: *u8);              # equivalent explicit cast

Notes:
- If a function parameter is a pointer type `*U`, passing a slice `[]T` may use its data pointer automatically when compatible. See [Functions and Methods](functions-and-methods.md).
- Conversions between unrelated pointer types and between pointers and integers should be avoided unless an API explicitly requires them.

## Pointers with arrays and slices

Arrays `[N]T` and slices `[]T` interact naturally with pointers:

- First element pointer:
    
    var a: [3]u8 = [3]u8{ 1, 2, 3 };
    var p0: *u8 = ?a[0];

- Slice fields:
    
    var s: []u8 = []u8{ 10, 20, 30 };
    var data: *u8 = s.data;  # pointer
    var n: u64  = s.len;     # length

- Passing to foreign APIs:
    
    ext "C:memset" memset: fun(*u8, i32, u64) *u8;
    memset(s.data, 0, s.len);   # zero a byte slice

Indexing and assignment:
- For fixed arrays, indexing computes an element address. For slices, indexing is bounds-checked at runtime.

See [Arrays and Slices](arrays-and-slices.md) for full details.

## Pointers with records and unions

- Take addresses of fields with `?obj.field`.
- Read and write through the pointer with `@` and field access:
    
    rec Header { tag: u32; len: u32; }
    var h: Header;
    var ph: *Header = ?h;
    @ph.tag = 7;     # write through pointer

- For unions, field access reinterprets the shared storage as that field’s type. Use care when writing and reading different fields.

See [Records and Unions](records-and-unions.md).

## Methods and pointer receivers

Methods can declare either value or pointer receivers. Method calls apply auto address-of/deref to match the receiver type:

- If the method expects `*T` and you call on `T`, the language takes the address.
- If the method expects `T` and you call on `*T`, the language dereferences.

Examples:
    
    rec Counter { value: u64; }

    fun (c: *Counter) inc() {
        c.value = c.value + 1;
    }

    fun (c: Counter) get() u64 {
        ret c.value;
    }

    var c: Counter;
    c.value = 0;
    c.inc();           # auto &c to match *Counter
    var v: u64 = c.get();

See [Functions and Methods](functions-and-methods.md).

## Null pointers and comparisons

- `nil` is the null literal for pointer-like values.
- Compare pointers with `==` / `!=`. The result is `u8` (0/1).
- Do not use pointers directly as conditions; compare to `nil` (or produce a `u8` by other means).

Examples:
    
    var p: *u8 = nil;
    if (p == nil) {
        # handle null
    }
    or {
        # non-null
    }

## Best practices

- Prefer slices `[]T` for passing buffers; they carry both data pointer and length. Use `slice.data` and `slice.len` when interacting with foreign APIs.
- Avoid pointer arithmetic. Use array/slice indexing and explicit indices. If you must compute addresses, do so with clear integer offsets and explicit casts, and isolate such code near interop boundaries.
- Keep pointer lifetimes clear. Avoid storing pointers to short-lived locals beyond their scope.
- Use `?` and `@` sparingly in high-level code; keep raw pointer manipulation localized and well-documented.
- Always check for `nil` where appropriate before dereferencing.

## Examples

Address-of, dereference, and assignment:
    
    var x: u64 = 1;
    var px: *u64 = ?x;
    @px = @px + 41;     # x == 42

Arrays/slices with pointers:
    
    var a: [3]u8 = [3]u8{ 1, 2, 3 };
    var p0: *u8 = ?a[0];

    var s: []u8 = []u8{ 10, 20, 30 };
    var d: *u8 = s.data;
    @d = 11;            # s[0] becomes 11

Interop with foreign functions:
    
    ext "C:memcpy" memcpy: fun(*u8, *u8, u64) *u8;

    var dst: [4]u8 = [4]u8{ 0, 0, 0, 0 };
    var src: []u8  = []u8{ 1, 2, 3, 4 };

    # pass pointer to dst[0] and src slice data
    memcpy(?dst[0], src.data, src.len)

    ext "C:puts" puts: fun(*u8) i32;
    puts("Hello!\n")    # string literal as *u8

Pointers with records:
    
    rec Point { x: f64; y: f64; }
    var p: Point;
    var px: *u64 = ?p.x
    @px = 3       # p.x becomes 3.0 (implicit numeric conversion must be valid); use casts if needed