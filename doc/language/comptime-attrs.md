# Symbol attributes

Symbol attributes are expressed as **decorators** — leading, per-declaration
codegen directives on the declared symbol. A decorator is written as an
attribute: `#[name]` for a bare flag or `#[name(args)]` for a directive with
arguments. See [decorators.md](decorators.md) for the full reference.

```mach
#[symbol("main")]
fun entry(argc: i64, argv: **u8) i64 { ... }

#[library("ws2_32.dll")] #[symbol("WSAStartup")]
ext fun wsa_startup(ver: u16, data: *u8) i32;

#[align(64)]
pub var cache_line: u8 = 0;
```

A backtick form (`` `name(args)` ``) existed through v2.3.0 and was removed in
v2.4.0; a backtick at decorator position is now a migration error. The earlier
`$sym.attr = value;` setter form was removed in v2.0.0 in favor of decorators; a
stray `=` after a comptime directive is also a parse error.

## See also

- [decorators.md](decorators.md) — full decorator reference
- [ext-fun.md](ext-fun.md) — `ext` imports and `library` / `symbol` use cases
