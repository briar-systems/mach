# `fwd` — re-exports

`fwd` re-exports a symbol from another module under this module's public
surface. It is the public counterpart to `use`.

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

## When to use

- Composing a public surface from split implementation files.
- Aliasing platform-specific impls under a stable name for consumers.

## See also

- [use.md](use.md) — the private-import counterpart
- [modules.md](modules.md) — shadow-module pattern
