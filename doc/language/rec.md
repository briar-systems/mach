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

By default the compiler may insert padding between fields for alignment.
Two attributes are intended to adjust this:

- `$NAME.align = N;` — minimum type alignment in bytes (power of two).
- `$NAME.packed = true;` — disable padding entirely (binary protocol /
  on-disk layouts).

> **Not yet honored.** Both attributes parse but do not currently affect
> layout — the compiler always uses the natural C-style rule. See the
> status note in [comptime-attrs.md](comptime-attrs.md).

## See also

- [uni.md](uni.md) — overlapping-memory counterpart
- [comptime-attrs.md](comptime-attrs.md) — `.align` and `.packed`
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$offset_of`
