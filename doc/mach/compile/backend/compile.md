[mach](../../README.md) / [compile](../README.md) / [backend](README.md) / **compile**

# mach.compile.backend.compile

one mirl module compiled on the back half's prepared target, and what mirl and masc said of it
in mach's terms: a refusal as the failure it is reported under, with the source location mirl
states for it, and each warning mirl returns as a failure of the warning's kind, located the
same way. the compile reads the module alone and leaves each failure unplaced, and the caller
places it in the build's sources with `placed` and reports it

Source: [`src/compile/backend/compile.mach`](../../../../src/compile/backend/compile.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md), [`mach.compile.view`](../view.md), [`mach.lower.context`](../../lower/context.md).

| Type | | |
| :--- | :--- | :--- |
| [`Compiled`](#compiled) | `rec` | a module's object and the warnings mirl gave compiling it, each a failure of its warning's kind with where mirl states it, both owned by the caller and given back with `dnit`, or the warnings alone with `warnings_dnit` once the object has moved on |
| [`Unplaced`](#unplaced) | `rec` | a failure of the compile before it is placed in the build's sources: the failure, where mirl's debug table states it, the places of the failures it names besides itself, each owned through the allocator the compile was given, and the function of the module it is about as a whole when it is about one, whose definition is its place |
| [`Spot`](#spot) | `rec` | a source position as mirl's debug table states it: the path of its file, its line and its column |

| On `Compiled` | |
| :--- | :--- |
| [`compile`](#compile) | the mirl module `m` compiled to mink's object under the setting `optimise`, its schedule's own checks run with `checked`, the linker let relax its code with `relax`, its frames stated for unwind tables where `unwind` holds, and the float relaxations `floats` permitted, the module's text after the pass schedule and masc's listing of the object written where `streams` names writers for them, and its stages timed and counted into `stats` when there is one |
| [`warnings_dnit`](#warnings_dnit) | give back the warnings of what `compile` made, which own their text and places through `a`, the allocator it was given |
| [`dnit`](#dnit) | give back what `compile` made: its object and its warnings |

| On `Unplaced` | |
| :--- | :--- |
| [`unplaced_dnit`](#unplaced_dnit) | give back a failure of the compile, placed or not: its text and places, and the paths of the spots it still holds, all owned through `a`, the allocator the compile was given |
| [`placed`](#placed) | the failure `u` placed in `sources`: one about a function as a whole at the function's definition, as `defined`, the facts the module's lowering kept, states it, and any other at the byte of its spot, with the bytes of the spots it names besides as its related places, as `located` finds them |
| [`unverified`](#unverified) | mirl's verifier's refusal `e` of the lowered module `name`, a defect of the lowering, in mirl's words and at the source location mirl states for the instruction it names |
| [`bare`](#bare) | the failure `f` with no spot |

---

## Compiled

```mach
pub rec Compiled {
    object:   Object;
    warnings: std.collections.vector.Vector[Unplaced];
}
```

a module's object and the warnings mirl gave compiling it, each a failure of its warning's kind
with where mirl states it, both owned by the caller and given back with `dnit`, or the
warnings alone with `warnings_dnit` once the object has moved on

### compile

```mach
pub fun compile(b: *Back, a: *std.allocator.Allocator, m: *Module, optimise: Optimise, checked: bool, relax: bool, unwind: bool, floats: mirl.pass.relaxation.Relaxations,
    require: bool, report: bool, closed: bool, streams: mach.compile.view.Streams, stats: opt[*mirl.base.stats.Stats], name: str, owner: str) res[Compiled, Unplaced]
```

the mirl module `m` compiled to mink's object under the setting `optimise`, its schedule's own checks
run with `checked`, the linker let relax its code with `relax`, its frames stated for unwind tables where `unwind`
holds, and the float relaxations `floats`
permitted, the module's text after the pass schedule and masc's listing of the object written where `streams` names
writers for them, and its stages
timed and counted into `stats` when there is one. a vector operation mirl takes lane by lane is
refused where `require` holds, as the profile's `simd = "require"` asks, and is a warning
otherwise, a refusal naming the dependency `owner` whose code the module is, nil for the project
being built. where `report` holds, mirl writes its report of the optimisation passes that changed
each function to stdout, each line whole. where `closed` holds, nothing outside the object refers into it, so mirl
leaves out what no root of its own reaches. a failure or warning carries the location mirl names, unplaced.
`name` is the object's, no other object's in a link. the module is the schedule's to rewrite, so
it is spent here

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

## Unplaced

```mach
pub rec Unplaced {
    fail:  mach.base.fail.Fail;
    at:    opt[Spot];
    also:  std.collections.vector.Vector[Spot];
    whole: opt[Function];
}
```

a failure of the compile before it is placed in the build's sources: the failure, where mirl's
debug table states it, the places of the failures it names besides itself, each owned through
the allocator the compile was given, and the function of the module it is about as a whole when
it is about one, whose definition is its place. `placed` puts it in the sources

### unplaced_dnit

```mach
pub fun unplaced_dnit(a: *std.allocator.Allocator, u: *Unplaced)
```

give back a failure of the compile, placed or not: its text and places, and the paths of the
spots it still holds, all owned through `a`, the allocator the compile was given

### placed

```mach
pub fun placed(a: *std.allocator.Allocator, sources: *mach.base.source.SourceMap, u: *Unplaced, defined: *mach.lower.context.Unit) mach.base.fail.Fail
```

the failure `u` placed in `sources`: one about a function as a whole at the function's definition, as
`defined`, the facts the module's lowering kept, states it, and any other at the byte of its spot, with
the bytes of the spots it names besides as its related places, as `located` finds them. a module
whose lowering kept no facts passes nil, and its failures stand at their spots. `u` holds the
placed failure from then on and no spot, its spots given back through `a`, the allocator the
compile was given

### unverified

```mach
pub fun unverified(a: *std.allocator.Allocator, m: *Module, name: str, e: mirl.verify.failure.Error) Unplaced
```

mirl's verifier's refusal `e` of the lowered module `name`, a defect of the lowering, in mirl's words and at the
source location mirl states for the instruction it names

### bare

```mach
pub fun bare(f: mach.base.fail.Fail) Unplaced
```

the failure `f` with no spot

## Spot

```mach
pub rec Spot {
    path: str;
    line: usize;
    col:  usize;
}
```

a source position as mirl's debug table states it: the path of its file, its line and its column

