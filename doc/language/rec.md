# `rec` — record

A `rec` is a named, structurally-laid-out collection of typed fields. Each
field has its own storage; the record's size is the sum of field sizes
plus any padding for alignment.

## Grammar

```mach fragment
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

A literal may leave fields out. **Every omitted field is zero-initialized**:
integers and floats read `0`, pointers read `nil`, and a nested record or array
is zero in every byte. This holds for secret (`^`) fields and secret-welded
pointers (`*^T`) as well, for a literal built at runtime from non-constant
values, for one built from constants inside a function, and for a constant
literal that initializes a module-level `val`. `T{}` names no field and so is
all zero. The guarantee is a contract, not an accident of the stack: a literal
never exposes what the storage held before.

```mach
rec Key { id: u64; material: ^[32]u8; buf: *^u8; }

fun fresh(id: u64) Key {
    ret Key{ id: id };                # material is all zero, buf is nil
}

val EMPTY: Key = Key{ id: 0 };        # the same holds for a constant literal
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
