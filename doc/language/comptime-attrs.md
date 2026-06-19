# Symbol attributes

In v1.7, symbol attributes are expressed as **backtick decorators** —
leading, per-declaration codegen directives on the declared symbol. See
[decorators.md](decorators.md) for the full reference.

```mach
`symbol("main")`
fun entry(argc: i64, argv: **u8) i64 { ... }

`library("ws2_32.dll")` `symbol("WSAStartup")`
ext fun wsa_startup(ver: u16, data: *u8) i32;

`align(64)`
pub var cache_line: u8 = 0;
```

The previous `$sym.attr = value;` setter form was removed in v2.0.0 in favor of
decorators; a stray `=` after a comptime directive is now a parse error.

## See also

- [decorators.md](decorators.md) — full decorator reference
- [ext-fun.md](ext-fun.md) — `ext` imports and `library` / `symbol` use cases
