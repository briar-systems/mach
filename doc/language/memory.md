# Pointers and Memory

Mach provides direct pointer operations with no garbage collection or automatic memory management.


## Pointer Types

- `*T` -- mutable pointer to T
- `&T` -- read-only pointer to T
- `ptr` -- untyped raw pointer (like C `void*`)

Mutable pointers (`*T`) can be implicitly used where a read-only pointer (`&T`) is expected, but not the reverse.


## Address-of (?)

The `?` operator takes the address of a value:

```mach
var x: i32 = 42;
val p: *i32 = ?x;   # mutable pointer (x is var)

val y: i32 = 100;
val q: &i32 = ?y;   # read-only pointer (y is val)
```


## Dereference (@)

The `@` operator reads or writes through a pointer:

```mach
var x: i32 = 42;
var p: *i32 = ?x;

val value: i32 = @p;   # read: 42
@p = 100;               # write: x is now 100
```

Dereferencing a read-only pointer yields a value that cannot be assigned to. Dereferencing a null or invalid pointer is undefined behavior.


## Null Pointers

`nil` represents a null pointer and can be assigned to any pointer type:

```mach
val p: *i32 = nil;
val s: str = nil;
```


## Field Access Through Pointers

The `.` operator works uniformly on both values and pointers:

```mach
rec Point { x: f32; y: f32; }

var p: Point = Point{x: 1.0, y: 2.0};
var ptr: *Point = ?p;
val x: f32 = ptr.x;   # automatic dereference
```


## Array Indexing

Both arrays and pointers support `[]` indexing:

```mach
val arr: [5]i32 = [5]i32{10, 20, 30, 40, 50};
val elem: i32 = arr[2];   # 30

val p: &i32 = ?arr[0];
val same: i32 = p[2];     # also 30
```


## Memory Safety

Mach does not enforce memory safety at compile time or runtime. The programmer is responsible for:

- Checking pointers before dereferencing
- Avoiding use-after-free
- Preventing buffer overflows
- Managing allocation and deallocation

The standard library provides allocator types (arena, heap) for structured memory management.
