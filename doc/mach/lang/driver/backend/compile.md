# mach.lang.driver.backend.compile

one mirl module compiled on the back half's prepared target, and what mirl and masc said of it
in mach's terms: a refusal as the failure it is reported under, at the source location mirl
states for it, and each warning mirl returns as a failure of the warning's kind, located the
same way, which the caller reports

## rec Compiled

```mach
pub rec Compiled;
```

a module's object and the warnings mirl gave compiling it, each a failure of its warning's kind
located where mirl states, both owned by the caller and given back with `release`, or the
warnings alone with `release_warnings` once the object has moved on

## fun compile

```mach
pub fun compile(b: *Back, a: *A.Allocator, m: *Module, level: u8, checked: bool, relax: bool, floats: relaxation.Relaxations,
listing: opt[*std_writer.Writer], stats: opt[*mirl_stats.Stats], name: str) res[Compiled, fail.Fail];
```

the mirl module `m` compiled to mink's object at mirl's level `level`, its schedule's own checks
run with `checked`, the linker let relax its code with `relax` and the float relaxations `floats`
permitted, masc's listing of the object written to `listing` when there is one, and its stages
timed and counted into `stats` when there is one. the module is the schedule's to rewrite, so
it is spent here

## fun release_warnings

```mach
pub fun release_warnings(a: *A.Allocator, c: *Compiled);
```

give back the warnings of what `compile` made, which own their text and places through `a`, the
allocator it was given

## fun release

```mach
pub fun release(a: *A.Allocator, c: *Compiled);
```

give back what `compile` made: its object and its warnings

