# Symbol attributes

A symbol attribute is a piece of metadata the developer attaches to a
declared symbol via a write into the comptime channel. The compiler reads
attributes during codegen to influence emission (linker names, alignment,
inlining, etc.).

## Grammar

```mach
$sym.attr = value;          # write
$sym.attr                   # read
```

- `sym` must be a symbol declared in the same module.
- `attr` must be a known attribute name (closed set).
- The path is flat: exactly one symbol component, one attribute component.
- Writes are write-once. RHS must be comptime-evaluable.

## Header-form convention

Attribute writes can appear at any point in the source, but by convention
they go **before** the declaration they target. The compiler resolves
binding at module scope; lexical order doesn't matter for correctness, but
the header placement reads as part of the decl.

```mach
$panic.noreturn = true;
pub fun panic(msg: *u8) {
    asm x86_64 { ud2 }
}
```

## Known attribute names

| Attribute | Applies to | Value | Purpose |
|---|---|---|---|
| `.symbol` | functions, vars | `*u8` literal | Linker name override |
| `.noreturn` | functions | `u8` flag | Function never returns |
| `.inline` | functions | `u8` flag | Strong hint to inline at call sites |
| `.align` | vars, records, unions | power-of-two int | Storage / type alignment in bytes |
| `.section` | functions, vars | `*u8` literal | Linker section name |
| `.packed` | records, unions | `u8` flag | Disable field padding |

The set is closed. New attributes require a compiler change.

Flag attributes take a `u8` (`1` on, `0` off). The stdlib constants `true` /
`false` (`def bool: u8;` with `true`/`false` = `1`/`0`) are the idiomatic
spelling when stdlib is in scope; `$panic.noreturn = true;` and
`$panic.noreturn = 1;` are equivalent.

## See also

- [comptime.md](comptime.md) — channel overview
- [ext-fun.md](ext-fun.md) — common `.symbol` use case
