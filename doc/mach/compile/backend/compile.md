[mach](../../README.md) / [compile](../README.md) / [backend](README.md) / **compile**

# mach.compile.backend.compile

one mirl module compiled on the back half's prepared target, and what mirl and masc said of it
in mach's terms: a refusal as the failure it is reported under, at the source location mirl
states for it, and each warning mirl returns as a failure of the warning's kind, located the
same way, which the caller reports

Source: [`src/compile/backend/compile.mach`](../../../../src/compile/backend/compile.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`Compiled`](#compiled) | `rec` | a module's object and the warnings mirl gave compiling it, each a failure of its warning's kind located where mirl states, both owned by the caller and given back with `dnit`, or the warnings alone with `warnings_dnit` once the object has moved on |

| On `Compiled` | |
| :--- | :--- |
| [`compile`](#compile) | the mirl module `m` compiled to mink's object at mirl's level `level`, its schedule's own checks run with `checked`, the linker let relax its code with `relax` and the float relaxations `floats` permitted, masc's listing of the object written to `listing` when there is one, and its stages timed and counted into `stats` when there is one |
| [`warnings_dnit`](#warnings_dnit) | give back the warnings of what `compile` made, which own their text and places through `a`, the allocator it was given |
| [`dnit`](#dnit) | give back what `compile` made: its object and its warnings |

---

## Compiled

```mach
pub rec Compiled {
    object:   Object;
    warnings: std.collections.vector.Vector[mach.base.fail.Fail];
}
```

a module's object and the warnings mirl gave compiling it, each a failure of its warning's kind
located where mirl states, both owned by the caller and given back with `dnit`, or the
warnings alone with `warnings_dnit` once the object has moved on

### compile

```mach
pub fun compile(b: *Back, a: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, m: *Module, level: u8, checked: bool, relax: bool, floats: mirl.pass.relaxation.Relaxations,
    listing: opt[*std.io.writer.Writer], stats: opt[*mirl.base.stats.Stats], name: str) res[Compiled, mach.base.fail.Fail]
```

the mirl module `m` compiled to mink's object at mirl's level `level`, its schedule's own checks
run with `checked`, the linker let relax its code with `relax` and the float relaxations `floats`
permitted, masc's listing of the object written to `listing` when there is one, and its stages
timed and counted into `stats` when there is one. a failure or warning points at the file of
`sources` mirl's location names. `name` is the object's, no other object's in a link. the module is the schedule's
to rewrite, so it is spent here

### warnings_dnit

```mach
pub fun warnings_dnit(a: *std.allocator.Allocator, c: *Compiled)
```

give back the warnings of what `compile` made, which own their text and places through `a`, the
allocator it was given

### dnit

```mach
pub fun dnit(a: *std.allocator.Allocator, c: *Compiled)
```

give back what `compile` made: its object and its warnings

