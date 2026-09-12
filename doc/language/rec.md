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

```mach accept
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

```mach run "1 6"
use std.runtime;
use print: std.print;

rec Point { x: i64; y: i64; }
rec Pair[T, U] { left: T; right: U; }

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val p: Point          = Point{ x: 1, y: 2 };
    val q: Pair[i64, u8]  = Pair[i64, u8]{ left: 5, right: 6u8 };
    val n: i64            = p.x;            # field access via .
    print.printlnf("{} {}", n, q.right);
    ret 0;
}
```

## Layout

By default the compiler may insert padding between fields for alignment. The
`#[align(N)]` decorator on a record raises its minimum type alignment to `N`
bytes (a power of two), and `#[packed]` lays the record out with no padding
at all, for a shape whose layout is fixed elsewhere (a C struct, a file
header, a wire frame); see [decorators.md](decorators.md#packed--no-padding)
for what a packed record refuses.

## See also

- [tag.md](tag.md) - tagged value
- [uni.md](uni.md) - overlapping-memory counterpart
- [decorators.md](decorators.md) - #[align] and #[packed]
- [comptime-intrinsics.md](comptime-intrinsics.md) - $size_of, $offset_of
