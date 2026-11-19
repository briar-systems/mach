[mach](../README.md) / [tool](README.md) / **migrate**

# mach.tool.migrate

a project written for the previous major, rewritten into this one's spelling,
which `mach migrate` applies

a renamed spelling is data its owner already holds: a manifest key a major
removed or retired is a row of the manifest schema with a rewrite, a target
value's retired spelling is a row of the target reader's, a template variable's
previous name is a row of the template's renamed table, a comptime path's is a
row of the comptime path registry's, a decorator's is a row of the decorator
registry's, and an `asm` head's, a truth alias's and a step option's are rows
of the transition table's. so a name a later major retires is migrated by the
row that retires it, and no rule here spells one

what changed meaning rather than spelling has no owner's row, so its rule here
names what it reads: `[project].mach` for the range, a step's `argv` for which
step runs mach, `[project].out` and each artifact's `out` for the output a
previous major rooted, a freestanding target's `isa` and `of` for the machine and
the format a previous major assumed, a profile's `allow` for warnings no build
raises any more, and a dependency's `[artifact.*]` `export`, `default`, `kind` and
`entry` for the module a bare `use` bound

the rules are rows themselves, each tagged with the major it migrates from. a
release carries the rows of the major before it: 8.0.0 deletes every row tagged
6 and adds its own

Source: [`src/tool/migrate.mach`](../../../src/tool/migrate.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Dependency`](#dependency) | `rec` | a dependency of the root project, as `mach migrate` reads it |

| On `Dependency` | |
| :--- | :--- |
| [`dependencies`](#dependencies) | each dependency of the manifest `text`, whose project is at `root`, as dep/ holds it |
| [`source`](#source) | a source's text in this major's spelling: each comptime path whose leading segments a previous major named, each decorator it named, each `asm` head, each truth alias over a width and each bare use of a project rewritten |

| Functions | |
| :--- | :--- |
| [`manifest`](#manifest) | a manifest's text in this major's spelling: each key the schema retires with a rewrite rewritten, and each template variable a previous major named renamed wherever a string writes it |
| [`on_previous_major`](#on_previous_major) | whether a dependency's `[project].mach` range stands on the major this release migrates |
| [`word_in`](#word_in) | the offset of `word` standing as a word in `text` from `at` to `end`, `end` when it does not: after a quote, a space or a path's `/`, and before a quote, a space or an `=` |
| [`table_heading`](#table_heading) | the offset of the table header line above the byte `at` of a manifest's text, the line a diff names beside a change there; the text's length where none stands above it |
| [`runs_mach`](#runs_mach) | whether a step's `argv`, its literals in `text`, runs mach |

| Constants | |
| :--- | :--- |
| [`FROM_MAJOR`](#constants) | the major this release migrates from |

---

## Dependency

```mach
pub rec Dependency {
    id:     str;
    range:  str;
    module: str;
}
```

a dependency of the root project, as `mach migrate` reads it

| Field | Type | |
| :--- | :--- | :--- |
| `id` | `str` | its key under `[dep]` |
| `range` | `str` | its `[project].mach`, nil when dep/ holds no manifest for it |
| `module` | `str` | the module its exported artifact's entry is, nil when it exports none |

### dependencies

```mach
pub fun dependencies(a: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, root: str, text: str, out: *Vector[Dependency]) err[mach.base.fail.Fail]
```

each dependency of the manifest `text`, whose project is at `root`, as dep/ holds it

| Parameter | |
| :--- | :--- |
| `a` | owns what `out` holds |
| `sources` | receives the text of each dependency's manifest dep/ holds |
| `root` | the project's directory |
| `text` | its manifest's text |
| `out` | receives the dependencies |

### source

```mach
pub fun source(a: *std.allocator.Allocator, text: str, deps: *Vector[Dependency]) res[str, mach.base.fail.Fail]
```

a source's text in this major's spelling: each comptime path whose leading
segments a previous major named, each decorator it named, each `asm` head, each
truth alias over a width and each bare use of a project rewritten. only tokens
are read, so a comment or a string that spells one is left as written

| Parameter | |
| :--- | :--- |
| `a` | owns the returned text |
| `text` | the source's text |
| `deps` | the root project's dependencies |
| **returns** | the rewritten text, owned by `a`, or the lexer's refusal |

## Functions

### manifest

```mach
pub fun manifest(a: *std.allocator.Allocator, text: str) res[str, mach.base.fail.Fail]
```

a manifest's text in this major's spelling: each key the schema retires with
a rewrite rewritten, and each template variable a previous major named
renamed wherever a string writes it. a text that is not TOML is refused, and
one already in this spelling comes back unchanged

| Parameter | |
| :--- | :--- |
| `a` | owns the returned text |
| `text` | the manifest's text |
| **returns** | the rewritten text, owned by `a` |

### on_previous_major

```mach
pub fun on_previous_major(range: str) bool
```

whether a dependency's `[project].mach` range stands on the major this release migrates

### word_in

```mach
pub fun word_in(text: str, at: usize, end: usize, word: str) usize
```

the offset of `word` standing as a word in `text` from `at` to `end`, `end` when it does
not: after a quote, a space or a path's `/`, and before a quote, a space or an `=`

### table_heading

```mach
pub fun table_heading(text: str, at: usize) usize
```

the offset of the table header line above the byte `at` of a manifest's text, the line a
diff names beside a change there; the text's length where none stands above it

### runs_mach

```mach
pub fun runs_mach(text: str, args: *toml.value.Array) bool
```

whether a step's `argv`, its literals in `text`, runs mach

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FROM_MAJOR` | `u32` | `6` | the major this release migrates from |

