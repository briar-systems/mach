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

| Attribute | Applies to | Value | Purpose | Status |
|---|---|---|---|---|
| `.symbol` | functions, vars | `*u8` literal | Linker name override | honored |
| `.library` | `ext` functions | `*u8` literal | Pin a dynamic import to a dependency DLL (PE) | honored |
| `.noreturn` | functions | `u8` flag | Function never returns | not yet honored |
| `.inline` | functions | `u8` flag | Strong hint to inline at call sites | not yet honored |
| `.align` | vars, records, unions | power-of-two int | Storage / type alignment in bytes | not yet honored |
| `.section` | functions, vars | `*u8` literal | Linker section name | not yet honored |
| `.packed` | records, unions | `u8` flag | Disable field padding | not yet honored |

The set is closed. New attributes require a compiler change.

> **Implementation status.** Any well-formed `$sym.attr = value;` write
> parses, but only `.symbol` and `.library` currently change compiler output
> (the registered linker name and the PE import descriptor an import is routed
> to). The remaining attributes are accepted and ignored — record/union layout
> uses the natural C-style rule regardless of `.align` / `.packed`, and
> `.noreturn` / `.inline` / `.section` do not yet feed codegen.

Flag attributes take a `u8` (`1` on, `0` off). The stdlib constants `true` /
`false` (`def bool: u8;` with `true`/`false` = `1`/`0`) are the idiomatic
spelling when stdlib is in scope; `$panic.noreturn = true;` and
`$panic.noreturn = 1;` are equivalent.

## See also

- [comptime.md](comptime.md) — channel overview
- [ext-fun.md](ext-fun.md) — common `.symbol` and `.library` use cases
