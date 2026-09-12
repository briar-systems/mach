# `fwd` — re-exports

`fwd` re-exports a symbol or module from another module under this
module's public surface. It is the public counterpart to `use`.

## Grammar

```mach
fwd PATH;                   # re-export under the path leaf
fwd ALIAS: PATH;            # re-export with rename
```

- `fwd` always publishes; there is no `pub fwd` form.
- One name per line. No splat. Mirrors `use` grammar exactly.

## Examples

```mach
use impl: full.core.data;

fwd impl.Point;             # re-exports as 'Point'
fwd Pt: impl.Point;         # re-exports as 'Pt'
```

The surface file in a shadow-module pattern is typically a long list of
`fwd` lines — one per symbol the surface exposes.

## Module re-exports

A `fwd` path that ends at a **module** re-exports the whole module as a
public module alias, mirroring `use`'s module binding:

```mach
fwd demo.alpha;             # re-exports module 'alpha'
fwd deep: demo.deep.beta;   # re-exports module under 'deep'
```

A consumer reaches the alias's members with qualified access, chaining
through any depth of re-export — including a `fwd` of another library's
`fwd`. In a project whose `[project] id` is `example`:

```mach run "42"
# file: src/alpha.mach
pub fun answer() i64 { ret 42; }

# file: src/lib.mach
fwd example.alpha;          # re-exports module 'alpha'

# file: src/root.mach
use std.runtime;
use print: std.print;
use example.lib;            # lib.mach contains `fwd example.alpha;`

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", lib.alpha.answer());   # resolves through the module re-export
    ret 0;
}
```

As with `use`, a module alias is not a value; only its members can be
named.

## When to use

- Composing a public surface from split implementation files.
- Aliasing platform-specific impls under a stable name for consumers.

## See also

- [use.md](use.md) — the private-import counterpart
- [modules.md](modules.md) — shadow-module pattern
