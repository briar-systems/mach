# Symbol attributes

Symbol attributes are expressed as **decorators** — leading, per-declaration
codegen directives on the declared symbol. A decorator is written in either of
two interchangeable surfaces: the going-forward **attribute** form `#[name]` or
the original **backtick** form `` `name` ``. Both produce the same directive;
prefer `#[...]` in new code. See [decorators.md](decorators.md) for the full
reference.

```mach
#[symbol("main")]
fun entry(argc: i64, argv: **u8) i64 { ... }

#[library("ws2_32.dll")] #[symbol("WSAStartup")]
ext fun wsa_startup(ver: u16, data: *u8) i32;

#[align(64)]
pub var cache_line: u8 = 0;
```

The backtick form remains accepted this phase, so the two spellings may be
mixed; the compiler's own source still uses backticks until the migration
completes.

```mach
`symbol("main")`
fun entry(argc: i64, argv: **u8) i64 { ... }
```

The previous `$sym.attr = value;` setter form was removed in v2.0.0 in favor of
decorators; a stray `=` after a comptime directive is now a parse error.

## See also

- [decorators.md](decorators.md) — full decorator reference
- [ext-fun.md](ext-fun.md) — `ext` imports and `library` / `symbol` use cases
