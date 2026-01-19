# Proposal: Type Widening and Narrowing Conversions

## Status
- **Status:** Proposed
- **Date:** 2025-01-17

## Summary
This proposal outlines a design for introducing implicit type widening (promotion) and explicit type narrowing/conversion mechanisms to Mach. Currently, Mach requires explicit casting for all type mismatches, which promotes safety but can lead to verbosity. This proposal suggests a middle ground: allow safe, lossless implicit conversions while requiring explicit syntax for potentially lossy or dangerous conversions.

## Motivation
Mach's current type system is strict: a `u8` cannot be assigned to a `u64` without an explicit bit-cast (`::`). While this prevents accidental truncation, it makes working with mixed-size integers cumbersome.

Example of current friction:
```mach
val a: u8 = 10;
val b: u64 = a::u64; # Explicit bit-cast required just to extend value
```

Goals:
1.  **Ergonomics:** Reduce verbosity for safe operations (e.g., passing a `u8` to a function expecting `i64`).
2.  **Safety:** Prevent accidental data loss (truncation) or sign errors (signed/unsigned mismatch) by requiring explicit intent.
3.  **Clarity:** Make conversion sites obvious when they matter.

## Proposed Design

### 1. Implicit Widening (Safe Conversions)
The compiler should allow **implicit** conversion between primitive numeric types if and only if the conversion is guaranteed to be lossless and value-preserving.

**Allowed Implicit Conversions:**
- **Unsigned Integers:** `uN` -> `uM` where `M > N`
  - e.g., `u8` -> `u16`, `u32` -> `u64`
- **Signed Integers:** `iN` -> `iM` where `M > N`
  - e.g., `i8` -> `i16`, `i32` -> `i64`
- **Floating Point:** `f32` -> `f64`

**Disallowed Implicit Conversions:**
- **Signedness Change:** `u32` -> `i64` (even if it fits, treating mixed signs implicitly can lead to subtle bugs).
- **Narrowing:** `u64` -> `u32` (lossy).
- **Float/Int Mixing:** `i32` -> `f64` (precision loss possible for large integers, usually requires explicit intent).

### 2. Explicit Narrowing and Conversion
For conversions that may lose data (truncation) or change interpretation (signed/unsigned), explicit syntax is required.

The current bit-cast operator `::` does a raw bitwise reinterpretation. We propose formalizing or adding syntax for value conversions (e.g., numeric casts).

**Scenarios requiring explicit casts:**
- **Narrowing:** `u64` -> `u8`
- **Signedness Change:** `i32` -> `u32`
- **Float <-> Int:** `f32` -> `i32`

### 3. Syntax Options

#### Option A: Enhance `::` (Cast Operator)
Redefine `expr::Type` to perform value conversion for numeric types, and bit-cast for non-numeric/pointer types.

- `val x: u64 = 1000;`
- `val y: u8 = x::u8;` // Truncates value to 232 (1000 % 256)

*Pros:* Reuses existing syntax.
*Cons:* conflates "reinterpret bits" with "convert value".

#### Option B: New `as` Keyword (Recommended)
Introduce `as` for value conversions.

```mach
val big: u64 = 1000;
val small: u8 = big as u8;   # Explicit truncation
val s: i32 = -1;
val u: u32 = s as u32;       # Explicit sign cast
```

*Pros:* Clear distinction between value conversion (`as`) and bitwise reinterpretation (`::`).

### 4. Checked Conversions
To further enhance safety, the standard library or language could provide checked casting that fails (or returns an Option/Result) on overflow.

```mach
val x: u64 = 1000;
val y: u8 = x.try_cast[u8](); # Returns error/none because 1000 > 255
```

## Impact on Arithmetic
If implicit widening is adopted, binary operators should support operands of different sizes by widening the smaller operand to the size of the larger one, provided they share the same signedness.

```mach
val a: u8 = 10;
val b: u64 = 100;
val c = a + b; // 'a' implicitly promoted to u64, result is u64
```

## Recommendation
1.  Implement **implicit widening** for integers of the same sign (up-casting).
2.  Retain strictness for signed/unsigned mixing.
3.  Introduce `as` or clarify `::` for explicit **narrowing** and **signedness** casts.
4.  Document these rules clearly in `types.md`.