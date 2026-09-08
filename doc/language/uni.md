# `uni` — raw union

A `uni` is a collection of named fields that share the same memory. Writing
to one field overwrites whatever bytes were in the others. The compiler
does not track which field is "live"; the user is responsible for that.

## Grammar

```mach
uni NAME {
    field1: type;
    field2: type;
    ...
}

uni NAME[T] { ... }         # generic
```

## Examples

```mach
pub uni Number {
    i: i64;
    f: f64;
}

pub uni Maybe[T] {
    some: T;
    none: u8;
}
```

A `uni`'s size is the size of its largest field, plus any alignment
padding.

A `uni`'s overlapping variants must agree on secrecy: every field is either
secret (`^`) or every field is public. A mixed union would let the same storage
be read at two secrecy classifications, the aliasing leak the welded-storage
rules forbid. See [secrecy.md](secrecy.md).

## Raw unions versus tagged values

A `uni` is unchecked, raw memory. The compiler tracks neither which variant was
written nor whether reading a variant is valid.

For safe, discriminated values where the active case is tracked and payloads
require proof before access, use `tag`. See [tag.md](tag.md).

Low-level systems code can still compose `rec` and `uni` manually when modeling
foreign data structures, hardware registers, or wire formats:

```mach
rec RawPacket {
    kind: u8;
    data: uni { header: Header; raw: [64]u8; }
}
```

With manual composition, the compiler does not verify that `kind` and `data`
agree. Keeping them consistent is the programmer's responsibility. In ordinary
Mach code, prefer first-class `tag` declarations.

## See also

- [tag.md](tag.md) - checked tagged values with proof-guarded payload access
- [rec.md](rec.md) - records and sequential aggregate layout
- [statements.md](statements.md) - if/or chains for branching
- [secrecy.md](secrecy.md) - secrecy agreement across overlapping variants
