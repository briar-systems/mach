[mach](../README.md) / [manifest](README.md) / **load**

# mach.manifest.load

reading a manifest: its TOML held to the schema, decoded into the model and checked

one rule holds for every key whoever reads the manifest, so a dependency's manifest is
held to exactly the rules of the project being built. a refusal points at the key or
value it concerns.

Source: [`src/manifest/load.mach`](../../../src/manifest/load.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Doc`](#doc) | `rec` | a manifest's text, the file it was read from, its slot in the source map its reader holds, and its TOML tree, owned through the allocator that made it and released with `doc_dnit` |

| On `Doc` | |
| :--- | :--- |
| [`parse`](#parse) | build a `Manifest` from a parsed TOML document, in three stages |
| [`doc_parse`](#doc_parse) | `text`, the manifest at `path`, as a doc whose text is loaded into `sources`: bytes a manifest refuses and a document that is not TOML are failures pointing at the byte refused |
| [`doc_parse_bytes`](#doc_parse_bytes) | `doc_parse` over the `n` bytes at `data`, which need not end in a NUL and may hold one, which is refused where it is written |
| [`doc_read`](#doc_read) | the manifest file at `path` read and parsed as `doc_parse` parses it; a file that cannot be read is refused as a path the project names |
| [`doc_dnit`](#doc_dnit) | release a doc and everything it owns |

| Functions | |
| :--- | :--- |
| [`project_parse`](#project_parse) | the `[project]` table of a parsed TOML document alone, held to its rows as `parse` holds it, for a reader such as `mach fmt` that needs nothing else |
| [`source_load`](#source_load) | the `n` bytes at `data`, the file at `path`, loaded into `sources` so a refusal can point into them; a NUL ends the text the map holds, and a refusal at it points at its end |
| [`parse_text`](#parse_text) | the manifest `text`, read from `path`, parsed and checked |
| [`read_at`](#read_at) | the manifest of the project rooted at `dir`, read off disk and parsed |
| [`check`](#check) | the checks every manifest model passes, whether decoded from mach.toml or made by an entry build: the rules that span its entries and values, then the host target it builds for where it declares none |

| Constants | |
| :--- | :--- |
| [`MANIFEST_FILE`](#constants) | the file name of a project manifest, looked up in a project root |

---

## Doc

```mach
pub rec Doc {
    path:  str;
    text:  str;
    file:  mach.base.source.FileId;
    table: toml.value.Table;
}
```

a manifest's text, the file it was read from, its slot in the source map its
reader holds, and its TOML tree, owned through the allocator that made it and
released with `doc_dnit`. a failure that names a key points into `file`

### parse

```mach
pub fun parse(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, doc: *Doc) res[Manifest, mach.base.fail.Fail]
```

build a `Manifest` from a parsed TOML document, in three stages. the schema
check holds the document to the rows of `mach.manifest.schema`: a key no
row names, a value of the wrong shape, a missing required key and a removed key
are refused where they are written. decoding then reads each table into the
model and refuses a value its key does not accept. validation last checks the
rules that span entries and values: `need` entries and their cycles, `link`
names, at most one `default = true` profile, and every template against what
its place admits. one rule holds for every key whoever reads the manifest, so
a dependency's manifest is held to exactly the rules of the project being
built. on any error every array allocated so far is freed

| Parameter | |
| :--- | :--- |
| `alloc` | owns the manifest's arrays |
| `itn` | receives every string of the manifest |
| `doc` | the manifest's text and its TOML document |
| **returns** | the manifest, or the first error as a "mach.toml: ..." message pointing at the key or value in `doc` it concerns. the manifest records where each field a later refusal names is written, in `doc`'s file |

### doc_parse

```mach
pub fun doc_parse(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, path: str, text: str) res[Doc, mach.base.fail.Fail]
```

`text`, the manifest at `path`, as a doc whose text is loaded into `sources`:
bytes a manifest refuses and a document that is not TOML are failures pointing
at the byte refused. the doc keeps its own copies of both

### doc_parse_bytes

```mach
pub fun doc_parse_bytes(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, path: str, data: *u8, n: usize) res[Doc, mach.base.fail.Fail]
```

`doc_parse` over the `n` bytes at `data`, which need not end in a NUL and may
hold one, which is refused where it is written

### doc_read

```mach
pub fun doc_read(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, path: str) res[Doc, mach.base.fail.Fail]
```

the manifest file at `path` read and parsed as `doc_parse` parses it; a file
that cannot be read is refused as a path the project names

### doc_dnit

```mach
pub fun doc_dnit(alloc: *std.allocator.Allocator, d: *Doc)
```

release a doc and everything it owns

## Functions

### project_parse

```mach
pub fun project_parse(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, t: *toml.value.Table, m: *Manifest) err[mach.base.fail.Fail]
```

the `[project]` table of a parsed TOML document alone, held to its rows as
`parse` holds it, for a reader such as `mach fmt` that needs nothing else

| Parameter | |
| :--- | :--- |
| `alloc` | owns the refusal's text |
| `itn` | receives the table's strings |
| `t` | the document root |
| `m` | receives the project fields |
| **returns** | ok, or the first refusal |

### source_load

```mach
pub fun source_load(alloc: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, path: str, data: *u8, n: usize) res[mach.base.source.FileId, mach.base.fail.Fail]
```

the `n` bytes at `data`, the file at `path`, loaded into `sources` so a refusal can point
into them; a NUL ends the text the map holds, and a refusal at it points at its end

### parse_text

```mach
pub fun parse_text(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, sources: *mach.base.source.SourceMap, path: str, text: str) res[Manifest, mach.base.fail.Fail]
```

the manifest `text`, read from `path`, parsed and checked

| Parameter | |
| :--- | :--- |
| `alloc` | owns the manifest, released with `dnit`, and a refusal's text |
| `itn` | interns its names |
| `sources` | receives the text, which a refusal points into |
| **returns** | the manifest; err as the text does not parse or fails a check |

### read_at

```mach
pub fun read_at(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, sources: *mach.base.source.SourceMap, dir: str) res[Manifest, mach.base.fail.Fail]
```

the manifest of the project rooted at `dir`, read off disk and parsed

| Parameter | |
| :--- | :--- |
| `alloc` | owns the manifest, released with `dnit`, and a refusal's text |
| `itn` | interns its names |
| `sources` | receives the text, which a refusal points into |
| `dir` | the project root, which holds MANIFEST_FILE |
| **returns** | the manifest; err from the environment when it cannot be read, else as it does not parse or fails a check |

### check

```mach
pub fun check(alloc: *std.allocator.Allocator, itn: *mach.base.intern.Interner, m: *Manifest) err[mach.base.fail.Fail]
```

the checks every manifest model passes, whether decoded from mach.toml or made by an entry
build: the rules that span its entries and values, then the host target it builds for where
it declares none

| Parameter | |
| :--- | :--- |
| `alloc` | owns the model's arrays and a refusal's text |
| `itn` | holds the model's strings |
| `m` | the model, which gains its native target where it declares none |
| **returns** | ok, or the first refusal |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MANIFEST_FILE` | `str` | `"mach.toml"` | the file name of a project manifest, looked up in a project root |

