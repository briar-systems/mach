# mach.lang.manifest.load

## fun parse

```mach
pub fun parse(alloc: *A.Allocator, itn: *intern.Interner, doc: *Doc) res[Manifest, fail.Fail];
```

build a `Manifest` from a parsed TOML document, in three stages. the schema
check holds the document to the rows of `mach.lang.manifest.schema`: a key no
row names, a value of the wrong shape, a missing required key and a removed key
are refused where they are written. decoding then reads each table into the
model and refuses a value its key does not accept. validation last checks the
rules that span entries: `need` entries and their cycles, `link` names, and at
most one `default = true` profile. one rule holds for every key whoever reads
the manifest, so a dependency's manifest is held to exactly the rules of the
project being built. on any error every array allocated so far is freed

alloc: owns the manifest's arrays
itn: receives every string of the manifest
doc: the manifest's text and its TOML document
ret: the manifest, or the first error as a "mach.toml: ..." message pointing at
       the key or value in `doc` it concerns. the manifest records where each
       field a later refusal names is written, resolved in `doc`

## fun project_parse

```mach
pub fun project_parse(alloc: *A.Allocator, itn: *intern.Interner, t: *toml.Table, m: *Manifest) err[fail.Fail];
```

the `[project]` table of a parsed TOML document alone, held to its rows as
`parse` holds it, for a reader such as `mach fmt` that needs nothing else

alloc: owns the refusal's text
itn: receives the table's strings
t: the document root
m: receives the project fields
ret: ok, or the first refusal

## rec Doc

```mach
pub rec Doc;
```

a manifest's text, the file it was read from, and its TOML tree, all owned
through the allocator that made it and released with `doc_dnit`. a failure
that names a key points into `text` at `path`

## fun doc_parse

```mach
pub fun doc_parse(alloc: *A.Allocator, path: str, text: str) res[Doc, fail.Fail];
```

`text`, the manifest at `path`, as a doc: bytes a manifest refuses and a
document that is not TOML are failures pointing at the byte refused. the doc
keeps its own copies of both

## fun doc_read

```mach
pub fun doc_read(alloc: *A.Allocator, path: str) res[Doc, fail.Fail];
```

the manifest file at `path` read and parsed as `doc_parse` parses it; a file
that cannot be read is the environment's

## fun doc_dnit

```mach
pub fun doc_dnit(alloc: *A.Allocator, d: *Doc);
```

release a doc and everything it owns

