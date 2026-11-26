[mach](../README.md) / [base](README.md) / **diagnostic**

# mach.base.diagnostic

diagnostics, the builder that assembles one and the store that collects them

a store keeps its diagnostics in commit order and never holds the same report twice. it drops
the warnings an `#[expect]` covers, and the warnings their owner's governance hides or
silences, and reports an expectation nothing fulfilled. a store replays into another and encodes to bytes that name files by path, so
another session can decode it. the texts a diagnostic or failure carries are interned here
when they must outlive the scratch storage they were made in.

Source: [`src/base/diagnostic.mach`](../../../src/base/diagnostic.mach)

Uses types from [`mach.base.diagnostic.annotation`](diagnostic/annotation.md), [`mach.base.diagnostic.kind`](diagnostic/kind.md), [`mach.base.fail`](fail.md), [`mach.base.intern`](intern.md), [`mach.base.source`](source.md), [`mach.base.wire`](wire.md).

| Type | | |
| :--- | :--- | :--- |
| [`Origin`](#origin) | `def` | the phase or command that produced a diagnostic |
| [`ChildKind`](#childkind) | `def` |  |
| [`Child`](#child) | `rec` | a note or a help line attached to a diagnostic |
| [`FixId`](#fixid) | `def` | a fix's position in its diagnostic's fix list |
| [`Diagnostic`](#diagnostic) | `rec` | one diagnostic; its severity is its kind's, read with `is_error` |
| [`DiagnosticStore`](#diagnosticstore) | `rec` | the diagnostics of a build, in the order they were committed |
| [`Governance`](#governance) | `rec` | how the owner of a warning, the project being built or one of its dependencies, governs it as it reaches a store: whether the build shows the owner's warnings at all, and the kinds the owner's own `allow` silences |
| [`Expectation`](#expectation) | `rec` | one key of a declaration's `#[expect]`: a warning of a kind it selects, located in the declaration's own file between `start` and `end`, is acknowledged there and dropped |
| [`DiagnosticBuilder`](#diagnosticbuilder) | `rec` | a diagnostic being assembled: notes, help, related places and fixes attach here, and `commit` hands the whole to a store |
| [`SourceTable`](#sourcetable) | `rec` | how an encoded store names its files: by path, which outlives the file ids of the session that encoded it |

| On `Origin` | |
| :--- | :--- |
| [`ORIGIN_NONE`](#origin), [`ORIGIN_BUILD`](#origin), [`ORIGIN_LOAD`](#origin), [`ORIGIN_RESOLVE`](#origin), [`ORIGIN_SEMA`](#origin), [`ORIGIN_LOWER`](#origin), [`ORIGIN_CODEGEN`](#origin), [`ORIGIN_LINK`](#origin), [`ORIGIN_TEST`](#origin) | |
| [`origin_name`](#origin_name) | the stable name of an origin |

| On `ChildKind` | |
| :--- | :--- |
| [`CHILD_NOTE`](#childkind), [`CHILD_HELP`](#childkind) | |

| On `Diagnostic` | |
| :--- | :--- |
| [`is_error`](#is_error) | the diagnostic is an error: its kind's row declares it one |
| [`report_key`](#report_key) | what tells one report from another wherever it is held: the digest of everything it reports but its origin, so a store rebuilt from its sources gives a report it held before the same key |

| On `DiagnosticStore` | |
| :--- | :--- |
| [`store_init`](#store_init) | an empty store attributing to no origin, showing every warning of the project being built |
| [`set_origin`](#set_origin) | attribute every diagnostic committed to the store from now on whose builder names no origin to `o`; a diagnostic already committed keeps its own |
| [`govern`](#govern) | govern the warnings of the project being built that reach the store from now on by `g` |
| [`owner_add`](#owner_add) | a further owner of warnings, governed by `g`, whose files `own` hands it |
| [`own`](#own) | the warnings that stand in `file` belong to `owner`, one `owner_add` gave |
| [`expect`](#expect) | acknowledge every warning an expectation in `list` covers as it reaches the store; the store takes the list, whatever the outcome, and indexes it by file |
| [`report_unfulfilled`](#report_unfulfilled) | an expectation nothing fulfilled is itself a warning, where it is judged: a build with errors judges none, since a pass it stopped may have been the one to raise the warning |
| [`drop_file`](#drop_file) | drop every diagnostic located in one file, keeping the rest in order |
| [`drop_file_except`](#drop_file_except) | drop every diagnostic located in one file except those that begin at one of `sites`, keeping the rest in order, as drop_file does |
| [`store_dnit`](#store_dnit) | release a store and every diagnostic it holds; a nil store holds nothing and releases nothing |
| [`len`](#len) | how many diagnostics `store` holds; a nil store, as a walk that reports nowhere has, holds none |
| [`error_count`](#error_count) | how many errors `store` counts; a nil store counts none |
| [`report`](#report) | a diagnostic of kind `k` with no attachments |
| [`reportf`](#reportf) | a diagnostic of kind `k` with no attachments, its message `fmt` formatted with `args` |
| [`record`](#record) | a diagnostic of kind `k` reported where the reporter has no failure to return: one the store cannot take is counted lost, which fails the build |
| [`record_unlocated`](#record_unlocated) | a diagnostic about the build as a whole, located at no source |
| [`reject`](#reject) | a pass past the front end rejects the program: the refusal is an error located at `loc` on the store and the pass answers `reported`, exactly as a front-end pass does, so rendering, the tally and the exit status derive from one list |
| [`note_lost`](#note_lost) | count a diagnostic `store` could not take, which fails the build; the store is live, as `live` holds |
| [`lost_count`](#lost_count) | how many diagnostics `store` could not take; a nil store lost none |
| [`content_equal`](#content_equal) | compare observable diagnostic content without allocation or store identity |
| [`copy`](#copy) | an owned copy of `src` from `a`, every diagnostic and its lost count kept, released with store_dnit |
| [`replay_into`](#replay_into) | commit a copy of every diagnostic of `src` to `dst`, each where it stands, and add `src`'s lost count to `dst`'s |
| [`replay_located`](#replay_located) | replay into `dst` only the diagnostics of `src` placed somewhere, with its lost count: what a report placed nowhere waits for a requester to place does not come along |
| [`replay_unplaced`](#replay_unplaced) | replay into `dst` only the diagnostics of `src` placed nowhere (their file base_source.FILE_NIL), each reported at `at` |
| [`encode`](#encode) | write the store's diagnostics as bytes: their count, then per diagnostic its origin, its kind's key, location and message, then its notes, related places and fixes |

| On `DiagnosticBuilder` | |
| :--- | :--- |
| [`builder_init`](#builder_init) | a builder for a diagnostic of kind `k` |
| [`builder_initf`](#builder_initf) | a builder for a diagnostic of kind `k` whose message is `fmt` formatted with `args`; a format the allocator refuses is a failure, never a substitute message |
| [`builder_dnit`](#builder_dnit) | release what a builder owns; a nil builder, or one already committed or released, owns nothing |
| [`attach_note`](#attach_note) | attach a note, copied through the builder's allocator |
| [`attach_help`](#attach_help) | attach a help line, copied through the builder's allocator |
| [`attach_related`](#attach_related) | a related place; a nil or empty label shows the place without one |
| [`attach_fix`](#attach_fix) | a fix of one edit, which `attach_fix_edit` extends |
| [`attach_fix_edit`](#attach_fix_edit) | a further edit of fix `fix`, which may not overlap one it already makes |
| [`withdraw_fix`](#withdraw_fix) | drop the most recently attached fix, as a fix whose later edit could not be attached is withdrawn whole |
| [`commit`](#commit) | hand the builder's diagnostic to the store; the builder is consumed whatever the outcome |

| On `SourceTable` | |
| :--- | :--- |
| [`decode`](#decode) | read one store `encode` wrote and commit its diagnostics to `into`: false when the bytes are malformed or name a kind this compiler does not raise, which ends the read where it stands |

| Functions | |
| :--- | :--- |
| [`format`](#format) | `fmt` formatted with `va` into `a`, which owns the text; a refusal to format is the failure instead |
| [`join`](#join) | the texts `va` joined into `a`, which owns the result; a refusal to join is the failure instead |
| [`text_retain`](#text_retain) | the interner's copy of a borrowed `text`, which outlives the storage `text` came from |
| [`failure_retain`](#failure_retain) | `f` with its text the interner's copy, so it outlives the storage its text was made in, as a failure a product or a status keeps past the operation that made it does |
| [`text_intern`](#text_intern) | `text`, owned by `a`, moved into the interner: the interned copy outlives `a`'s storage, and `text` is released either way |
| [`format_intern`](#format_intern) | `fmt` formatted with `va` through `a` as scratch and interned, so the text outlives `a`'s storage |
| [`refusal_interned`](#refusal_interned) | `fail.refusal` of `op` with its text interned, so the failure outlives `a`'s storage; a failure that stands in for the text carries a static one and is returned as it is |
| [`suggestion_build`](#suggestion_build) | the text "did you mean `name`?", owned by `a` |
| [`fix_label_replace`](#fix_label_replace) | the fix label "replace with `name`", owned by `a` |

---

## Origin

```mach
pub def Origin: u8
```

the phase or command that produced a diagnostic. ORIGIN_NONE is a store
nothing has attributed yet; every other member has a stable name that the
machine-readable rendering carries

| Name | Value | |
| :--- | :--- | :--- |
| `ORIGIN_NONE` | `0` |  |
| `ORIGIN_BUILD` | `1` |  |
| `ORIGIN_LOAD` | `2` |  |
| `ORIGIN_RESOLVE` | `3` |  |
| `ORIGIN_SEMA` | `4` |  |
| `ORIGIN_LOWER` | `5` |  |
| `ORIGIN_CODEGEN` | `6` |  |
| `ORIGIN_LINK` | `7` |  |
| `ORIGIN_TEST` | `8` |  |

### origin_name

```mach
pub fun origin_name(o: Origin) opt[str]
```

the stable name of an origin

| Parameter | |
| :--- | :--- |
| `o` | the origin |
| **returns** | its name, or none for ORIGIN_NONE and a tag outside the catalog |

## ChildKind

```mach
pub def ChildKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CHILD_NOTE` | `0` |  |
| `CHILD_HELP` | `1` |  |

## Child

```mach
pub rec Child {
    kind: ChildKind;
    text: str;
}
```

a note or a help line attached to a diagnostic

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `ChildKind` | CHILD_NOTE or CHILD_HELP |
| `text` | `str` | the line, owned by the allocator of the diagnostic that holds it |

## FixId

```mach
pub def FixId: usize
```

a fix's position in its diagnostic's fix list

## Diagnostic

```mach
pub rec Diagnostic {
    kind:     mach.base.diagnostic.kind.Kind;
    origin:   Origin;
    loc:      mach.base.source.Location;
    message:  str;
    children: Vector[Child];
    related:  Vector[mach.base.diagnostic.annotation.Related];
    fixes:    Vector[mach.base.diagnostic.annotation.Fix];
}
```

one diagnostic; its severity is its kind's, read with `is_error`

### is_error

```mach
pub fun is_error(d: *Diagnostic) bool
```

the diagnostic is an error: its kind's row declares it one

### report_key

```mach
pub fun report_key(d: *Diagnostic) u64
```

what tells one report from another wherever it is held: the digest of everything it reports
but its origin, so a store rebuilt from its sources gives a report it held before the same key

## DiagnosticStore

```mach
pub rec DiagnosticStore {
    a:     *std.allocator.Allocator;
    items: Vector[Diagnostic];
    # the digest of each held report, to the first item that carries it
    reports:     std.collections.map.Map[u64, usize];
    error_count: usize;
    lost:        usize;
    epoch:       u64;
    # the origin a diagnostic committed here takes when its builder names none
    origin: Origin;
    # how the project being built governs its warnings, and a warning that stands in no file
    # another owner holds
    built: Governance;
    # how each other owner governs its warnings, and the owner of each file one holds, a
    # position in `others` keyed by the file's index
    others: Vector[Governance];
    owners: std.collections.map.Map[u32, u32];
    # the `#[expect]` acknowledgements a warning is matched against before it
    # is silenced or kept, in the order they were gathered
    expects: Vector[Expectation];
    # each file's expectations: positions in `expects`, grouped by file in
    # gathering order, and the run of them each file owns
    expect_order:   Vector[usize];
    expect_by_file: std.collections.map.Map[u32, ExpectRun];
    # how many expectations warnings have been matched against
    expect_examined: u64;
}
```

the diagnostics of a build, in the order they were committed. a store never
holds the same report twice: a commit equal to one it holds, in everything but
its origin, is dropped

### store_init

```mach
pub fun store_init(a: *std.allocator.Allocator) DiagnosticStore
```

an empty store attributing to no origin, showing every warning of the project being built

| Parameter | |
| :--- | :--- |
| `a` | the allocator every diagnostic the store holds is owned through |
| **returns** | the store, released with store_dnit |

### set_origin

```mach
pub fun set_origin(store: *DiagnosticStore, o: Origin)
```

attribute every diagnostic committed to the store from now on whose builder
names no origin to `o`; a diagnostic already committed keeps its own

### govern

```mach
pub fun govern(store: *DiagnosticStore, g: Governance)
```

govern the warnings of the project being built that reach the store from now on by `g`

### owner_add

```mach
pub fun owner_add(store: *DiagnosticStore, g: Governance) res[u32, mach.base.fail.Fail]
```

a further owner of warnings, governed by `g`, whose files `own` hands it

| Parameter | |
| :--- | :--- |
| **returns** | the owner, which `own` takes |

### own

```mach
pub fun own(store: *DiagnosticStore, file: mach.base.source.FileId, owner: u32) err[mach.base.fail.Fail]
```

the warnings that stand in `file` belong to `owner`, one `owner_add` gave

### expect

```mach
pub fun expect(store: *DiagnosticStore, list: Vector[Expectation]) err[mach.base.fail.Fail]
```

acknowledge every warning an expectation in `list` covers as it reaches the
store; the store takes the list, whatever the outcome, and indexes it by file

### report_unfulfilled

```mach
pub fun report_unfulfilled(store: *DiagnosticStore)
```

an expectation nothing fulfilled is itself a warning, where it is judged: a
build with errors judges none, since a pass it stopped may have been the one
to raise the warning. the judgement is type checking's, whatever the store
stamps otherwise

### drop_file

```mach
pub fun drop_file(store: *DiagnosticStore, file_id: mach.base.source.FileId)
```

drop every diagnostic located in one file, keeping the rest in order. a module refreshed
in place re-reports its own load diagnostics, so the old ones go; a mark taken before the
drop is stale afterwards, as after a truncate

| Parameter | |
| :--- | :--- |
| `store` | the store |
| `file_id` | the file whose diagnostics go |

### drop_file_except

```mach
pub fun drop_file_except(store: *DiagnosticStore, file_id: mach.base.source.FileId, sites: *u32, site_count: usize)
```

drop every diagnostic located in one file except those that begin at one of `sites`,
keeping the rest in order, as drop_file does

| Parameter | |
| :--- | :--- |
| `store` | the store |
| `file_id` | the file whose diagnostics go |
| `sites` | the offsets in the file whose diagnostics stay, `site_count` of them |

### store_dnit

```mach
pub fun store_dnit(store: *DiagnosticStore)
```

release a store and every diagnostic it holds; a nil store holds nothing and releases
nothing

### len

```mach
pub fun len(store: *DiagnosticStore) usize
```

how many diagnostics `store` holds; a nil store, as a walk that reports nowhere has, holds none

### error_count

```mach
pub fun error_count(store: *DiagnosticStore) usize
```

how many errors `store` counts; a nil store counts none

### report

```mach
pub fun report(store: *DiagnosticStore, k: mach.base.diagnostic.kind.Kind, file_id: mach.base.source.FileId, span: mach.base.source.Span, message: str) err[mach.base.fail.Fail]
```

a diagnostic of kind `k` with no attachments

### reportf

```mach
pub fun reportf(store: *DiagnosticStore, k: mach.base.diagnostic.kind.Kind, file_id: mach.base.source.FileId, span: mach.base.source.Span, fmt: str, args: ...) err[mach.base.fail.Fail]
```

a diagnostic of kind `k` with no attachments, its message `fmt` formatted with `args`

### record

```mach
pub fun record(store: *DiagnosticStore, k: mach.base.diagnostic.kind.Kind, file_id: mach.base.source.FileId, span: mach.base.source.Span, message: str)
```

a diagnostic of kind `k` reported where the reporter has no failure to
return: one the store cannot take is counted lost, which fails the build. the
store is live, as `live` holds

### record_unlocated

```mach
pub fun record_unlocated(store: *DiagnosticStore, k: mach.base.diagnostic.kind.Kind, message: str)
```

a diagnostic about the build as a whole, located at no source

### reject

```mach
pub fun reject(store: *DiagnosticStore, k: mach.base.diagnostic.kind.Kind, loc: mach.base.source.Location, text: str) mach.base.fail.Fail
```

a pass past the front end rejects the program: the refusal is an error
located at `loc` on the store and the pass answers `reported`, exactly as a
front-end pass does, so rendering, the tally and the exit status derive from
one list. an append the store refuses is an internal failure; a store wired
as nil is a compiler defect, and the text then stands as an internal failure
rather than vanishing

### note_lost

```mach
pub fun note_lost(store: *DiagnosticStore)
```

count a diagnostic `store` could not take, which fails the build; the store is live, as `live`
holds

### lost_count

```mach
pub fun lost_count(store: *DiagnosticStore) usize
```

how many diagnostics `store` could not take; a nil store lost none

### content_equal

```mach
pub fun content_equal(left: *DiagnosticStore, right: *DiagnosticStore) res[bool, mach.base.fail.Fail]
```

compare observable diagnostic content without allocation or store identity

### copy

```mach
pub fun copy(src: *DiagnosticStore, a: *std.allocator.Allocator) res[DiagnosticStore, mach.base.fail.Fail]
```

an owned copy of `src` from `a`, every diagnostic and its lost count kept, released with store_dnit

### replay_into

```mach
pub fun replay_into(dst: *DiagnosticStore, src: *DiagnosticStore) err[mach.base.fail.Fail]
```

commit a copy of every diagnostic of `src` to `dst`, each where it stands, and add `src`'s lost
count to `dst`'s. a copy that fails rolls `dst` back to what it held before

| Parameter | |
| :--- | :--- |
| `dst` | the store that takes the copies |
| `src` | the store replayed, left as it was |
| **returns** | a failure when either store is nil, they are one store, or a copy is refused |

### replay_located

```mach
pub fun replay_located(dst: *DiagnosticStore, src: *DiagnosticStore) err[mach.base.fail.Fail]
```

replay into `dst` only the diagnostics of `src` placed somewhere, with its lost count: what a
report placed nowhere waits for a requester to place does not come along

### replay_unplaced

```mach
pub fun replay_unplaced(dst: *DiagnosticStore, src: *DiagnosticStore, at: mach.base.source.Location) err[mach.base.fail.Fail]
```

replay into `dst` only the diagnostics of `src` placed nowhere (their file base_source.FILE_NIL),
each reported at `at`. the lost count comes along with replay_located, so a store replayed at
several places counts what it lost once

### encode

```mach
pub fun encode(w: *mach.base.wire.Sink, store: *DiagnosticStore, sources: *SourceTable)
```

write the store's diagnostics as bytes: their count, then per diagnostic its
origin, its kind's key, location and message, then its notes, related places
and fixes. a location is its file's path (absent as 0xFFFFFFFF), offset and
length; a text is its length (absent as 0xFFFFFFFF) and bytes; every number
little endian. a kind travels by its key, which never changes meaning

## Governance

```mach
pub rec Governance {
    shown:    bool;
    silenced: mach.base.diagnostic.kind.KindSet;
}
```

how the owner of a warning, the project being built or one of its dependencies, governs
it as it reaches a store: whether the build shows the owner's warnings at all, and the
kinds the owner's own `allow` silences

## Expectation

```mach
pub rec Expectation {
    file_id:   mach.base.source.FileId;
    start:     usize;
    end:       usize;
    key:       mach.base.source.Span;
    text:      str;
    kinds:     mach.base.diagnostic.kind.KindSet;
    judged:    bool;
    fulfilled: bool;
}
```

one key of a declaration's `#[expect]`: a warning of a kind it selects,
located in the declaration's own file between `start` and `end`, is
acknowledged there and dropped

file_id:   the declaration's file
start:     the offset the declaration begins at, its doc comment included
end:       the offset just past the declaration
key:       where the key is spelled, which an unfulfilled report points at
text:      the key as written, owned by the interner it was decoded into
kinds:     the warning kinds the key selects
judged:    the key selects a kind the source alone decides and the
           declaration was checked this build, so fulfilment is reported
fulfilled: a warning matched it

## DiagnosticBuilder

```mach
pub rec DiagnosticBuilder {
    a:    *std.allocator.Allocator;
    live: bool;
    kind: mach.base.diagnostic.kind.Kind;
    # ORIGIN_NONE takes the store's origin at commit
    origin:   Origin;
    loc:      mach.base.source.Location;
    message:  str;
    children: Vector[Child];
    related:  Vector[mach.base.diagnostic.annotation.Related];
    fixes:    Vector[mach.base.diagnostic.annotation.Fix];
}
```

a diagnostic being assembled: notes, help, related places and fixes attach
here, and `commit` hands the whole to a store

### builder_init

```mach
pub fun builder_init(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, file_id: mach.base.source.FileId, span: mach.base.source.Span, message: str) res[DiagnosticBuilder, mach.base.fail.Fail]
```

a builder for a diagnostic of kind `k`

### builder_initf

```mach
pub fun builder_initf(a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, file_id: mach.base.source.FileId, span: mach.base.source.Span, fmt: str, args: ...) res[DiagnosticBuilder, mach.base.fail.Fail]
```

a builder for a diagnostic of kind `k` whose message is `fmt` formatted with `args`; a
format the allocator refuses is a failure, never a substitute message

### builder_dnit

```mach
pub fun builder_dnit(b: *DiagnosticBuilder)
```

release what a builder owns; a nil builder, or one already committed or released, owns
nothing

### attach_note

```mach
pub fun attach_note(b: *DiagnosticBuilder, text: str) err[mach.base.fail.Fail]
```

attach a note, copied through the builder's allocator

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `text` | the note |
| **returns** | a failure when the builder is not live, or the allocator's refusal |

### attach_help

```mach
pub fun attach_help(b: *DiagnosticBuilder, text: str) err[mach.base.fail.Fail]
```

attach a help line, copied through the builder's allocator

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `text` | the help line |
| **returns** | a failure when the builder is not live, or the allocator's refusal |

### attach_related

```mach
pub fun attach_related(b: *DiagnosticBuilder, file_id: mach.base.source.FileId, span: mach.base.source.Span, label: str) err[mach.base.fail.Fail]
```

a related place; a nil or empty label shows the place without one

### attach_fix

```mach
pub fun attach_fix(b: *DiagnosticBuilder, label: str, file_id: mach.base.source.FileId, span: mach.base.source.Span, replacement: str) res[FixId, mach.base.fail.Fail]
```

a fix of one edit, which `attach_fix_edit` extends

### attach_fix_edit

```mach
pub fun attach_fix_edit(b: *DiagnosticBuilder, fix: FixId, file_id: mach.base.source.FileId, span: mach.base.source.Span, replacement: str) err[mach.base.fail.Fail]
```

a further edit of fix `fix`, which may not overlap one it already makes

### withdraw_fix

```mach
pub fun withdraw_fix(b: *DiagnosticBuilder, fix: FixId) err[mach.base.fail.Fail]
```

drop the most recently attached fix, as a fix whose later edit could not be
attached is withdrawn whole

### commit

```mach
pub fun commit(b: *DiagnosticBuilder, store: *DiagnosticStore) err[mach.base.fail.Fail]
```

hand the builder's diagnostic to the store; the builder is consumed whatever
the outcome. a warning the store silences or an expectation acknowledges is
dropped, and so is a report the store already holds

## SourceTable

```mach
pub rec SourceTable {
    ctx:     ptr;
    path_of: fun(ptr, mach.base.source.FileId) str;
    file_at: fun(ptr, str) res[mach.base.source.FileId, mach.base.fail.Fail];
}
```

how an encoded store names its files: by path, which outlives the file ids
of the session that encoded it

ctx:     the table's own state, passed back to each function
path_of: the path a file was loaded from, nil for one the table does not hold
file_at: the file the reading session loaded from `path`, FILE_NIL when it
         loaded none; a refusal when the lookup cannot be made

### decode

```mach
pub fun decode(r: *mach.base.wire.Source, sources: *SourceTable, into: *DiagnosticStore) res[bool, mach.base.fail.Fail]
```

read one store `encode` wrote and commit its diagnostics to `into`: false
when the bytes are malformed or name a kind this compiler does not raise,
which ends the read where it stands

## Functions

### format

```mach
pub fun format(a: *std.allocator.Allocator, fmt: str, va: ...) res[str, mach.base.fail.Fail]
```

`fmt` formatted with `va` into `a`, which owns the text; a refusal to format
is the failure instead

### join

```mach
pub fun join(a: *std.allocator.Allocator, va: ...) res[str, mach.base.fail.Fail]
```

the texts `va` joined into `a`, which owns the result; a refusal to join is
the failure instead

### text_retain

```mach
pub fun text_retain(itn: *mach.base.intern.Interner, text: str) res[str, mach.base.fail.Fail]
```

the interner's copy of a borrowed `text`, which outlives the storage `text`
came from

### failure_retain

```mach
pub fun failure_retain(itn: *mach.base.intern.Interner, f: mach.base.fail.Fail) mach.base.fail.Fail
```

`f` with its text the interner's copy, so it outlives the storage its text was made in, as a
failure a product or a status keeps past the operation that made it does. a reported failure
and a refusal of storage carry no text to copy, and a refused copy is that refusal

### text_intern

```mach
pub fun text_intern(itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, text: str) res[str, mach.base.fail.Fail]
```

`text`, owned by `a`, moved into the interner: the interned copy outlives
`a`'s storage, and `text` is released either way

### format_intern

```mach
pub fun format_intern(itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, fmt: str, va: ...) res[str, mach.base.fail.Fail]
```

`fmt` formatted with `va` through `a` as scratch and interned, so the text
outlives `a`'s storage

### refusal_interned

```mach
pub fun refusal_interned(itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, k: mach.base.diagnostic.kind.Kind, op: mach.base.fail.Op, cause: mach.base.fail.Cause) mach.base.fail.Fail
```

`fail.refusal` of `op` with its text interned, so the failure outlives `a`'s
storage; a failure that stands in for the text carries a static one and is
returned as it is

### suggestion_build

```mach
pub fun suggestion_build(a: *std.allocator.Allocator, name: str) res[str, mach.base.fail.Fail]
```

the text "did you mean `name`?", owned by `a`

| Parameter | |
| :--- | :--- |
| `a` | the allocator the text is owned through |
| `name` | the suggested name |
| **returns** | the text, or the failure that refused the join |

### fix_label_replace

```mach
pub fun fix_label_replace(a: *std.allocator.Allocator, name: str) res[str, mach.base.fail.Fail]
```

the fix label "replace with `name`", owned by `a`

| Parameter | |
| :--- | :--- |
| `a` | the allocator the label is owned through |
| `name` | the replacement name |
| **returns** | the label, or the failure that refused the join |

