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
`fwd`:

```mach
use demo.lib;               # lib.mach contains `fwd demo.alpha;`

lib.alpha.answer();         # resolves through the module re-export
```

As with `use`, a module alias is not a value; only its members can be
named.

## When to use

- Composing a public surface from split implementation files.
- Aliasing platform-specific impls under a stable name for consumers.

## See also

- [use.md](use.md) — the private-import counterpart
- [modules.md](modules.md) — shadow-module pattern
