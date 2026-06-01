# Comptime channel

The `$` prefix opens the **bidirectional channel** between the developer
and the compiler. Reads observe compiler-provided state; writes annotate
declarations the developer made. Everything that touches comptime in
Mach — conditional compilation, attributes, intrinsics, target queries —
uses one of four shapes on this channel.

## The four shapes

| Shape | Meaning | Direction |
|---|---|---|
| `$mach.path.to.value` | Compiler-owned value | compiler → developer |
| `$sym.attr = value` / `$sym.attr` | Attribute on declared symbol | bidirectional |
| `$sym(args)` | Comptime function call (intrinsic) | call |
| `$if`, `$or` | Comptime control flow | structural |

The parser distinguishes these by structure:

- `$mach.<anything>` — always a read into the compiler-owned tree.
  `mach` is reserved at the top of `$`; user symbols cannot collide here.
- `$ident.attr = ...` — write to an attribute slot on `ident`, which must
  be a symbol declared in this module.
- `$ident(args)` — comptime call; the closed compiler-intrinsic set lives
  here.
- `$if` / `$or` — comptime branches, structurally distinct from runtime
  `if` / `or`.

## What's not in the channel

- No reflection-via-`$<Type>.*` subtree. Types are not first-class
  comptime values.
- No decl-attached prefix sugar (`$inline pub fun ...` does not exist) —
  use attribute writes.
- No comptime function definitions, no comptime loops.

## See also

- [comptime-mach.md](comptime-mach.md) — the `$mach.*` namespace
- [comptime-attrs.md](comptime-attrs.md) — symbol attributes
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$assert`, …
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
