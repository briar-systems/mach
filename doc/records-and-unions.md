# Records and Unions

This document covers composite data types in Mach: record types (`rec`) and union types (`uni`). It explains declaration syntax, fields, generic parameters, construction via literals, field access and mutation, and methods on types.

Related topics:
- [Types](types.md)
- [Literals](literals.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Generics](generics.md)
- [Functions and Methods](functions-and-methods.md)
- [Compile-time Features](compile-time.md)

## Overview

- Records (`rec`) are product types with named fields, similar to structs.
- Unions (`uni`) are overlapping-storage aggregates with named alternatives, similar to raw (untagged) unions.
- Declarations appear at top level and may be exported with `pub`.
- Generic records and unions accept type parameters in `[...]`.
- Composite values are constructed via typed literals like `Type{ name: value, ... }` or via anonymous composite literals.
- Fields are accessed with `.` and can be read or assigned when the base is assignable.

## Declarations

### Record declarations

- Syntax:
  - Named: `rec Name [TypeParams]? { FieldList }`
  - Anonymous (in type positions): `rec { FieldList }`

```/dev/null/records-and-unions.mach#L1-18
rec Point {
    x: f64;
    y: f64;
}

pub rec Pair[T] {
    a: T;
    b: T;
}
```

- Fields use `name: Type` with semicolons (`;`) as separators. A trailing semicolon is allowed.

### Union declarations

- Syntax:
  - Named: `uni Name [TypeParams]? { FieldList }`
  - Anonymous (in type positions): `uni { FieldList }`

```/dev/null/records-and-unions.mach#L1-18
uni Value {
    u: u64;
    f: f64;
}

pub uni Either[L, R] {
    left:  L;
    right: R;
}
```

- Unions provide raw overlapping storage for their fields; only one variant should be treated as active at a time.

### Visibility

- Add `pub` before `rec`/`uni` to export the type from the defining module.

```/dev/null/records-and-unions.mach#L1-8
pub rec Config {
    debug: u8;
    level: u64;
}
```

### Generics

- Records and unions may be generic over type parameters declared in `[...]`. Instantiate with `TypeName[Args]`.

```/dev/null/records-and-unions.mach#L1-24
rec Box[T] {
    value: T;
}

fun use_box() {
    val b_u64: Box[u64]   = Box[u64]{ value: 42 };
    val b_pair: Box[Pair[u64]] = Box[Pair[u64]]{ value: Pair[u64]{ a: 1, b: 2 } };
}
```

- See [Generics](generics.md) for parameter lists and instantiation rules.

## Fields and Access

- Access a field with `.`:
  - `record.field`
  - `union.field` (interprets the storage as the named field’s type)

```/dev/null/records-and-unions.mach#L1-24
rec Point { x: f64; y: f64; }

fun demo_fields() {
    var p: Point;
    p.x = 1.5;
    p.y = p.x + 2.5;     # read + assign

    uni Value { u: u64; f: f64; }
    var v: Value;
    v.u = 123;           # interpret storage as u64
    v.f = 3.14;          # reinterpret the same storage as f64
}
```

- Assignability:
  - `record.field` and `union.field` are assignable if the base is assignable (e.g., a `var` or a dereferenced pointer).
- For pointer receivers and nested aggregates, use address-of `?` and dereference `@` as needed (see [Expressions and Operators](expressions-and-operators.md)).

### Field list formatting rules

- Fields are written as `name: Type` inside `{ ... }`.
- Separate fields with `;`.
- A trailing `;` before `}` is allowed.

## Construction via Literals

Mach supports two complementary ways to construct record/union values:

1) Typed composite literals using a named type (including generic instantiations).
2) Anonymous composite literals starting directly with `rec`/`uni`.

### Typed literals

- Records: `Type{ field0: value0, field1: value1, ... }`
- Unions: `Type{ field: value }` (exactly one field should be provided)

```/dev/null/records-and-unions.mach#L1-30
rec Point { x: f64; y: f64; }
uni Value { u: u64; f: f64; }

val origin: Point = Point{ x: 0.0, y: 0.0 };

# union: initialize the active variant by naming the field
val v_u: Value = Value{ u: 123 };
val v_f: Value = Value{ f: 3.14 };

# generic typed literal
rec Box[T] { value: T; }
val bx: Box[u64] = Box[u64]{ value: 42 };
```

