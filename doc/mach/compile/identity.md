[mach](../README.md) / [compile](README.md) / **identity**

# mach.compile.identity

source identity: what a merged definition was made from, independent of how
it was compiled (mink.model.merged)

a module's identity folds its fully qualified name, the digest of its
template text (compile/surface: its text without the bodies of tests and of
functions that are not templates, free of paths and positions), the digests
of the files it embeds and the identities of the modules it imports, so it
covers every declaration and every template body an instance of it can
read, transitively. an
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

Source: [`src/compile/identity.mach`](../../../src/compile/identity.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.compile.project`](project.md), [`mach.front.sema.instance`](../front/sema/instance.md), [`mach.session.module`](../session/module.md), [`mach.session.query`](../session/query.md).

| Functions | |
| :--- | :--- |
| [`of_module`](#of_module) | the source identity of module `mid` |
| [`of_instance`](#of_instance) | the identity of the instance `item` of symbol `name` |

| Constants | |
| :--- | :--- |
| [`SOURCE_IDENTITY`](#constants) | a module's source identity, keyed by its stable id |

---

## Functions

### of_module

```mach
pub fun of_module(p: *mach.compile.project.Project, mid: mach.session.module.ModuleId, out: *[32]u8) err[mach.base.fail.Fail]
```

the source identity of module `mid`

### of_instance

```mach
pub fun of_instance(p: *mach.compile.project.Project, item: *mach.front.sema.instance.Instance, name: mach.base.intern.StrId) res[u64, mach.base.fail.Fail]
```

the identity of the instance `item` of symbol `name`

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SOURCE_IDENTITY` | `mach.session.query.Kind` | `query.Kind{ name: "source identity", role: query.DERIVED, owner: query.KEY_MODULE, origin: diagnostic.ORIGIN_LOWER, compute: module_compute }` | a module's source identity, keyed by its stable id |

