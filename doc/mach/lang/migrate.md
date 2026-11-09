# mach.lang.migrate

a project written for the previous major, rewritten into this one's spelling,
which `mach migrate` applies

every rule is data its owner already holds: a manifest key a major removed or
retired is a row of the manifest schema with a rewrite, a template variable's
previous name is a row of the template's renamed table, a comptime path's is a
row of the comptime path registry's, a decorator's is a row of the decorator
registry's, and an `asm` head's and a truth alias's are rows of the transition
table's. so a name a later major retires is migrated by the row that retires
it, and this module names none of them

the rules are rows themselves, each tagged with the major it migrates from. a
release carries the rows of the major before it: 8.0.0 deletes every row tagged
6 and adds its own

## val FROM_MAJOR

```mach
pub val FROM_MAJOR: u32 = 6
```

the major this release migrates from

## rec Dependency

```mach
pub rec Dependency;
```

a dependency of the root project, as `mach migrate` reads it

id: its key under `[dep]`
range: its `[project].mach`, nil when dep/ holds no manifest for it
module: the module its exported artifact's entry is, nil when it exports none

## fun manifest

```mach
pub fun manifest(a: *A.Allocator, text: str) res[str, fail.Fail];
```

a manifest's text in this major's spelling: each key the schema retires with
a rewrite rewritten, and each template variable a previous major named
renamed wherever a string writes it. a text that is not TOML is refused, and
one already in this spelling comes back unchanged

a: owns the returned text
text: the manifest's text
ret: the rewritten text, owned by `a`

## fun on_previous_major

```mach
pub fun on_previous_major(range: str) bool;
```

whether a dependency's `[project].mach` range stands on the major this release migrates

## fun dependencies

```mach
pub fun dependencies(a: *A.Allocator, sources: *lang_source.SourceMap, root: str, text: str, out: *Vector[Dependency]) err[fail.Fail];
```

each dependency of the manifest `text`, whose project is at `root`, as dep/ holds it

a: owns what `out` holds
sources: receives the text of each dependency's manifest dep/ holds
root: the project's directory
text: its manifest's text
out: receives the dependencies

## fun word_in

```mach
pub fun word_in(text: str, at: usize, end: usize, word: str) usize;
```

the offset of `word` standing as a word in `text` from `at` to `end`, `end` when it does
not: after a quote, a space or a path's `/`, and before a quote, a space or an `=`

## fun source

```mach
pub fun source(a: *A.Allocator, text: str, deps: *Vector[Dependency]) res[str, fail.Fail];
```

a source's text in this major's spelling: each comptime path whose leading
segments a previous major named, each decorator it named, each `asm` head, each
truth alias over a width and each bare use of a project rewritten. only tokens
are read, so a comment or a string that spells one is left as written

a: owns the returned text
text: the source's text
deps: the root project's dependencies
ret: the rewritten text, owned by `a`, or the lexer's refusal

