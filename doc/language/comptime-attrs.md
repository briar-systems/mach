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

A backtick is not a token: one anywhere in source is a lexer error. A comptime
directive takes no `=`; a stray one is a parse error at the directive's
terminator.

## See also

- [decorators.md](decorators.md) — full decorator reference
- [ext-fun.md](ext-fun.md) — `ext` imports and `library` / `symbol` use cases
