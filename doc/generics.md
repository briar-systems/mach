# Generics

This document explains type parameterization in Mach: how to declare generic functions, records, and unions; how to supply type arguments; and how to use generics with methods, typed literals, and qualified names.

Related topics:
- [Types](types.md)
- [Functions and Methods](functions-and-methods.md)
- [Records and Unions](records-and-unions.md)
- [Arrays and Slices](arrays-and-slices.md)
- [Expressions and Operators](expressions-and-operators.md)
- [Modules and Visibility](modules-and-visibility.md)
- [Literals](literals.md)

## Overview

- Declarations may introduce type parameters in square brackets after the name: `[T, U, ...]`.
- Supported generic declarations:
  - Functions: `fun name[T, U](...) ... { ... }`
  - Methods: `fun (recv: Type) name[T, U](...) ... { ... }`
  - Records: `rec Name[T, U] { ... }`
  - Unions: `uni Name[T, U] { ... }`
- Use type arguments by writing `Name[Arg1, Arg2, ...]` at the use site:
  - In type positions (e.g., fields, parameters, returns)
  - In expressions for typed literals: `Name[Args]{ ... }`
  - In generic calls: `func[Args](...)` and `value.method[Args](...)`
- Type arguments are types; value arguments are not supported in type parameter lists.
- There are no constraints or default type arguments on parameters; provide all type arguments explicitly where required.

## Declaring type parameters

Place a bracketed list of one or more identifiers immediately after the declaration name.

- Syntax rules:
  - Each parameter is an identifier (e.g., `T`, `Key`, `Elem`).
  - Parameters are comma-separated within `[...]`.
  - Parameters are in scope within the declaration’s body (fields, parameter and return types, nested type expressions).

Examples (declarations):

    # generic record
    rec Box[T] {
        value: T;
    }

    # generic union
    uni Either[L, R] {
        left:  L;
        right: R;
    }

    # generic function
    fun id[T](x: T) T {
        ret x;
    }

    # generic method (receiver may be generic or non-generic)
    rec Pair[T] { a: T; b: T; }

    fun (p: *Pair[T]) swap() {
        var tmp: T = p.a;
        p.a = p.b;
        p.b = tmp;
    }

Notes:
- Methods may appear on generic types and may also introduce their own type parameters (see below).
- Use pointer receivers (e.g., `*Pair[T]`) to mutate the receiver.

## Using type arguments

Supply concrete types in square brackets `[...]` when referencing a generic name.

### In type positions

- Named types:

      val a: Box[u64];
      val e: Either[u8, u64];

- Qualified type names:

      use coll: mylib.collections;
      val q: coll.Queue[u8];

- As fields and nested types:

      rec Node[T] {
          value: T;
          next:  *Node[T];
      }

### In typed literals

Typed literals allow construction of generic values by prefixing the literal with the instantiated type:

    val b: Box[u64] = Box[u64]{ value: 42 };
    val v: Either[u64, f64] = Either[u64, f64]{ right: 3.14 };

For records and unions, supply named field initializers as usual. See [Records and Unions](records-and-unions.md).

### In generic calls

Provide type arguments before the argument list:

    val x: u64 = id[u64](10);

For methods with their own type parameters:

    rec Holder { tag: u64; }

    fun (h: Holder) wrap[U](x: U) Box[U] {
        ret Box[U]{ value: x };
    }

    fun demo() {
        var h: Holder;
        val b: Box[u64] = h.wrap[u64](5);
    }

Notes:
- Provide all required type arguments; there is no partial application or defaulting.
- The argument list `()` follows the type argument list `[...]`.

## Generic records and unions

- Declare type parameters on the type name:

      rec Result[T] {
          ok:  u8;
          val: T;
      }

      uni Value[T] {
          i: i64;
          t: T;
      }

- Instantiate the types with concrete arguments wherever a type is expected:

      val r: Result[u64] = Result[u64]{ ok: 1, val: 42 };
      val w: Value[f64]  = Value[f64]{ t: 2.5 };

- Methods can be declared for generic records/unions:

      rec Pair[T] { a: T; b: T; }

      fun (p: Pair[T]) first() T { ret p.a; }
      fun (p: *Pair[T]) set_first(x: T) { p.a = x; }

      fun use_pair() {
          var p: Pair[u64] = Pair[u64]{ a: 1, b: 2 };
          val a0: u64 = p.first();
          p.set_first(10);
      }

- Generic parameters may be used in nested type expressions:

      rec MapEntry[K, V] { key: K; value: V; }
      rec Map[K, V]      { entries: []MapEntry[K, V]; }

## Generic functions

- Define with type parameters in `[...]` after the function name:

      fun choose[T](a: T, b: T, use_a: u8) T {
          if (use_a) { ret a; } or { ret b; }
      }

- Call with type arguments at the call site:

      val sel: u64 = choose[u64](10, 20, 1);

- Generic functions can return generic types:

      fun singleton[T](x: T) []T {
          ret []T{ x };
      }

      val one_i64: []i64 = singleton[i64](7);

## Generic methods

Methods can be:
- On a generic receiver, using the receiver’s type parameters:

      rec Vec2[T] { x: T; y: T; }

      fun (v: Vec2[T]) length_sq() T {
          ret v.x * v.y;  # example; use appropriate arithmetic for T
      }

- Additionally generic in their own parameters:

      fun (v: Vec2[T]) cast_to[U]() Vec2[U] {
          ret Vec2[U]{ x: (v.x) :: U, y: (v.y) :: U };
      }

      fun demo_vec2() {
          val f: Vec2[f64] = Vec2[f64]{ x: 1.5, y: 2.0 };
          val i: Vec2[i64] = f.cast_to[i64]();
      }

Notes:
- When a method introduces its own type parameters, supply them as `value.method[Args](...)`.

## Qualified names and generics

You can combine module aliases with type arguments:

    use ds: mylib.ds;

    val s: ds.Stack[u64];
    val p: *ds.Pair[u8];    # pointer to a specialized generic type

This applies in both type and expression positions (e.g., typed literals: `ds.Stack[u64]{ ... }`).

## Interactions and limitations

- Type parameters are types only. They cannot be values and cannot represent non-type information.
- No default type arguments or constraints are provided in the parameter list syntax.
- Provide explicit type arguments at use sites for functions and methods that declare type parameters.
- Generics compose with other type constructors (pointers, arrays, slices, function types):

      rec Box[T] { value: T; }
      val pbox: *Box[u64];
      val arr:  [4]Box[u8];
      val slc:  []Box[f64];
      val fnp:  fun(Box[u64]) i64;

- Typed literals accept generic instantiations directly: `Name[Args]{ ... }`.
- Method dispatch uses the instantiated receiver type; call as `value.method(...)` or `value.method[Args](...)` as appropriate.

## Best practices

- Use descriptive parameter names (`Elem`, `Key`, `Val`) when it improves readability over single-letter names.
- Keep APIs minimal: prefer a small number of well-chosen type parameters.
- Use pointer receivers for mutating methods on generic aggregates to avoid unnecessary copies.
- Prefer explicit, consistent type arguments at call sites to make intent clear.

## Summary

- Introduce generics with `[TypeParams]` on functions, methods, records, and unions.
- Instantiate with concrete type arguments via `Name[TypeArgs]` in types, literals, and calls.
- Methods can rely on receiver type parameters and may declare their own parameters.
- Generics compose with other types and features uniformly across the language.

For related details, see:
- [Functions and Methods](functions-and-methods.md)
- [Records and Unions](records-and-unions.md)
- [Types](types.md)
- [Expressions and Operators](expressions-and-operators.md)