# `use` — imports

`use` brings an external symbol or module into the current scope under a
local name. It is a private import; the imported name is not exposed to
consumers of this module.

## Grammar

```mach
use PATH;                   # binds the leaf component
use ALIAS: PATH;            # binds ALIAS
```

- The alias defaults to the path's leaf component when not given.
- One name per line. No splat (`use foo.*` does not exist) and no
  combined forms (`use foo.{a, b, c}` does not exist).

The resolver binds whatever the path points to. A path ending at a **module**
binds the module — access its members with qualified `module.member`. A path
ending at a **symbol** binds the symbol for bare use. Importing a module does
**not** pull its members into scope unqualified; to use `usize` bare, import
the symbol, not its module.

## Examples

```mach
use std.types.size;             # binds module 'size'; use as `size.usize`
use sz: std.types.size;         # binds module under 'sz'; use as `sz.usize`
use std.types.size.usize;       # binds symbol 'usize'; use bare as `usize`
use my_usize: std.types.size.usize;  # binds symbol under 'my_usize'
```

## Design rule

A Mach module imports every dependency it directly names — including any
dependency reached only through a re-export. There is no shortcut for
"my surface uses these transitively; just give me the leaf." The dependency
graph is visible at the top of every file.

## See also

- [fwd.md](fwd.md) — the public-re-export counterpart
- [modules.md](modules.md) — how paths form module identifiers
