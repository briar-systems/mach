[mach](../README.md) / [compile](README.md) / **cache**

# mach.compile.cache

the persistent object cache. every module the build emits is keyed once per
load, after the load walk and before anything is resolved: its key covers
the build configuration and compiler identity, the module's own source and
embedded files, and the surface of every module it imports, transitively.
an import's surface is its source without any function body (see
compile/surface and key_module), so an edit inside a body keys only its own
module. an object holds every instance its module's code reaches, and the
body of each is held to the object by the object's record, which lists each
under its key (instance_key, read), so an edit to a generic's body rebuilds
the objects holding an instance of it. a module whose `obj/` object carries
its key is reused: it is not lowered or compiled, its instances are not
typed, and when nothing the build still compiles imports it, its front end is
skipped: its record stands in for its warnings.
nothing about a file but its bytes decides anything: every source text load
read is hashed each build, and the digest memo in the cell's `.cache`
directory (compile/cache/memo) keeps only what a text's digest names. an
object is reused only when its bytes hash to what the build that wrote it
recorded (compile/cache/objects)

Source: [`src/compile/cache.mach`](../../../src/compile/cache.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.backend`](backend.md), [`mach.compile.cache.compiler`](cache/compiler.md), [`mach.compile.project`](project.md), [`mach.host.output`](../host/output.md), [`mach.lower.testrunner`](../lower/testrunner.md), [`mach.report.readout`](../report/readout.md), [`mach.session.module`](../session/module.md), [`mach.session.query`](../session/query.md).

| Type | | |
| :--- | :--- | :--- |
| [`ObjectWrite`](#objectwrite) | `rec` | an object to write, with the record it carries, nil for none |

| On `ObjectWrite` | |
| :--- | :--- |
| [`object_write`](#object_write) | the object's bytes, its record in the section the target carries it in, fill `t`, and the digest of what reached `obj/` is noted |

| Functions | |
| :--- | :--- |
| [`configuration`](#configuration) | what every module's key shares: the compiler identity, the configuration bytes the driver captured (target, platform, project identity), the request, the realized dependency closure and the link requirements |
| [`test_object_path`](#test_object_path) | where the build writes module m's test object: `obj/<project>/<module path>.test.<ext>` |
| [`has_test_object`](#has_test_object) | the module gets a test object in this build: a test build, and a module whose source declares a test or a `#[testing]` declaration |
| [`early_restore_enabled`](#early_restore_enabled) | an object is restored from the cache only when no emitter is on: the ir text and the listing are written as a module compiles, so under `--emit ir` or `--emit asm` every module compiles |
| [`restored`](#restored) | the module's codegen product is its cached object under the key this load built, whether it is still staged or a query already took it |
| [`staged_restored`](#staged_restored) | restored and still staged, which is what the codegen query publishes |
| [`test_restored`](#test_restored) | the module's test object is its cached object under the key this load built |
| [`test_staged_restored`](#test_staged_restored) | the test object is restored and still staged for the codegen query to publish |
| [`skips_lowering`](#skips_lowering) | the module is restored and nothing needs its lowered ir |
| [`lowered_count`](#lowered_count) | the units the build lowers |
| [`generated_count`](#generated_count) | the units the build generates code for |
| [`frontend_count`](#frontend_count) | the modules the front end still processes |
| [`forget`](#forget) | every key, skip decision and record of the previous load is dropped: the next load keys again, and a project that is not keyed processes every module |
| [`note_written`](#note_written) | the object written to `location` was the `len` bytes at `bytes`: a keyed object under `obj/` is remembered by its digest for the restore that reads it back |
| [`store_written`](#store_written) | store the digests of the objects this build wrote, once they are in place |
| [`object_written`](#object_written) | a completed unit's object reaches its `obj/` file through a sibling temporary, carrying its record, since the object cache reads it back, and the build holds no form of it after: a link reads it back from there (objects_back) |
| [`test_object_written`](#test_object_written) | the same for the module's test object, its time carved out of test codegen's |
| [`objects_back`](#objects_back) | the objects of the units in the link's order, each read back from the file it was written to when its unit completed into the slot it left, as the cache restores one, so a link reads the one form whether the object was made now or restored |
| [`test_objects_back`](#test_objects_back) | the same for the test objects of the emitted modules, in their order |
| [`object_reread`](#object_reread) | `o` replaced by the object the file `from` holds, which the build wrote from it |
| [`test_input_key`](#test_input_key) | the query key of a module's test object products and its object key input |
| [`instance_key`](#instance_key) | the key of an instance: the configuration, its symbol and what it reads |
| [`prepare`](#prepare) | once per load, after the load walk and the embedded inputs, before anything is resolved: key every module, read each emitted module's `obj/` object under its key and mark the modules the front end skips |
| [`take_staged`](#take_staged) | hand the staged object to the query that publishes it; the facts stay with the module |
| [`take_staged_test`](#take_staged_test) | hand the staged test object to the query that publishes it, its facts staying with the module |
| [`collect_tests`](#collect_tests) | a module's tests, from its lowered test unit or from the facts its restored test object carries |
| [`source_table`](#source_table) | the session's files named by path, as a recorded store names them |
| [`publish`](#publish) | a generated object carries its record into `obj/`: its key, its warnings and where they are placed, the instances it holds a copy of and, for a module that declares tests, the symbols it defines, which its test object declares |
| [`publish_test`](#publish_test) | the same for the module's test object, which records only its own back half's warnings: the normal object carries the front end's |
| [`link_stands`](#link_stands) | the link that would write `artifact` need not run: this build compiled nothing, the link reads no input found by name and its last run read no file a search found, its key and the files that run read beyond its keyed inputs key it as the record of the artifact's path does, and the file there hashes to the digest that record holds |
| [`note_link`](#note_link) | the link that wrote `artifact` as the bytes `digest` names, having read `read`, is recorded under its key and the files it read, so a later build that compiles nothing, keys its link the same and finds those files holding the same bytes leaves the artifact as it is |

| Constants | |
| :--- | :--- |
| [`TEST_KEY_BIT`](#constants) | a module's test object is keyed in the query database by its stable id and this bit |
| [`OBJECT_KEY`](#constants) | the key a module's reused object was built under, keyed by its stable id and the test bit |

---

## ObjectWrite

```mach
pub rec ObjectWrite {
    p:      *mach.compile.project.Project;
    o:      *Object;
    record: *mach.compile.project.Record;
}
```

an object to write, with the record it carries, nil for none

### object_write

```mach
pub fun object_write(w: *ObjectWrite, t: *mach.host.output.Temporary) err[mach.base.fail.Fail]
```

the object's bytes, its record in the section the target carries it in, fill `t`, and the
digest of what reached `obj/` is noted

## Functions

### configuration

```mach
pub fun configuration(scratch: *std.allocator.Allocator, p: *mach.compile.project.Project, config: *u8, config_len: usize,
    compiler_id: *mach.compile.cache.compiler.Identity, digest: *[32]u8) err[mach.base.fail.Fail]
```

what every module's key shares: the compiler identity, the configuration
bytes the driver captured (target, platform, project identity), the request,
the realized dependency closure and the link requirements.
a step's output reaches a module only as a file it embeds or a source it
loads, each keyed by its bytes, so no step is part of the configuration

### test_object_path

```mach
pub fun test_object_path(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry, a: *std.allocator.Allocator) res[str, mach.base.fail.Fail]
```

where the build writes module m's test object: `obj/<project>/<module path>.test.<ext>`

### has_test_object

```mach
pub fun has_test_object(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the module gets a test object in this build: a test build, and a module whose
source declares a test or a `#[testing]` declaration

### early_restore_enabled

```mach
pub fun early_restore_enabled(p: *mach.compile.project.Project) bool
```

an object is restored from the cache only when no emitter is on: the ir text and the
listing are written as a module compiles, so under `--emit ir` or `--emit asm` every
module compiles

### restored

```mach
pub fun restored(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the module's codegen product is its cached object under the key this load
built, whether it is still staged or a query already took it

### staged_restored

```mach
pub fun staged_restored(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

restored and still staged, which is what the codegen query publishes

### test_restored

```mach
pub fun test_restored(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the module's test object is its cached object under the key this load built

### test_staged_restored

```mach
pub fun test_staged_restored(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the test object is restored and still staged for the codegen query to publish

### skips_lowering

```mach
pub fun skips_lowering(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) bool
```

the module is restored and nothing needs its lowered ir

### lowered_count

```mach
pub fun lowered_count(p: *mach.compile.project.Project) u32
```

the units the build lowers

### generated_count

```mach
pub fun generated_count(p: *mach.compile.project.Project) u32
```

the units the build generates code for

### frontend_count

```mach
pub fun frontend_count(p: *mach.compile.project.Project) u32
```

the modules the front end still processes

### forget

```mach
pub fun forget(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

every key, skip decision and record of the previous load is dropped: the next load
keys again, and a project that is not keyed processes every module

### note_written

```mach
pub fun note_written(p: *mach.compile.project.Project, location: str, bytes: *u8, len: usize) err[mach.base.fail.Fail]
```

the object written to `location` was the `len` bytes at `bytes`: a keyed object under `obj/`
is remembered by its digest for the restore that reads it back

### store_written

```mach
pub fun store_written(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

store the digests of the objects this build wrote, once they are in place

### object_written

```mach
pub fun object_written(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) err[mach.base.fail.Fail]
```

a completed unit's object reaches its `obj/` file through a sibling temporary, carrying its
record, since the object cache reads it back, and the build holds no form of it after: a link
reads it back from there (objects_back). a module the cache restored is there already, and one
with nowhere to go keeps its object. the time is the emit row's, carved out of codegen's

### test_object_written

```mach
pub fun test_object_written(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) err[mach.base.fail.Fail]
```

the same for the module's test object, its time carved out of test codegen's

### objects_back

```mach
pub fun objects_back(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the objects of the units in the link's order, each read back from the file it was written to
when its unit completed into the slot it left, as the cache restores one, so a link reads the
one form whether the object was made now or restored. what the format writes and reads back
otherwise is not the object's and never reaches a link. the units before the first without an
object, which a link refuses, are read, and an object held in memory is left as it is. on more
than one thread the reads run on a crew, and what they make and the failure returned are the
same as reading them in order

### test_objects_back

```mach
pub fun test_objects_back(p: *mach.compile.project.Project) err[mach.base.fail.Fail]
```

the same for the test objects of the emitted modules, in their order

### object_reread

```mach
pub fun object_reread(p: *mach.compile.project.Project, o: *Object, from: str) err[mach.base.fail.Fail]
```

`o` replaced by the object the file `from` holds, which the build wrote from it

### test_input_key

```mach
pub fun test_input_key(m: *mach.compile.project.ModuleEntry) u64
```

the query key of a module's test object products and its object key input

### instance_key

```mach
pub fun instance_key(p: *mach.compile.project.Project, name: mach.base.intern.StrId, reads: *mach.session.module.ModuleId, count: usize, out: *[32]u8) res[bool, mach.base.fail.Fail]
```

the key of an instance: the configuration, its symbol and what it reads. an object holding a
copy of it lists it under it. false when a module it reads is not keyed

### prepare

```mach
pub fun prepare(p: *mach.compile.project.Project, config: *u8, config_len: usize, ph: mach.report.readout.Phase) err[mach.base.fail.Fail]
```

once per load, after the load walk and the embedded inputs, before anything
is resolved: key every module, read each emitted module's `obj/` object
under its key and mark the modules the front end skips. config is the
captured configuration identity. its items are reported under the readout
phase ph

### take_staged

```mach
pub fun take_staged(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) *Object
```

hand the staged object to the query that publishes it; the facts stay with the module

### take_staged_test

```mach
pub fun take_staged_test(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry) *Object
```

hand the staged test object to the query that publishes it, its facts staying with the module

### collect_tests

```mach
pub fun collect_tests(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry, mid: u32, c: *mach.lower.testrunner.Collected) err[mach.base.fail.Fail]
```

a module's tests, from its lowered test unit or from the facts its restored test object carries

### source_table

```mach
pub fun source_table(p: *mach.compile.project.Project) mach.base.diagnostic.SourceTable
```

the session's files named by path, as a recorded store names them

### publish

```mach
pub fun publish(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry, o: *Object, defined: *mach.base.intern.StrId, defined_count: u32, copied: *mach.base.intern.StrId,
    copied_count: u32, object: mach.session.query.QueryKey) err[mach.base.fail.Fail]
```

a generated object carries its record into `obj/`: its key, its warnings and where they are
placed, the instances it holds a copy of and, for a module that declares tests, the symbols it
defines, which its test object declares

### publish_test

```mach
pub fun publish_test(p: *mach.compile.project.Project, m: *mach.compile.project.ModuleEntry, o: *Object, copied: *mach.base.intern.StrId, copied_count: u32, object: mach.session.query.QueryKey) err[mach.base.fail.Fail]
```

the same for the module's test object, which records only its own back half's warnings:
the normal object carries the front end's

### link_stands

```mach
pub fun link_stands(p: *mach.compile.project.Project, artifact: str) res[bool, mach.base.fail.Fail]
```

the link that would write `artifact` need not run: this build compiled nothing, the link reads
no input found by name and its last run read no file a search found, its key and the files that
run read beyond its keyed inputs key it as the record of the artifact's path does, and the file
there hashes to the digest that record holds. every file is read and hashed every time, never
trusted from its metadata

### note_link

```mach
pub fun note_link(p: *mach.compile.project.Project, artifact: str, digest: *[32]u8, read: *mach.compile.backend.LinkRead) err[mach.base.fail.Fail]
```

the link that wrote `artifact` as the bytes `digest` names, having read `read`, is recorded under
its key and the files it read, so a later build that compiles nothing, keys its link the same and
finds those files holding the same bytes leaves the artifact as it is. a link that read a file a
search found is recorded as one that always runs

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `TEST_KEY_BIT` | `u64` | `0x100000000` | a module's test object is keyed in the query database by its stable id and this bit |
| `OBJECT_KEY` | `mach.session.query.Kind` | `query.Kind{name: "object key", role: query.INPUT, owner: query.KEY_MODULE}` | the key a module's reused object was built under, keyed by its stable id and the test bit |