Notes:
- Field names in literals must match declared fields.
- For unions, provide exactly one field initializer to select the active variant.

### Anonymous composite literals (expression form)

- Records:
  - `rec { field0: value0, field1: value1, ... }`
- Unions:
  - `uni { field: value }`

```/dev/null/records-and-unions.mach#L1-22
val tmp_rec  = rec { x: 1.0, y: 2.0 };
val tmp_uni  = uni { f: 3.14 };

# use in context (e.g., parameter passing or assignment)
fun takes_point(p: rec { x: f64; y: f64; }) {
    # ...
}

takes_point(rec { x: 0.0, y: 0.0 });
```

- Anonymous composite literals are useful for ad-hoc construction without naming a type. In type positions, you can also write anonymous record/union types (`rec { ... }`, `uni { ... }`).

## Methods on Records and Unions

Methods are functions with a receiver, declared by placing a parameter in parentheses before the function name. The receiver can be a value or pointer to the composite type.

- Syntax: `fun (recv: Type) name(params) ReturnType { ... }`
- Pointer receiver: `fun (recv: *Type) name(...) { ... }`

```/dev/null/records-and-unions.mach#L1-50
rec Counter {
    value: u64;
}

# mutate via pointer receiver
fun (c: *Counter) inc() {
    c.value = c.value + 1;
}

# read via value receiver
fun (c: Counter) get() u64 {
    ret c.value;
}

fun method_demo() {
    var c: Counter;
    c.value = 0;

    # call via method syntax; auto address-of/deref is applied:
    # c.inc() takes &c if needed to match (*Counter)
    c.inc();
    val v: u64 = c.get();
}
```

- Method calls use `value.method(...)` syntax.
- If the receiver type in the method declaration is `*T` and you call on a `T`, the language automatically takes the address. If the receiver is `T` and you call on `*T`, it automatically dereferences.
- Methods are resolved based on the receiver’s type (including generic specializations).

See [Functions and Methods](functions-and-methods.md) for full method and function semantics.

## Pointers to Records and Unions

Use `?` (address-of) to obtain a pointer, and `@` (dereference) to access through pointers:

```/dev/null/records-and-unions.mach#L1-28
rec Pair { a: u64; b: u64; }

fun ptr_demo() {
    var p: Pair;
    p.a = 1;
    p.b = 2;

    var pp: *Pair = ?p;    # address-of
    @pp.a = 3;             # mutate through pointer
}
```

- A dereferenced pointer to a record/union is assignable, so fields remain assignable.
- Combine with method receivers for ergonomic APIs.

## Field Offsets and Layout Queries

Use compile-time intrinsics for layout-related queries:

- `$size_of(Type)` and `$align_of(Type)` return the size and alignment.
- `$offset_of(Type, field)` returns the byte offset of `field` within a record or union.

```/dev/null/records-and-unions.mach#L1-20
rec Header { tag: u32; len: u32; }

val sz:  u64 = $size_of(Header);
val al:  u64 = $align_of(Header);
val off: u64 = $offset_of(Header, len);
```

- See [Compile-time Features](compile-time.md) for these intrinsics.

## Best Practices

- Prefer records for product data (multiple fields used together).
- Use unions when you need a single storage location interpreted as different types; track the active variant using your own protocol (there is no built-in tag).
- Keep field names descriptive and consistent across related types.
- Use pointer receivers for methods that mutate the receiver or to avoid copies of large aggregates.
- For public APIs, export types with `pub` and consider providing helper constructors (functions that return initialized records).

## Summary

- Define product types with `rec` and overlapping-storage types with `uni`.
- Declare fields as `name: Type;` separated by semicolons.
- Construct with typed literals (`Type{ ... }`) or anonymous composite literals (`rec { ... }`, `uni { ... }`).
- Access with `.`; assign when the base is assignable.
- Add methods with a receiver; method calls support auto address-of/deref.
- Use compile-time intrinsics for size, alignment, and field offset information.

For how these types interact with expressions, casting, and function calls, see [Expressions and Operators](expressions-and-operators.md) and [Functions and Methods](functions-and-methods.md).