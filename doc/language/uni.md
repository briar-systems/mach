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

## Discriminated values — by convention

Mach has no tagged-union construct and no pattern-matching dispatch.
Discriminated values compose from `rec` and `uni`:

```mach
rec Value {
    kind: ValueKind;
    data: uni { i: i64; f: f64; }
}
```

Consumers read `kind`, then access the matching `data` field via ordinary
`if`/`or` chains. The compiler does not verify that `kind` and `data`
agree; keeping them consistent is the constructor's responsibility.

This is deliberate. Tagged unions and `match`-style dispatch would add
significant surface area to the language and compiler for an abstraction
the composed form already expresses honestly.

## See also

- [rec.md](rec.md) — the discriminator-and-payload outer shape
- [statements.md](statements.md) — `if`/`or` chains for discrimination
- [secrecy.md](secrecy.md) — why overlapping variants must agree on secrecy
