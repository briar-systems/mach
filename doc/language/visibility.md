# Visibility — `pub` and `ext`

Two declaration modifiers control how a symbol is seen.

## `pub`

Marks a declaration as part of its module's public surface. Other modules
that `use` this module can reference `pub`-marked symbols by name; symbols
without `pub` are file-private.

```mach
pub fun add(a: i64, b: i64) i64 { ret a + b; }
fun helper() { ... }            # private: only callable inside this file
pub rec Point { x: i64; y: i64; }
pub val MAX: i64 = 100;
```

Applies to: `fun`, `rec`, `uni`, `def`, `val`, `var`, `ext fun`.

`fwd` always publishes and does not take an explicit `pub` modifier.

## `ext`

Declares a function with C ABI as a forward reference — body-less. The
linker resolves the symbol at link time. Only functions can be `ext`.

```mach
$libc_write.symbol = "write";
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

- `ext fun` declarations have no body.
- The C ABI is the contract; argument and return types must be
  representable in C.
- Use `$NAME.symbol = "real_name";` to override the linker name.

There are no body-less functions outside of `ext fun`. Regular forward
declarations do not exist.

## See also

- [fun.md](fun.md) — regular function declarations
- [ext-fun.md](ext-fun.md) — full reference for `ext fun`
- [comptime-attrs.md](comptime-attrs.md) — `.symbol` and other attributes
