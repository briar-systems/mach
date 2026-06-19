# `rec` — record

A `rec` is a named, structurally-laid-out collection of typed fields. Each
field has its own storage; the record's size is the sum of field sizes
plus any padding for alignment.

## Grammar

```mach
rec NAME {
    field1: type;
    field2: type;
    ...
}

rec NAME[T, U] { ... }      # generic over type parameters
```

## Examples

```mach
pub rec Point {
    x: i64;
    y: i64;
}

pub rec Pair[T, U] {
    left:  T;
    right: U;
}
```

## Construction and access

A record literal names the type and provides each field by name:

```mach
val p: Point          = Point{ x: 1, y: 2 };
val q: Pair[i64, u8]  = Pair[i64, u8]{ left: 5, right: 6u8 };
val n: i64            = p.x;            # field access via .
```

## Layout

By default the compiler may insert padding between fields for alignment. The
`` `align(N)` `` backtick decorator on a record raises its minimum type
alignment to `N` bytes (a power of two); see [decorators.md](decorators.md).

> Disabling padding entirely (a `packed` layout for binary-protocol / on-disk
> structs) is not yet available — the field-to-field padding always follows the
> natural C-style rule. The legacy `$NAME.align = N;` / `$NAME.packed = true;`
> attribute setters were removed in v2.0.0.

## See also

- [uni.md](uni.md) — overlapping-memory counterpart
- [comptime-attrs.md](comptime-attrs.md) — `.align` and `.packed`
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$offset_of`
