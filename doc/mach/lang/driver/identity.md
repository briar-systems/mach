# mach.lang.driver.identity

source identity: what a merged definition was made from, independent of how
it was compiled (mink.model.merged)

a module's identity folds its fully qualified name, the digest of its bare
surface (driver/surface: its text without the bodies no importer compiles,
free of paths and positions), the digests of the files it embeds and the
identities of the modules it imports, so it covers every declaration and
every generic or comptime body an instance of it can read, transitively. an
instance's identity has the parts mink.model.merged states: its source, a
digest of its symbol and the identities of the modules its typing reads
(frontend.instance_reads), then the float relaxations the build permits and
the digest of the build's closed instruction set selection mirl makes of
every merged definition (mirl.target.basis.selection_digest), the two
settings that change
what a copy computes or where it runs, apart so a refusal can say which
differed. the compiler, the optimisation level, debug information and every
path stay out, so two libraries built from the same source under any of
those settings hold copies that merge

the parts are digests cut to 32 and 16 bits: ample against accident, and no
authentication. an object made by hand that copies an identity onto other
code merges with the copies it imitates

## val SOURCE_IDENTITY

```mach
pub val SOURCE_IDENTITY: query.Kind = query.Kind;
```

a module's source identity, keyed by its stable id

## fun of_module

```mach
pub fun of_module(p: *project.Project, mid: module.ModuleId, out: *[32]u8) err[fail.Fail];
```

the source identity of module `mid`

## fun of_instance

```mach
pub fun of_instance(p: *project.Project, item: *sema_instance.Instance, name: intern.StrId) res[u64, fail.Fail];
```

the identity of the instance `item` of symbol `name`

