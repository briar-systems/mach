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
rules that span entries and values: `need` entries and their cycles, `link`
names, at most one `default = true` profile, and every template against what
its place admits. one rule holds for every key whoever reads the manifest, so
a dependency's manifest is held to exactly the rules of the project being
built. on any error every array allocated so far is freed

alloc: owns the manifest's arrays
itn: receives every string of the manifest
doc: the manifest's text and its TOML document
ret: the manifest, or the first error as a "mach.toml: ..." message pointing at
       the key or value in `doc` it concerns. the manifest records where each
       field a later refusal names is written, in `doc`'s file

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

a manifest's text, the file it was read from, its slot in the source map its
reader holds, and its TOML tree, owned through the allocator that made it and
released with `doc_dnit`. a failure that names a key points into `file`

## fun doc_parse

```mach
pub fun doc_parse(alloc: *A.Allocator, sources: *lang_source.SourceMap, path: str, text: str) res[Doc, fail.Fail];
```

`text`, the manifest at `path`, as a doc whose text is loaded into `sources`:
bytes a manifest refuses and a document that is not TOML are failures pointing
at the byte refused. the doc keeps its own copies of both

## fun doc_parse_bytes

```mach
pub fun doc_parse_bytes(alloc: *A.Allocator, sources: *lang_source.SourceMap, path: str, data: *u8, n: usize) res[Doc, fail.Fail];
```

`doc_parse` over the `n` bytes at `data`, which need not end in a NUL and may
hold one, which is refused where it is written

## fun source_load

```mach
pub fun source_load(alloc: *A.Allocator, sources: *lang_source.SourceMap, path: str, data: *u8, n: usize) res[lang_source.FileId, fail.Fail];
```

the `n` bytes at `data`, the file at `path`, loaded into `sources` so a refusal can point
into them; a NUL ends the text the map holds, and a refusal at it points at its end

## fun doc_read

```mach
pub fun doc_read(alloc: *A.Allocator, sources: *lang_source.SourceMap, path: str) res[Doc, fail.Fail];
```

the manifest file at `path` read and parsed as `doc_parse` parses it; a file
that cannot be read is the environment's

## val MANIFEST_FILE

```mach
pub val MANIFEST_FILE: str = "mach.toml"
```

the file name of a project manifest, looked up in a project root

## fun parse_text

```mach
pub fun parse_text(alloc: *A.Allocator, itn: *intern.Interner, sources: *lang_source.SourceMap, path: str, text: str) res[Manifest, fail.Fail];
```

the manifest `text`, read from `path`, parsed and checked

alloc: owns the manifest, released with `dnit`, and a refusal's text
itn: interns its names
sources: receives the text, which a refusal points into
ret: the manifest; err as the text does not parse or fails a check

## fun read_at

```mach
pub fun read_at(alloc: *A.Allocator, itn: *intern.Interner, sources: *lang_source.SourceMap, dir: str) res[Manifest, fail.Fail];
```

the manifest of the project rooted at `dir`, read off disk and parsed

alloc: owns the manifest, released with `dnit`, and a refusal's text
itn: interns its names
sources: receives the text, which a refusal points into
dir: the project root, which holds MANIFEST_FILE
ret: the manifest; err from the environment when it cannot be read, else as
         it does not parse or fails a check

## fun doc_dnit

```mach
pub fun doc_dnit(alloc: *A.Allocator, d: *Doc);
```

release a doc and everything it owns

## fun check

```mach
pub fun check(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail];
```

the checks every manifest model passes, whether decoded from mach.toml or made by an entry
build: the rules that span its entries and values, then the host target it builds for where
it declares none

alloc: owns the model's arrays and a refusal's text
itn: holds the model's strings
m: the model, which gains its native target where it declares none
ret: ok, or the first refusal

