# Comptime channel

The `$` prefix opens the **bidirectional channel** between the developer
and the compiler. Reads observe compiler-provided state; writes annotate
declarations the developer made. Everything that touches comptime in
Mach — conditional compilation, attributes, intrinsics, target queries —
uses one of the shapes on this channel.

## The shapes

| Shape | Meaning | Direction |
|---|---|---|
| `$mach.*` / `$project.*` / `$target.*` / `$bin.*` | Rooted compiler-owned read | compiler → developer |
| `$sym.attr = value` / `$sym.attr` | Attribute on declared symbol | bidirectional |
| `$sym(args)` | Comptime function call (intrinsic) | call |
| `$if`, `$or` | Comptime control flow | structural |

The parser distinguishes these by structure:

- `$<root>.<path>`, where `<root>` is one of the reserved roots `mach`,
  `project`, `target`, `bin` — a read into a compiler-owned tree. The roots
  are reserved at the top of `$`; user symbols cannot collide with them.
- `$ident.attr = ...` — write to an attribute slot on `ident`, which must
  be a symbol declared in this module.
- `$ident(args)` — comptime call; the closed compiler-intrinsic set lives
  here.
- `$if` / `$or` — comptime branches, structurally distinct from runtime
  `if` / `or`.

## Compiler-owned roots

| Root | Reads | Source |
|---|---|---|
| `$mach.*` | target/os/arch tags, pointer width, compiler identity | active target + compiler |
| `$project.{id,version,name,description}` | project metadata | `[project]` in `mach.toml` |
| `$target.{os,isa,abi}` | the selected target's declared tuple, as strings | the selected `mach.toml` target |
| `$bin.name` | the artifact being built | the selected build unit (`[bin.*]` / `[lib.*]`) |

`$target.*` carries the manifest's declared **string** spellings (`"linux"`,
`"x86_64"`, `"sysv64"`), distinct from `$mach.target.*`'s numeric tags used for
`$mach.os.*` comparison — both keep working. A root field the manifest does not
declare (`$project.description` on a v1 manifest, `$bin.name` on a v1 manifest
with no artifact stanza) reports the read as unavailable rather than folding to
an empty string. See [comptime-mach.md](comptime-mach.md) for the `$mach.*`
subtree.

```mach
val ver: str = $project.version;            # "1.3.0", from [project].version
$if ($target.os == "windows") { ... }       # the declared os string
```

## The bare `$ident` is rejected

A standalone bare `$ident` — `$mode`, `$foo` — is **none** of the shapes above
and is rejected with one teaching diagnostic, owned by the comptime evaluator:

> comptime parameters are referenced without `$`; comptime paths are rooted:
> `` $mach ``, `` $project ``, `` $target ``, `` $bin ``

A comptime **parameter** is referenced by its bare name (no `$`); every comptime
**path** is rooted. The rule applies identically in a `$if` gate and in value
position — sema and lowering both defer to the evaluator's single verdict rather
than each carrying their own.

## What's not in the channel

- No reflection-via-`$<Type>.*` subtree. Types are not first-class
  comptime values.
- No decl-attached prefix sugar (`$inline pub fun ...` does not exist) —
  use attribute writes.
- No comptime function definitions, no comptime loops.
- No bare `$ident` — see above.

## See also

- [comptime-mach.md](comptime-mach.md) — the `$mach.*` namespace
- [comptime-attrs.md](comptime-attrs.md) — symbol attributes
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of`,
  `$assert`, …
- [comptime-control.md](comptime-control.md) — `$if` / `$or`
